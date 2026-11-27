#!/usr/bin/env python3
"""Preflight changed WKC translation units and approve safe cached-object reuse."""
import concurrent.futures
import json
import os
import pathlib
import re
import shlex
import subprocess
import sys

root = pathlib.Path.cwd().resolve()
build_dir = root / "build"
baseline_file = build_dir / ".wkc-cache-base-sha"
safe_marker = build_dir / ".wkc-preflight-objects-safe"
safe_marker.unlink(missing_ok=True)
baseline = baseline_file.read_text(encoding="utf-8").strip() if baseline_file.is_file() else ""

CXX_SUFFIXES = {".cpp", ".cc", ".cxx", ".c"}
HEADER_SUFFIXES = {".h", ".hh", ".hpp", ".hxx", ".ipp", ".inl", ".inc"}


def has_commit(revision):
    return subprocess.run(
        ["git", "cat-file", "-e", f"{revision}^{{commit}}"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    ).returncode == 0


has_successful_baseline = bool(baseline and has_commit(baseline))
if has_successful_baseline:
    compare_from = baseline
    print(f"Checking source changes since last successful WKC build: {baseline}")
else:
    if baseline:
        print("Cached source baseline is unavailable; rebuilding the bootstrap range.")
    else:
        print("No successful-build baseline found; checking recent commits once.")

    compare_from = "HEAD~5"
    if not has_commit(compare_from):
        ref = os.environ.get("GITHUB_REF")
        if not ref:
            print("::error::Checkout history is too shallow and GITHUB_REF is unavailable.")
            raise SystemExit(1)
        print(f"Checkout does not contain {compare_from}; fetching additional history from {ref}.")
        subprocess.run(
            ["git", "fetch", "--no-tags", "--no-recurse-submodules", "--deepen=10", "origin", ref],
            check=True,
        )

    if not has_commit(compare_from):
        recent = subprocess.check_output(
            ["git", "rev-list", "--max-count=6", "HEAD"], text=True
        ).splitlines()
        if len(recent) > 1:
            compare_from = recent[-1]
            print(f"Using oldest available commit in bootstrap window: {compare_from}")
        else:
            compare_from = "4b825dc642cb6eb9a060e54bf8d69288fbee4904"
            print("Only one commit is available; preflighting sources from the complete initial tree.")

try:
    changed = subprocess.check_output(
        ["git", "diff", "--name-only", "-z", f"{compare_from}..HEAD"],
        text=True,
    ).split("\0")
except subprocess.CalledProcessError:
    print(f"::error::Cannot determine changed files from {compare_from}.")
    raise SystemExit(1)

changed = sorted(set(path for path in changed if path))
if not changed:
    print("No tracked source changes since the successful WKC build baseline.")

try:
    with (build_dir / "compile_commands.json").open(encoding="utf-8") as file:
        commands = json.load(file)
except (OSError, json.JSONDecodeError) as error:
    print(f"::error::Cannot load WKC compile_commands.json: {error}")
    raise SystemExit(1)


def command_arguments(entry):
    if "arguments" in entry:
        return entry["arguments"]
    return shlex.split(entry["command"])


def argument_output_path(entry, flag):
    arguments = command_arguments(entry)
    for index, argument in enumerate(arguments):
        if argument == flag and index + 1 < len(arguments):
            value = pathlib.Path(arguments[index + 1])
            if not value.is_absolute():
                value = pathlib.Path(entry["directory"]) / value
            return value.resolve()
    return None


def depfile_path(entry):
    return argument_output_path(entry, "-MF")


def object_path(entry):
    return argument_output_path(entry, "-o")


def source_path(entry):
    source = pathlib.Path(entry["file"])
    if not source.is_absolute():
        source = pathlib.Path(entry["directory"]) / source
    return source.resolve()


# Some OpenGL-to-GX2 translation units are compiled by the dedicated,
# parallel validation step before this preflight. That step writes their
# successful source paths here; those objects are fresh for this checkout
# even though they intentionally have no Ninja dependency-log records.
verified_manifest = build_dir / ".wkc-preflight-verified-sources"
precompiled_sources = set()
if verified_manifest.is_file():
    for line in verified_manifest.read_text(encoding="utf-8").splitlines():
        relative = line.strip()
        if relative:
            precompiled_sources.add((root / relative).resolve())


# Build a header-to-translation-unit map from compiler depfiles, then
# recover any missing on-disk depfiles from Ninja's persisted .ninja_deps
# database. This matters for restored caches: Ninja keeps dependency records
# even when an individual GCC .d file is absent from the cache archive.
dependency_map = {}
by_source = {}
object_sources = {}
existing_objects = set()
missing_depfiles = set()

for entry in commands:
    source = source_path(entry)
    by_source[source] = entry
    output = object_path(entry)
    if output is not None:
        output = output.resolve()
        object_sources[output] = source
        if output.is_file():
            existing_objects.add(source)

    depfile = depfile_path(entry)
    if depfile is None or not depfile.is_file():
        if output is None or output.is_file():
            missing_depfiles.add(source)
        continue

    try:
        contents = depfile.read_text(encoding="utf-8", errors="replace")
        contents = re.sub(r"\\\r?\n", " ", contents)
        _, separator, prerequisites = contents.partition(":")
        if not separator:
            missing_depfiles.add(source)
            continue
        for dependency in shlex.split(prerequisites):
            dependency_path = pathlib.Path(dependency)
            if not dependency_path.is_absolute():
                dependency_path = pathlib.Path(entry["directory"]) / dependency_path
            dependency_map.setdefault(dependency_path.resolve(), set()).add(source)
    except (OSError, ValueError):
        missing_depfiles.add(source)

# A full-engine cache snapshot can contain older WebCore objects under the same
# CMake target directory even though the bootstrap compile database does not
# include them. Remove those orphaned outputs before preflight and the bootstrap
# build so they cannot be mistaken for WKC cache entries or inflate its manifest.
wkc_object_directory = (build_dir / "CMakeFiles" / "WebKitWKC.dir").resolve()
active_wkc_objects = {
    output for output in object_sources
    if output.is_relative_to(wkc_object_directory)
}
stale_wkc_objects = []
if wkc_object_directory.is_dir():
    for output in wkc_object_directory.rglob("*.obj"):
        resolved = output.resolve()
        if resolved not in active_wkc_objects:
            stale_wkc_objects.append(output)
for output in stale_wkc_objects:
    output.unlink(missing_ok=True)
    output.with_name(output.name + ".d").unlink(missing_ok=True)
if stale_wkc_objects:
    print(f"Removed {len(stale_wkc_objects)} stale WKC-target object(s) outside the current bootstrap graph.")

ninja_dep_sources = set()
try:
    ninja_result = subprocess.run(
        ["ninja", "-C", str(build_dir), "-t", "deps"],
        text=True, capture_output=True, check=False,
    )
except OSError as error:
    ninja_result = None
    print(f"::warning::Could not read Ninja's dependency database: {error}")

if ninja_result is not None and ninja_result.returncode == 0:
    active_source = None
    for line in ninja_result.stdout.splitlines():
        if not line[:1].isspace():
            target, separator, _ = line.partition(": #deps ")
            if not separator:
                active_source = None
                continue
            target_path = pathlib.Path(target)
            if not target_path.is_absolute():
                target_path = build_dir / target_path
            active_source = object_sources.get(target_path.resolve())
            if active_source is not None:
                ninja_dep_sources.add(active_source)
            continue

        if active_source is None:
            continue
        dependency = line.strip()
        if not dependency:
            continue
        dependency_path = pathlib.Path(dependency)
        if not dependency_path.is_absolute():
            dependency_path = build_dir / dependency_path
        dependency_map.setdefault(dependency_path.resolve(), set()).add(active_source)

uncovered_depfiles = (missing_depfiles & existing_objects) - ninja_dep_sources - precompiled_sources
dependency_inventory_complete = bool(commands) and not uncovered_depfiles
if uncovered_depfiles:
    print(
        f"::warning::{len(uncovered_depfiles)} existing object(s) have neither "
        "a readable .d file nor a Ninja dependency record; header-based cache "
        "reuse is disabled for safety."
    )
    for source in sorted(uncovered_depfiles)[:12]:
        print(f"  - {source.relative_to(root) if source.is_relative_to(root) else source}")
elif missing_depfiles:
    print(
        f"Recovered {len(missing_depfiles)} missing compiler depfile(s) "
        "from Ninja's dependency database."
    )

changed_sources = set()
changed_headers = set()
unsafe_inputs = []
for relative in changed:
    if relative.startswith(".github/"):
        continue

    # These files only control the standalone CI generator/configuration or
    # this preflight. Their effect on cached compiler commands is checked by the
    # dependency-aware Ninja dry-run below; do not mark them as blanket-unsafe.
    if relative in {
        "Tools/wiiu/preflight_changed_cpp.py",
        "Tools/wiiu/snapshot_successful_ninja_outputs.py",
        "Source/WebKit/WKC/CMakeLists.txt",
        "Source/WebKit/WKC/scripts/generate-webcore-derived-sources.py",
    }:
        continue

    path = pathlib.PurePosixPath(relative)
    suffix = path.suffix.lower()

    if relative.startswith("Tools/wiiu/"):
        unsafe_inputs.append(relative)
        continue

    if relative.startswith("Source/"):
        absolute = (root / pathlib.Path(relative)).resolve()
        if suffix in CXX_SUFFIXES:
            changed_sources.add(absolute)
        elif suffix in HEADER_SUFFIXES:
            changed_headers.add(absolute)
        else:
            # CMake files, generator inputs, platform configuration, JSON
            # protocols, and other non-C++ inputs can invalidate object files
            # in ways compiler depfiles cannot fully describe.
            unsafe_inputs.append(relative)

affected_sources = {
    source for source in changed_sources
    if source in by_source and source not in precompiled_sources
}
for header in changed_headers:
    affected_sources.update(dependency_map.get(header, set()) - precompiled_sources)

if changed_headers and not dependency_inventory_complete:
    print("::warning::An existing object lacks a readable depfile; cache timestamps will not be trusted after header edits.")
if unsafe_inputs:
    print("::warning::Non-compiler build inputs changed; restored objects will be left for Ninja to validate:")
    for path in unsafe_inputs[:40]:
        print(f"  - {path}")


def compile_source(source):
    entry = by_source[source]
    relative = source.relative_to(root).as_posix() if source.is_relative_to(root) else str(source)
    print(f"=== Preflight compile: {relative} ===", flush=True)
    command = entry.get("command")
    try:
        if command:
            result = subprocess.run(
                command, cwd=entry["directory"], shell=True,
                text=True, capture_output=True,
            )
        else:
            result = subprocess.run(
                entry["arguments"], cwd=entry["directory"],
                text=True, capture_output=True,
            )
        return relative, result.stdout, result.stderr, result.returncode
    except OSError as error:
        return relative, "", str(error), 127


failed = []
if affected_sources:
    workers = min(4, len(affected_sources))
    print(f"Preflighting {len(affected_sources)} affected translation unit(s) with {workers} workers.")
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as executor:
        futures = [executor.submit(compile_source, source) for source in sorted(affected_sources)]
        for future in concurrent.futures.as_completed(futures):
            relative, stdout, stderr, status = future.result()
            if stdout:
                print(stdout, end="" if stdout.endswith("\n") else "\n")
            if stderr:
                print(stderr, end="" if stderr.endswith("\n") else "\n", file=sys.stderr)
            if status:
                failed.append((relative, status))
            else:
                print(f"PASS: {relative}", flush=True)

if failed:
    for source, status in failed:
        print(f"::error::Preflight failed: {source} (exit {status})")
    raise SystemExit(1)

safe_to_reuse = has_successful_baseline and dependency_inventory_complete and not unsafe_inputs

if safe_to_reuse:
    # Checkout gives every source/header a fresh timestamp. Do not touch restored
    # object files: Ninja records each output mtime in .ninja_log, so touching an
    # output makes Ninja believe that it was modified outside the build.
    # Instead, after all changed translation units have passed preflight, move
    # only compiler inputs inside this workspace just before the oldest cached
    # object. Their contents are unchanged or have already been precompiled.
    outputs = []
    missing_outputs = []
    for entry in commands:
        output = object_path(entry)
        if output is None or not output.is_file():
            missing_outputs.append(source_path(entry))
        else:
            outputs.append(output)

    if missing_outputs:
        safe_to_reuse = False
        print(f"Incremental cache reuse not approved: {len(missing_outputs)} object output(s) are missing.")
        for source in missing_outputs[:12]:
            print(f"  - {source.relative_to(root) if source.is_relative_to(root) else source}")
    elif outputs:
        oldest_object_mtime_ns = min(path.stat().st_mtime_ns for path in outputs)
        input_mtime_ns = max(1, oldest_object_mtime_ns - 1_000_000_000)
        workspace_dependencies = set(dependency_map)
        workspace_dependencies.update(by_source)
        normalized_inputs = 0

        for dependency in workspace_dependencies:
            if not dependency.is_relative_to(root) or not dependency.is_file():
                continue
            try:
                stat = dependency.stat()
                if stat.st_mtime_ns > input_mtime_ns:
                    os.utime(dependency, ns=(stat.st_atime_ns, input_mtime_ns))
                    normalized_inputs += 1
            except OSError as error:
                safe_to_reuse = False
                print(f"::warning::Could not normalize cached compiler input {dependency}: {error}")
                break

        if safe_to_reuse:
            # Prove the restored graph is genuinely clean before allowing the
            # workflow to call the normal build command. A changed compiler
            # command, missing output, or newer external toolchain header must
            # remain dirty and be rebuilt by Ninja.
            preview = subprocess.run(
                ["ninja", "-C", str(build_dir), "-n", "WebKitWKC"],
                text=True, capture_output=True, check=False,
            )
            if preview.returncode:
                sys.stderr.write(preview.stdout + preview.stderr)
                safe_to_reuse = False
            else:
                pending_objects = [
                    line for line in preview.stdout.splitlines()
                    if re.search(r"\] Building (?:C|CXX) object ", line)
                ]
                print(f"Restored WKC object outputs present: {len(outputs)}")
                print(f"Workspace compiler-input timestamps normalized: {normalized_inputs}")
                print(f"Ninja dry-run object compilations pending: {len(pending_objects)}")
                if pending_objects:
                    safe_to_reuse = False
                    print("Incremental cache reuse not approved; Ninja must rebuild these objects:")
                    for line in pending_objects[:20]:
                        print(f"  {line}")
                else:
                    print("Ninja confirms that no cached WKC object needs recompilation.")

if safe_to_reuse:
    state = "objects-preflighted" if affected_sources else "unchanged"
    safe_marker.write_text(state + "\n", encoding="utf-8")
    print(f"Incremental cache reuse approved: {state}.")
else:
    if not has_successful_baseline:
        print(
            "No successful-build baseline is available; preflight is diagnostic only. "
            "Ninja must validate restored objects before the first successful build."
        )
    elif not unsafe_inputs and dependency_inventory_complete:
        print("Incremental cache reuse not approved; Ninja will validate restored objects normally.")
    else:
        print("Incremental cache reuse not approved because build inputs or dependency records are incomplete.")

if affected_sources:
    print(f"Preflight passed for {len(affected_sources)} affected WKC translation unit(s).")
else:
    print("No WKC translation units need preflight compilation.")
