#!/usr/bin/env python3
"""Remove outputs from failed Ninja edges before persisting an incremental build tree.

Ninja records successful edges in .ninja_log. Failed edges are not recorded as
completed, but a compiler or custom command may leave a partial output behind.
Removing the named FAILED outputs makes the next Ninja invocation rebuild only
those failed edges while retaining successful object files.
"""

from __future__ import annotations

import argparse
import shlex
import sys
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", default="build", help="Ninja build directory")
    parser.add_argument("--log", required=True, help="Ninja output log containing FAILED lines")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    workspace = Path.cwd().resolve()
    build_dir = Path(args.build_dir)
    if not build_dir.is_absolute():
        build_dir = workspace / build_dir
    build_dir = build_dir.resolve()

    try:
        build_dir.relative_to(workspace)
    except ValueError:
        print(f"Refusing to clean outside the workspace: {build_dir}", file=sys.stderr)
        return 2

    log_path = Path(args.log)
    if not log_path.is_absolute():
        log_path = workspace / log_path
    if not log_path.is_file():
        print(f"No Ninja log to clean; leaving build outputs unchanged: {log_path}")
        return 0

    failed_outputs: set[Path] = set()
    failed_edges = 0

    with log_path.open("r", encoding="utf-8", errors="replace") as log:
        for raw_line in log:
            line = raw_line.lstrip()
            if not line.startswith("FAILED:"):
                continue

            failed_edges += 1
            target_text = line[len("FAILED:") :].strip()
            if not target_text:
                print("A FAILED Ninja edge has no listed output; refusing to cache an ambiguous build tree.", file=sys.stderr)
                return 2

            try:
                targets = shlex.split(target_text)
            except ValueError as error:
                print(f"Could not parse failed Ninja outputs: {error}", file=sys.stderr)
                return 2

            for target in targets:
                output = Path(target)
                if not output.is_absolute():
                    # Ninja -C build normally prints paths relative to build/.
                    # Also accept paths that already include the build/ prefix.
                    if output.parts and output.parts[0] == build_dir.name:
                        output = workspace / output
                    else:
                        output = build_dir / output

                output = output.resolve()
                try:
                    output.relative_to(build_dir)
                except ValueError:
                    print(f"Refusing to remove a failed output outside the build tree: {target}", file=sys.stderr)
                    return 2

                failed_outputs.add(output)

    removed = 0
    for output in sorted(failed_outputs):
        if output.is_symlink() or output.is_file():
            try:
                output.unlink()
                removed += 1
            except OSError as error:
                print(f"Could not remove failed Ninja output {output}: {error}", file=sys.stderr)
                return 2

    print(f"Failed Ninja edges found: {failed_edges}")
    print(f"Failed output files removed before caching: {removed}")
    print("Successful object files and their Ninja success records were left untouched.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
