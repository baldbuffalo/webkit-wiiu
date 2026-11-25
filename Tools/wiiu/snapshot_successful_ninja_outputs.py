#!/usr/bin/env python3
"""Create a cache snapshot while excluding outputs of failed Ninja edges.

The active build directory is read-only to this script. Failed outputs are
identified from Ninja's FAILED lines and skipped during the copy, so they never
enter the cache snapshot. All other build files and CMake/Ninja metadata are
preserved for incremental reuse.
"""

from __future__ import annotations

import argparse
import os
import shlex
import shutil
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
    return parser.parse_args()


def failed_output_paths(log_path: Path, workspace: Path, build_dir: Path) -> tuple[set[Path], int]:
    failed: set[Path] = set()
    edge_count = 0

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
                output = Path(target)
                if not output.is_absolute():
                    # Ninja may print paths relative to either the workspace or
                    # the build directory, depending on how CMake invoked it.
                    if output.parts and output.parts[0] == build_dir.name:
                        output = workspace / output
                    else:
                        output = build_dir / output

                output = Path(os.path.abspath(output))
                try:
                    output.relative_to(build_dir)
                except ValueError as error:
                    raise ValueError(
                        f"Refusing to filter a failed output outside the build directory: {target}"
                    ) from error

                failed.add(output)

    return failed, edge_count


def copy_snapshot(source: Path, destination: Path, excluded: set[Path]) -> tuple[int, int]:
    copied_files = 0
    excluded_files = 0

    destination.mkdir(parents=True, exist_ok=True)

    for current_root, directory_names, file_names in os.walk(source, topdown=True, followlinks=False):
        current = Path(current_root)
        relative_dir = current.relative_to(source)
        destination_root = destination / relative_dir
        destination_root.mkdir(parents=True, exist_ok=True)

        kept_directories: list[str] = []
        for name in directory_names:
            source_item = current / name
            relative_item = source_item.relative_to(source)
            destination_item = destination / relative_item

            if source_item in excluded:
                excluded_files += 1
                continue

            if source_item.is_symlink():
                os.symlink(os.readlink(source_item), destination_item, target_is_directory=True)
            else:
                destination_item.mkdir(parents=True, exist_ok=True)
                kept_directories.append(name)

        directory_names[:] = kept_directories

        for name in file_names:
            source_item = current / name
            destination_item = destination / source_item.relative_to(source)

            if source_item in excluded:
                excluded_files += 1
                continue

            if source_item.is_symlink():
                os.symlink(os.readlink(source_item), destination_item)
            else:
                shutil.copy2(source_item, destination_item)
            copied_files += 1

    return copied_files, excluded_files


def main() -> int:
    args = parse_args()
    workspace = Path.cwd().resolve()
    build_dir = workspace_path(args.build_dir, workspace)
    destination = workspace_path(args.destination, workspace)
    log_path = workspace_path(args.log, workspace)

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

    if not log_path.is_file():
        if args.require_failures:
            print(f"Failed build log is missing; refusing to create a cache snapshot: {log_path}", file=sys.stderr)
            return 2
        failed = set()
        edge_count = 0
    else:
        try:
            failed, edge_count = failed_output_paths(log_path, workspace, build_dir)
        except ValueError as error:
            print(str(error), file=sys.stderr)
            return 2

    if args.require_failures and edge_count == 0:
        print("The build failed but the Ninja log contains no FAILED outputs; refusing to cache an unfiltered snapshot.", file=sys.stderr)
        return 2

    if destination.is_symlink():
        print(f"Refusing to replace a symlink cache destination: {destination}", file=sys.stderr)
        return 2

    try:
        if destination.exists():
            shutil.rmtree(destination)
        copied, excluded = copy_snapshot(build_dir, destination, failed)
    except OSError as error:
        print(f"Could not create successful-output cache snapshot: {error}", file=sys.stderr)
        if destination.exists() and not destination.is_symlink():
            shutil.rmtree(destination, ignore_errors=True)
        return 2

    print(f"Failed Ninja edges identified: {edge_count}")
    print(f"Failed outputs excluded before copying: {len(failed)}")
    print(f"Files and symlinks copied into cache snapshot: {copied}")
    print(f"Failed output entries skipped: {excluded}")
    print(f"Active build directory left untouched: {build_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
