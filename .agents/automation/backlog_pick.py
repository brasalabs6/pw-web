#!/usr/bin/env python3
"""Deterministic workorder scheduler. Absence of approvals blocks dependent work."""
from __future__ import annotations
import argparse
import json
import re
from pathlib import Path
from catalog import ROOT, all_workorders, read

AGENT_PR = re.compile(r"^agent: (e[0-9]{2}-[a-z0-9-]+)/(?:(wo-[0-9]{2}))$")
AGENT_PR_SPACED = re.compile(r"^agent: (e[0-9]{2}-[a-z0-9-]+) / (wo-[0-9]{2})$")
MANUAL_PR = re.compile(r"^workorder: (e[0-9]{2}-[a-z0-9-]+)/(wo-[0-9]{2})$")

def completed_from_prs(prs: list) -> set[str]:
    done = set()
    for item in prs:
        title = item.get("title", "")
        m = AGENT_PR.fullmatch(title) or AGENT_PR_SPACED.fullmatch(title) or MANUAL_PR.fullmatch(title)
        if m:
            done.add(m.group(1) + "/" + m.group(2))
    return done

def completed_from_manifest() -> set[str]:
    p = ROOT / ".agents" / "completed-workorders.json"
    data = json.loads(p.read_text(encoding="utf-8"))
    if data.get("schema_version") != 1 or not isinstance(data.get("completed"), dict):
        raise ValueError("Invalid manual completion manifest")
    completed = set()
    for key, value in data["completed"].items():
        if not isinstance(value, dict) or not str(value.get("evidence_url", "")).startswith("https://github.com/") or not value.get("reviewer"):
            raise ValueError(f"Missing evidence/reviewer in completed workorder {key}")
        epic, sep, wo = key.partition("/")
        if not sep:
            raise ValueError(f"Invalid completed key {key}")
        read(epic, wo)
        completed.add(key)
    return completed

def eligibles(done: set[str]) -> list[dict]:
    result = []
    for wo in all_workorders():
        name = wo["epic"] + "/" + wo["id"]
        if wo["automation"] and name not in done and set(wo["dependencies"]).issubset(done):
            result.append(wo)
    return result

def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument("--merged", required=True, help="JSON list of merged PRs")
    p.add_argument("--require", help="Ensure a manually selected workorder has all completed dependencies")
    p.add_argument("--github-output")
    a = p.parse_args()
    prs = json.loads(Path(a.merged).read_text(encoding="utf-8"))
    if not isinstance(prs, list):
        raise SystemExit("Merged PR input must be a JSON array")
    done = completed_from_manifest() | completed_from_prs(prs)
    ready = eligibles(done)
    if a.require:
        epic, slash, wo = a.require.partition("/")
        if not slash:
            raise SystemExit("Use epic/workorder")
        read(epic, wo)
        match = next((x for x in ready if x["epic"] + "/" + x["id"] == a.require), None)
        if match is None:
            raise SystemExit("Workorder not eligible: manual gate, unfinished dependency or already completed")
        result = match
    else:
        result = ready[0] if ready else None
    if result:
        print(f'Selected {result["epic"]}/{result["id"]}')
        if a.github_output:
            with open(a.github_output, "a", encoding="utf-8") as f:
                f.write(f'epic={result["epic"]}\nworkorder={result["id"]}\nselected=true\n')
    else:
        print("No eligible workorder")
        if a.github_output:
            with open(a.github_output, "a", encoding="utf-8") as f:
                f.write("selected=false\n")

if __name__ == "__main__":
    main()
