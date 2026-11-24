#!/usr/bin/env python3
"""Compile only WKC C++ translation units changed since the last successful build."""
import json
import os
import pathlib
import subprocess
import sys

root = pathlib.Path.cwd()
baseline_file = root / "build/.wkc-cache-base-sha"
baseline = baseline_file.read_text(encoding="utf-8").strip() if baseline_file.is_file() else ""


def has_commit(revision):
    return subprocess.run(
        ["git", "cat-file", "-e", f"{revision}^{{commit}}"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    ).returncode == 0


if baseline and has_commit(baseline):
    compare_from = baseline
    print(f"Checking C++ changes since last successful WKC build: {baseline}")
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
        ["git", "diff", "--name-only", f"{compare_from}..HEAD", "--", "*.cpp"],
        text=True,
    ).splitlines()
except subprocess.CalledProcessError:
    print(f"::error::Cannot determine changed C++ files from {compare_from}.")
    raise SystemExit(1)

changed_cpp = sorted(set(path for path in changed if path.endswith(".cpp")))
if not changed_cpp:
    print("No changed C++ translation units since the last successful WKC build; preflight skipped.")
    raise SystemExit(0)

with (root / "build/compile_commands.json").open(encoding="utf-8") as file:
    commands = json.load(file)

by_source = {}
for entry in commands:
    source = pathlib.Path(entry["file"])
    if not source.is_absolute():
        source = pathlib.Path(entry["directory"]) / source
    by_source[source.resolve()] = entry

failed = []
for relative in changed_cpp:
    source = (root / relative).resolve()
    entry = by_source.get(source)
    if entry is None:
        print(f"Not part of this WKC target; skipping {relative}")
        continue

    print(f"=== Preflight compile: {relative} ===", flush=True)
    command = entry.get("command")
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

    if result.stdout:
        print(result.stdout, end="" if result.stdout.endswith("\n") else "\n")
    if result.stderr:
        print(
            result.stderr,
            end="" if result.stderr.endswith("\n") else "\n",
            file=sys.stderr,
        )
    if result.returncode:
        failed.append((relative, result.returncode))
    else:
        print(f"PASS: {relative}", flush=True)

if failed:
    for source, status in failed:
        print(f"::error::Preflight failed: {source} (exit {status})")
    raise SystemExit(1)

print("All changed WKC C++ translation units passed preflight.")
