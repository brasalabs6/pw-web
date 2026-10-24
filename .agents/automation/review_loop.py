#!/usr/bin/env python3
"""Trusted CI-side parser and state machine for two independent advisory Grok reviews."""
from __future__ import annotations
import argparse
import json
import re
from pathlib import Path

ROLES = ("deepseek", "glm-flash")
SHA = re.compile(r"^[0-9a-f]{40}$")
BRANCH = re.compile(r"^agent/([a-z][a-z0-9-]{2,70})/(wo-[0-9]{2})$")
SEVERITY = {"critical", "high", "medium", "low"}
MAX_FINDINGS = 30
HARD_MAX = 30

def guard_pr(pr: dict, sha: str, repo: str, round_number: int) -> tuple[str, str]:
    if not SHA.fullmatch(sha) or not 1 <= round_number <= HARD_MAX:
        raise ValueError("Invalid PR SHA or round")
    branch = pr.get("head", {}).get("ref", "")
    match = BRANCH.fullmatch(branch)
    if not match or pr.get("base", {}).get("ref") != "main" or pr.get("state") != "open":
        raise ValueError("Only open PW Web agent PRs targeting main are allowed")
    if pr.get("draft"):
        raise ValueError("PR is still draft; agent must request review")
    if pr["head"].get("sha") != sha or pr["head"].get("repo", {}).get("full_name") != repo:
        raise ValueError("Head SHA or repository mismatch; reject stale/untrusted event")
    return match.group(1), match.group(2)

def parse_review(result_file: Path, changed_files: list[str]) -> dict:
    envelope = json.loads(result_file.read_text(encoding="utf-8"))
    if not isinstance(envelope, dict) or not isinstance(envelope.get("text"), str):
        raise ValueError("Missing Grok text in JSON envelope")
    raw = envelope["text"].strip()
    if raw.startswith("```"):
        raw = re.sub(r"^\`\`\`(?:json)?\s*|\s*\`\`\`$", "", raw).strip()
    if len(raw) > 90000:
        raise ValueError("Review result too long")
    review = json.loads(raw)
    if not isinstance(review, dict):
        raise ValueError("Review must be JSON object")
    decision = review.get("decision")
    if decision not in ("approved", "changes_requested"):
        raise ValueError("Invalid review decision")
    findings = review.get("findings")
    if not isinstance(findings, list) or len(findings) > MAX_FINDINGS:
        raise ValueError("Invalid finding list")
    if not isinstance(review.get("summary"), str) or not review["summary"].strip():
        raise ValueError("Missing summary")
    allowed = set(changed_files)
    clean = []
    for finding in findings:
        if not isinstance(finding, dict):
            raise ValueError("Non-object finding")
        sev = finding.get("severity")
        path = finding.get("path")
        if sev not in SEVERITY or not isinstance(path, str) or path not in allowed:
            raise ValueError(f"Invalid severity or finding path {path!r}")
        line = finding.get("line")
        if line is not None and (type(line) is not int or line < 1 or line > 10000000):
            raise ValueError("Bad line number")
        item = {"severity": sev, "path": path, "line": line}
        for key in ("title", "detail", "recommendation"):
            val = finding.get(key)
            if not isinstance(val, str) or not val.strip() or len(val) > 1100:
                raise ValueError("Invalid finding text")
            item[key] = val
        clean.append(item)
    if decision == "approved" and any(x["severity"] in ("critical", "high", "medium") for x in clean):
        raise ValueError("Approval contradicts blocking findings")
    if decision == "changes_requested" and not clean:
        raise ValueError("Changes requested without actionable findings")
    for field in ("tests_assessed", "residual_risks"):
        arr = review.get(field, [])
        if not isinstance(arr, list) or len(arr) > 20 or any(not isinstance(v, str) or len(v) > 650 for v in arr):
            raise ValueError(f"Invalid {field}")
    return {"decision": decision, "summary": review["summary"][:1500], "findings": clean,
            "tests_assessed": review.get("tests_assessed", []),
            "residual_risks": review.get("residual_risks", [])}

def status(reviews: dict, ci: str, round_number: int, loop_enabled: bool, max_rounds: int) -> str:
    if set(reviews) != set(ROLES) or ci not in ("success", "failure"):
        return "blocked"
    if all(reviews[k]["decision"] == "approved" for k in ROLES) and ci == "success":
        return "human-review"
    if loop_enabled and round_number < min(max(1, max_rounds), HARD_MAX):
        return "repair"
    return "blocked"

def format_comment(reviews: dict, sha: str, round_number: int, ci: str, decision: str, run_id: str) -> str:
    marker = f"<!-- pw-agent-review sha={sha} round={round_number} -->"
    out = [marker, f"## Revisão automática dupla — rodada {round_number}",
           f"**Commit:** \`{sha}\` · **CI:** \`{ci}\` · **Próxima ação:** \`{decision}\`",
           f"**Evidências:** workflow run #{run_id} (artefatos por revisor).",
           "Estas são análises de agentes, não aprovações formais do GitHub."]
    for role in ROLES:
        data = reviews.get(role)
        out += [f"### {role} — " + (data["decision"] if data else "ERRO DE REVISÃO")]
        if not data:
            out.append("Resultado ausente/inválido; revisão humana necessária.")
            continue
        out.append(data["summary"].replace("@", "@\u200b"))
        if not data["findings"]:
            out.append("Nenhum finding acionável reportado.")
        for n, f in enumerate(data["findings"], 1):
            loc = f["path"] + (f":{f['line']}" if f["line"] else "")
            out.append(f"{n}. **[{f['severity']}]** \`{loc}\` — {f['title'].replace('@','@\u200b')}\n   {f['detail'].replace('@','@\u200b')}\n   **Correção/teste:** {f['recommendation'].replace('@','@\u200b')}")
        if data["tests_assessed"]:
            out.append("**Testes analisados:** " + "; ".join(x.replace("@","@\u200b") for x in data["tests_assessed"]))
        if data["residual_risks"]:
            out.append("**Riscos:** " + "; ".join(x.replace("@","@\u200b") for x in data["residual_risks"]))
    return "\n\n".join(out)[:64000] + "\n"

def main():
    p = argparse.ArgumentParser()
    p.add_argument("--deepseek", required=True)
    p.add_argument("--glm", required=True)
    p.add_argument("--changed", required=True)
    p.add_argument("--sha", required=True)
    p.add_argument("--round", type=int, required=True)
    p.add_argument("--ci", required=True)
    p.add_argument("--enabled", default="false")
    p.add_argument("--max-rounds", default="8")
    p.add_argument("--run-id", required=True)
    p.add_argument("--comment", required=True)
    p.add_argument("--output", required=True)
    a = p.parse_args()
    paths = json.loads(Path(a.changed).read_text(encoding="utf-8"))
    if not isinstance(paths, list) or any(not isinstance(p, str) for p in paths):
        raise SystemExit("Invalid changed files list")
    reviews = {}
    errors = {}
    for role, loc in [("deepseek", a.deepseek), ("glm-flash", a.glm)]:
        try:
            reviews[role] = parse_review(Path(loc), paths)
        except (ValueError, OSError, json.JSONDecodeError) as exc:
            errors[role] = str(exc)
    max_rounds = int(a.max_rounds) if str(a.max_rounds).isdigit() else 8
    next_action = status(reviews, a.ci, a.round, a.enabled == "true", max_rounds)
    comment = format_comment(reviews, a.sha, a.round, a.ci, next_action, a.run_id)
    if errors:
        comment += "\n**Falhas de validação:** " + "; ".join(f"{k}: {v[:200]}" for k,v in errors.items()) + "\n"
    Path(a.comment).write_text(comment, encoding="utf-8")
    with open(a.output, "a", encoding="utf-8") as out:
        out.write(f"action={next_action}\n")
        out.write(f"next_round={a.round+1}\n")
    print(json.dumps({"action": next_action,"ci":a.ci,"round":a.round,
                      "valid_reviews": list(reviews), "error_count":len(errors)}))

if __name__ == "__main__":
    main()
