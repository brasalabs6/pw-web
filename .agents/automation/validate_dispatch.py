#!/usr/bin/env python3
"""Fail closed on untrusted dispatch identifiers and attempt limits."""
import argparse
import os
from pathlib import Path
from catalog import read

p = argparse.ArgumentParser()
p.add_argument("--epic", required=True)
p.add_argument("--workorder", required=True)
p.add_argument("--attempt", type=int, required=True)
p.add_argument("--output", required=True)
a = p.parse_args()
wo = read(a.epic, a.workorder)
if not wo["automation"]:
    raise SystemExit("This workorder requires human-led execution (automation=false)")
if not 1 <= a.attempt <= 30:
    raise SystemExit("attempt must be between 1 and 30")
branch = f"agent/{a.epic}/{a.workorder}"
ref = "main" if a.attempt == 1 else branch
with open(a.output, "a", encoding="utf-8") as out:
    for key, value in {"branch": branch, "checkout_ref": ref, "attempt": a.attempt,
                       "epic": a.epic, "workorder": a.workorder}.items():
        out.write(f"{key}={value}\n")
print(f"Validated {a.epic}/{a.workorder} attempt={a.attempt}; base={ref}")
