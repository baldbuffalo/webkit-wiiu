#!/usr/bin/env python3
"""Create a cache snapshot containing only completed Ninja target outputs.

The active build directory is read-only. Ninja's build graph is compared with
.ninja_log and a dry-run dirty-target scan: outputs never successfully built,
currently dirty, or failed are not copied, even if stale files exist. CMake/Ninja
metadata and non-target configure files are preserved for incremental reuse.
"""

from __future__ import annotations

import argparse
import os
import shlex
import shutil
import subprocess
import sys
from pathlib import Path


def workspace_path(value: str, workspace: Path) -> Path:
    path = Path(value)
    if not path.is_absolute():
        path = workspace / path
    return Path(os.path.abspath(path))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", default="build", help="Active Ninja build directory (source)")
    parser.add_argument("--destination", required=True, help="New cache snapshot directory")
    parser.add_argument("--log", required=True, help="Ninja output log containing FAILED lines")
    parser.add_argument(
        "--require-failures",
        action="store_true",
        help="fail rather than create a snapshot if no FAILED Ninja edges are found",
    )
    parser.add_argument(
        "--object-manifest",
        help="write successful, clean WebKitWKC object paths relative to the build directory",
    )
    return parser.parse_args()


def target_path(value: str, build_dir: Path) -> Path:
    path = Path(value)
    if not path.is_absolute():
        path = build_dir / path
    return Path(os.path.abspath(path))


def read_successful_outputs(build_dir: Path) -> set[Path]:
    ninja_log = build_dir / ".ninja_log"
    if not ninja_log.is_file():
        raise ValueError(f"Ninja success log is missing: {ninja_log}")

    successful: set[Path] = set()
    with ninja_log.open("r", encoding="utf-8", errors="replace") as log:
        for line_number, raw_line in enumerate(log, 1):
            line = raw_line.rstrip("\r\n")
            if not line or line.startswith("#"):
                continue

            fields = line.split("\t")
            # Ninja build log v5/v6: start, end, restat_mtime, output, command hash.
            if len(fields) < 5:
                raise ValueError(f"Malformed .ninja_log entry at line {line_number}")

            output = target_path(fields[3], build_dir)
            try:
                output.relative_to(build_dir)
            except ValueError:
                # Outputs outside the cached build directory are not copied.
                continue
            successful.add(output)

    return successful


def read_graph_outputs(build_dir: Path, workspace: Path) -> set[Path]:
    command = ["ninja", "-C", str(build_dir), "-t", "targets", "all"]
    result = subprocess.run(command, cwd=workspace, text=True, capture_output=True)
    if result.returncode:
        detail = result.stderr.strip() or result.stdout.strip()
        raise ValueError(f"Could not enumerate Ninja build outputs: {detail}")

    outputs: set[Path] = set()
    for line in result.stdout.splitlines():
        if ": " not in line:
            continue
        name = line.split(": ", 1)[0]
        output = target_path(name, build_dir)
        try:
            output.relative_to(build_dir)
        except ValueError:
            continue
        outputs.add(output)

    if not outputs:
        raise ValueError("Ninja reported no build-graph outputs; refusing to create a cache snapshot.")
    return outputs


def read_dirty_outputs(build_dir: Path, workspace: Path) -> set[Path]:
    # Ninja's dry run identifies targets that are missing, out of date, or have
    # changed commands. It does not execute compiler or generator commands.
    command = ["ninja", "-C", str(build_dir), "-n", "-d", "explain"]
    result = subprocess.run(command, cwd=workspace, text=True, capture_output=True)
    if result.returncode:
        detail = result.stderr.strip() or result.stdout.strip()
        raise ValueError(f"Could not identify unbuilt/dirty Ninja outputs: {detail}")

    dirty: set[Path] = set()
    patterns = (
        ("output ", " doesn't exist"),
        ("output ", " older than most recent input "),
        ("output ", " is dirty"),
        # Ninja can also report generated force targets as a bare path:
        # "ninja explain: /.../VerifyGlobs.cmake_force is dirty".
        # Keep this after the prefixed form and validate the resolved path
        # against build_dir below before excluding it from the cache.
        ("", " is dirty"),
        ("command line changed for ", ""),
        ("deps for ", " are missing"),
        ("deps for ", " are out of date"),
    )

    for raw_line in result.stderr.splitlines():
        line = raw_line.strip()
        if not line.startswith("ninja explain: "):
            continue
        message = line[len("ninja explain: "):]
        matched = False
        for prefix, suffix in patterns:
            if message.startswith(prefix) and (not suffix or suffix in message):
                target = message[len(prefix):]
                if suffix:
                    target = target.split(suffix, 1)[0]
                output = target_path(target, build_dir)
                try:
                    output.relative_to(build_dir)
                except ValueError:
                    raise ValueError(
                        f"Ninja reported a dirty output outside the build directory: {target}"
                    )
                dirty.add(output)
                matched = True
                break
        if not matched:
            raise ValueError(
                f"Unrecognized Ninja dirty-target explanation; refusing to cache an uncertain build tree: {message}"
            )

    return dirty


def read_failed_outputs(log_path: Path, workspace: Path, build_dir: Path) -> tuple[set[Path], int]:
    failed: set[Path] = set()
    edge_count = 0

    if not log_path.is_file():
        return failed, edge_count

    with log_path.open("r", encoding="utf-8", errors="replace") as log:
        for raw_line in log:
            line = raw_line.lstrip()
            if not line.startswith("FAILED:"):
                continue

            edge_count += 1
            target_text = line[len("FAILED:") :].strip()
            if not target_text:
                raise ValueError("A FAILED Ninja edge has no listed outputs.")

            try:
                targets = shlex.split(target_text)
            except ValueError as error:
                raise ValueError(f"Could not parse failed Ninja outputs: {error}") from error

            for target in targets:
                output = target_path(target, build_dir)
                try:
                    output.relative_to(build_dir)
                except ValueError as error:
                    raise ValueError(
                        f"Refusing to filter a failed output outside the build directory: {target}"
                    ) from error
                failed.add(output)

    return failed, edge_count


def copy_snapshot(
    source: Path,
    destination: Path,
    graph_outputs: set[Path],
    successful_outputs: set[Path],
    failed_outputs: set[Path],
    dirty_outputs: set[Path],
) -> tuple[int, int, int, int, int]:
    copied_files = 0
    skipped_unbuilt = 0
    skipped_failed = 0
    skipped_dirty = 0
    skipped_stale_compiler_outputs = 0
    destination.mkdir(parents=True, exist_ok=True)

    for current_root, directory_names, file_names in os.walk(source, topdown=True, followlinks=False):
        current = Path(current_root)
        relative_dir = current.relative_to(source)
        destination_root = destination / relative_dir
        destination_root.mkdir(parents=True, exist_ok=True)

        kept_directories: list[str] = []
        for name in directory_names:
            source_item = current / name
            destination_item = destination / source_item.relative_to(source)
            if source_item.is_symlink():
                os.symlink(os.readlink(source_item), destination_item, target_is_directory=True)
            else:
                destination_item.mkdir(parents=True, exist_ok=True)
                kept_directories.append(name)
        directory_names[:] = kept_directories

        for name in file_names:
            source_item = current / name
            destination_item = destination / source_item.relative_to(source)

            in_cmake_target_directory = (
                "CMakeFiles" in source_item.parts
                and any(part.endswith(".dir") for part in source_item.parts)
            )
            if in_cmake_target_directory and source_item.name.endswith((".obj", ".o")):
                if source_item not in graph_outputs:
                    skipped_stale_compiler_outputs += 1
                    continue
            if in_cmake_target_directory and source_item.name.endswith((".obj.d", ".o.d")):
                object_output = source_item.with_name(source_item.name[:-2])
                if object_output not in graph_outputs:
                    skipped_stale_compiler_outputs += 1
                    continue

            if source_item in graph_outputs:
                if source_item in failed_outputs:
                    skipped_failed += 1
                    continue
                if source_item in dirty_outputs:
                    skipped_dirty += 1
                    continue
                if source_item not in successful_outputs:
                    skipped_unbuilt += 1
                    continue

            if source_item.is_symlink():
                os.symlink(os.readlink(source_item), destination_item)
            else:
                shutil.copy2(source_item, destination_item)
            copied_files += 1

    return copied_files, skipped_unbuilt, skipped_failed, skipped_dirty, skipped_stale_compiler_outputs


def main() -> int:
    args = parse_args()
    workspace = Path.cwd().resolve()
    build_dir = workspace_path(args.build_dir, workspace)
    destination = workspace_path(args.destination, workspace)
    log_path = workspace_path(args.log, workspace)
    manifest_path = workspace_path(args.object_manifest, workspace) if args.object_manifest else None

    for label, path in (("Build directory", build_dir), ("Cache destination", destination)):
        try:
            path.relative_to(workspace)
        except ValueError:
            print(f"Refusing {label.lower()} outside the workspace: {path}", file=sys.stderr)
            return 2

    if not build_dir.is_dir():
        print(f"Ninja build directory does not exist: {build_dir}", file=sys.stderr)
        return 2

    if destination == build_dir or destination.is_relative_to(build_dir) or build_dir.is_relative_to(destination):
        print("The cache destination must be separate from the active build directory.", file=sys.stderr)
        return 2

    if manifest_path is not None:
        try:
            manifest_path.relative_to(build_dir)
        except ValueError:
            print(f"Refusing object manifest outside the build directory: {manifest_path}", file=sys.stderr)
            return 2
        if manifest_path.is_symlink():
            print(f"Refusing to replace a symlink object manifest: {manifest_path}", file=sys.stderr)
            return 2

    try:
        successful = read_successful_outputs(build_dir)
        graph_outputs = read_graph_outputs(build_dir, workspace)
        dirty = read_dirty_outputs(build_dir, workspace)
        failed, failed_edges = read_failed_outputs(log_path, workspace, build_dir)
    except (OSError, ValueError) as error:
        print(str(error), file=sys.stderr)
        return 2

    if args.require_failures and failed_edges == 0:
        print("The build failed but its log contains no FAILED Ninja edges; refusing to cache an unfiltered failure.", file=sys.stderr)
        return 2

    # Only existing outputs that Ninja identifies as real build targets need
    # the success-log allowlist. Configure metadata and generated non-target
    # files are copied normally.
    graph_files = {
        path for path in graph_outputs
        if path.is_file() or path.is_symlink()
    }

    eligible_successes = graph_files & successful - dirty - failed
    manifest_objects: list[Path] = []
    if manifest_path is not None:
        object_root = build_dir / "CMakeFiles" / "WebKitWKC.dir"
        manifest_objects = sorted(
            path for path in eligible_successes
            if path.suffix == ".obj" and path.is_relative_to(object_root)
        )
        try:
            manifest_path.parent.mkdir(parents=True, exist_ok=True)
            manifest_path.write_text(
                "".join(f"{path.relative_to(build_dir).as_posix()}\n" for path in manifest_objects),
                encoding="utf-8",
            )
        except OSError as error:
            print(f"Could not write successful WKC object manifest: {error}", file=sys.stderr)
            return 2

    if destination.is_symlink():
        print(f"Refusing to replace a symlink cache destination: {destination}", file=sys.stderr)
        return 2

    try:
        if destination.exists():
            shutil.rmtree(destination)
        copied, skipped_unbuilt, skipped_failed, skipped_dirty, skipped_stale_compiler_outputs = copy_snapshot(
            build_dir, destination, graph_files, successful, failed, dirty
        )

        # These sidecars are control metadata, not Ninja target outputs. Preserve
        # them explicitly so a successful-output snapshot keeps its source baseline
        # and the verified bootstrap object manifest across workflow runs.
        preserved_control_metadata: list[str] = []
        for name in (".wkc-cache-base-sha", ".wkc-bootstrap-objects"):
            source_item = build_dir / name
            if source_item.is_file():
                shutil.copy2(source_item, destination / name)
                preserved_control_metadata.append(name)
    except OSError as error:
        print(f"Could not create successful-output cache snapshot: {error}", file=sys.stderr)
        if destination.exists() and not destination.is_symlink():
            shutil.rmtree(destination, ignore_errors=True)
        return 2

    print(f"Ninja build-graph output files checked: {len(graph_files)}")
    print(f"Successful and currently clean outputs eligible for cache: {len(eligible_successes)}")
    if manifest_path is not None:
        print(f"Successful clean WKC object files written to manifest: {len(manifest_objects)}")
    print(f"Outputs with no successful build record excluded: {skipped_unbuilt}")
    print(f"Failed outputs excluded: {skipped_failed}")
    print(f"Dirty or not-yet-built outputs excluded by Ninja dry run: {skipped_dirty}")
    print(f"Stale compiler object/depfile outputs outside the current Ninja graph excluded: {skipped_stale_compiler_outputs}")
    print(f"Other build metadata and successful outputs copied: {copied}")
    print(f"Control metadata preserved in cache: {', '.join(preserved_control_metadata) if preserved_control_metadata else 'none'}")
    print(f"Active build directory left untouched: {build_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
