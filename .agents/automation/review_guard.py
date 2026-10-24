#!/usr/bin/env python3
"""Strict identity gate before any review or access to model credentials."""
import argparse, json, re
from pathlib import Path
from review_loop import guard_pr

p=argparse.ArgumentParser()
p.add_argument("--pr-file",required=True)
p.add_argument("--sha",required=True)
p.add_argument("--repo",required=True)
p.add_argument("--round",type=int,required=True)
p.add_argument("--commit-message-file",required=True)
p.add_argument("--output",required=True)
a=p.parse_args()
pr=json.loads(Path(a.pr_file).read_text(encoding="utf-8"))
epic, wo=guard_pr(pr,a.sha,a.repo,a.round)
message=Path(a.commit_message_file).read_text(encoding="utf-8")
m=re.search(r"\[agent-attempt:(\d+)\]",message)
if not m or int(m.group(1))!=a.round:
    raise SystemExit("Not a matching agent-produced commit for requested round")
with open(a.output,"a",encoding="utf-8") as out:
    for key, value in {"sha":a.sha,"round":a.round,"epic":epic,"workorder":wo,
                       "branch":pr["head"]["ref"],"pr_number":pr["number"]}.items():
        out.write(f"{key}={value}\n")
print(f"Allowed review of #{pr['number']} SHA={a.sha[:12]} round={a.round}")
