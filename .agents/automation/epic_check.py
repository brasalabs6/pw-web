#!/usr/bin/env python3
"""Static validation for the whole epic/workorder tree."""
import json
from pathlib import Path
from catalog import EPICS, check_catalog

MANDATORY = ("README.md", "epic.md", "requirements.md", "architecture.md",
             "implementation.md", "testing.md", "risks.md", "tasks.md")
stats = check_catalog()
for folder in sorted(p for p in EPICS.iterdir() if p.is_dir()):
    for name in MANDATORY:
        f = folder / name
        if not f.exists() or len(f.read_text(encoding="utf-8")) < 120:
            raise SystemExit(f"Missing/underspecified epic file: {f}")
    if not list((folder / "workorders").glob("*.json")):
        raise SystemExit(f"No workorders: {folder}")
print(json.dumps(stats, sort_keys=True))
