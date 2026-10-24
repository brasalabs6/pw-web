#!/usr/bin/env python3
"""Write an immutable CI evidence summary; no game functionality claimed."""
import argparse
import json
import os
from pathlib import Path
from datetime import datetime, timezone
p = argparse.ArgumentParser()
p.add_argument("--log", required=True)
p.add_argument("--status", required=True)
p.add_argument("--output", required=True)
a = p.parse_args()
payload = {"schema_version": 1, "status": a.status, "sha": os.getenv("GITHUB_SHA", ""),
           "run_id": os.getenv("GITHUB_RUN_ID", ""), "workflow": os.getenv("GITHUB_WORKFLOW", ""),
           "utc": datetime.now(timezone.utc).isoformat(),
           "log": a.log, "scope": "CI bootstrap + epic contracts; no game client was tested"}
Path(a.output).parent.mkdir(parents=True, exist_ok=True)
Path(a.output).write_text(json.dumps(payload, indent=2), encoding="utf-8")
print(json.dumps(payload))
