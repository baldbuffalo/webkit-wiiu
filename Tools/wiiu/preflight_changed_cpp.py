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

uncovered_depfiles = (missing_depfiles & existing_objects) - ninja_dep_sources
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

    # This helper controls CI preflight only; it is not an input to compilation.
    if relative == "Tools/wiiu/preflight_changed_cpp.py":
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

affected_sources = {source for source in changed_sources if source in by_source}
for header in changed_headers:
    affected_sources.update(dependency_map.get(header, set()))

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

safe_to_reuse = has_successful_baseline and (dependency_inventory_complete or not changed_headers) and not unsafe_inputs
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
    else:
        print("Incremental cache reuse not approved; Ninja will validate restored objects normally.")

if affected_sources:
    print(f"Preflight passed for {len(affected_sources)} affected WKC translation unit(s).")
else:
    print("No WKC translation units need preflight compilation.")
