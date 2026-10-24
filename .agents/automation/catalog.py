#!/usr/bin/env python3
"""Validated workorders: trust only repo-controlled canonical paths and JSON contracts."""
from __future__ import annotations
import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EPICS = ROOT / ".agents" / "epics"
ID = re.compile(r"^[a-z][a-z0-9-]{2,70}$")
DENIED = (".github/", ".agents/", ".grok/", "references/", "assets/original/", "data/original/", "game-data/", ".git/", "scripts/verify.sh", ".gitmodules", "AGENTS.md", "SECURITY.md", ".env", "LICENSE")

def read(epic: str, workorder: str) -> dict:
    if not ID.fullmatch(epic) or not ID.fullmatch(workorder):
        raise ValueError("Invalid epic/workorder ID; expected lowercase ASCII slug")
    folder = EPICS / epic
    path = folder / "workorders" / (workorder + ".json")
    if not path.is_file():
        raise ValueError(f"Unknown workorder: {epic}/{workorder}")
    data = json.loads(path.read_text(encoding="utf-8"))
    if data.get("epic") != epic or data.get("id") != workorder:
        raise ValueError(f"Workorder identity mismatch: {path}")
    required = ("title", "objective", "deliverables", "acceptance", "tests", "dependencies", "write_scopes", "automation", "risk")
    for key in required:
        if key not in data:
            raise ValueError(f"{path}: missing {key}")
    if not isinstance(data["automation"], bool):
        raise ValueError(f"{path}: automation must be boolean")
    if not isinstance(data["write_scopes"], list) or not data["write_scopes"]:
        raise ValueError(f"{path}: missing scopes")
    for scope in data["write_scopes"]:
        if not isinstance(scope, str) or not scope.endswith("/") or scope.startswith("/") or ".." in scope.split("/") or scope.startswith(DENIED):
            raise ValueError(f"Unsafe write scope {scope!r} in {path}")
    if not isinstance(data["dependencies"], list):
        raise ValueError(f"{path}: dependencies must be list")
    return data

def all_workorders() -> list[dict]:
    found = []
    for p in sorted(EPICS.glob("*/workorders/*.json")):
        found.append(read(p.parent.parent.name, p.stem))
    return found

def path_allowed(path: str, data: dict) -> bool:
    if not path or path.startswith("/") or "\\" in path or ".." in path.split("/"):
        return False
    if path.startswith(DENIED):
        return False
    if path.endswith((".pck", ".pkx", ".dds", ".ecm", ".bon", ".ski", ".smd", ".exe", ".dll", ".wasm")):
        return False
    return any(path.startswith(scope) for scope in data["write_scopes"])

def check_catalog() -> dict:
    items = all_workorders()
    known = {f'{w["epic"]}/{w["id"]}' for w in items}
    if len(known) != len(items):
        raise ValueError("Duplicate workorder IDs")
    for w in items:
        for dep in w["dependencies"]:
            if dep not in known:
                raise ValueError(f'{w["epic"]}/{w["id"]}: unknown dependency {dep}')
        for key in ("deliverables", "acceptance", "tests"):
            if not isinstance(w[key], list) or not w[key]:
                raise ValueError(f'{w["epic"]}/{w["id"]}: empty {key}')
        if not isinstance(w["objective"], str) or len(w["objective"]) < 30:
            raise ValueError(f'{w["epic"]}/{w["id"]}: objective too short')
    return {"epics": len({w["epic"] for w in items}), "workorders": len(items),
            "automatable": sum(w["automation"] for w in items)}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--validate", action="store_true")
    parser.add_argument("--epic")
    parser.add_argument("--workorder")
    args = parser.parse_args()
    if args.validate:
        print(json.dumps(check_catalog(), indent=2))
    else:
        print(json.dumps(read(args.epic, args.workorder), indent=2, ensure_ascii=False))
