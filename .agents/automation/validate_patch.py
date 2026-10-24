#!/usr/bin/env python3
"""Independent patch review before any credentials with write access are used."""
import argparse
import re
import subprocess
from pathlib import Path
from catalog import path_allowed, read

def check(patch: str, workorder: dict) -> list[str]:
    if not patch or len(patch.encode("utf-8")) > 256_000:
        raise ValueError("Empty or oversized patch (limit 256 KiB)")
    if "GIT binary patch" in patch or re.search(r"^new file mode 120000$", patch, re.M) or re.search(r"^new file mode 160000$", patch, re.M):
        raise ValueError("Binary, symlink and submodule changes are forbidden")
    if re.search(r"^(rename|copy) (from|to) ", patch, re.M):
        raise ValueError("Renames/copies are not allowed")
    proc = subprocess.run(["git", "apply", "--numstat", "-z"], input=patch.encode(), capture_output=True, check=True)
    paths = []
    for record in proc.stdout.split(b"\x00"):
        if not record:
            continue
        fields = record.split(b"\t", 2)
        if len(fields) != 3:
            raise ValueError("Unrecognized patch path encoding")
        path = fields[2].decode("utf-8", errors="strict")
        if not path_allowed(path, workorder):
            raise ValueError(f"Patch touches forbidden or undeclared path: {path!r}")
        paths.append(path)
    if not paths or len(paths) > 24:
        raise ValueError("Patch has zero or too many files (>24)")
    return paths

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--epic", required=True)
    p.add_argument("--workorder", required=True)
    p.add_argument("--patch", required=True)
    args = p.parse_args()
    paths = check(Path(args.patch).read_text(encoding="utf-8"), read(args.epic, args.workorder))
    print("Approved patch paths: " + ", ".join(paths))

if __name__ == "__main__":
    main()
