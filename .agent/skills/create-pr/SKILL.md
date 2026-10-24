---
name: create-pr
description: Mandatory PW Web pull-request standard. Use before any agent or human creates, updates, requests review of, or completes a pull request; ensure scoped changes, reproducible evidence, security/legal gates and human review.
version: 1.0.0
---

# Create PR — PW Web canonical workflow

**Triggers:** "create PR", "abrir PR", "pull request", "PR ready", "solicitar review", "finish workorder", "publish changes" and automated workorder completion. This convention applies to **all** authors and assistants. Read `AGENTS.md`, `docs/LEGAL.md`, and the relevant `.agents/epics/.../workorders/wo-XX.json` first.

## 1. Do not overwrite parallel work

1. Fetch the current `main`, inspect current branch/PR/HEAD, and use a dedicated branch. Before updating an existing PR, **respect its state and its owner's WIP note**. An incomplete PR stays draft unless its owner explicitly declares it ready.
2. Use `git status --short`, `git diff --name-status main...HEAD`, `git diff --check` and review the entire patch. Do not rewrite other people's work or force-push a shared branch. Prefer separate scoped commits; abort on stale SHA and reconcile.
3. Enforce `write_scopes` and `forbidden` for agent workorders. Never commit `references/` copies, game assets, secrets, tokens, `.env`, generated WASM/native executables or research artifacts.
4. If branch differs from current `main`, reconcile consciously and preserve upstream workflows (including Grok/Chrome/agent review), not an old snapshot. Re-run tests after the merge.

## 2. Evidence before status

- State **Hypothesis → Implementation → Evidence → Limitations**. Include exact test commands, outcome `PASS / FAIL / BLOCKED / NOT RUN`, version and commit SHA when relevant.
- `Bootstrap CI` passing **does not** prove POC-001's original Angelica/WASM path, which is opt-in. Differentiate unit comparison tests, an independent Chrome WebGL smoke, a local authorized Angelica probe, and actual engine client/gameplay.
- Renderer/viewport/UI PRs: include Chrome desktop and mobile artifacts and WebGL context/driver details; software SwiftShader is not a GPU perf benchmark. Browser host `8480`, opt-in Angelica probe `8481`. Never use 8080.
- **Direitos autorais / rights gate:** for rights-gated work, do not claim original assets/code may be redistributed; cite `docs/LEGAL.md` and label unresolved distribution gate `BLOCKED`.
- Do not weaken failing tests, invent logs/screenshots, or classify unrun steps as pass. If a toolchain is missing, mark `BLOCKED`, not success.
- Include reproduction, negative tests, rollback, risk, dependencies, any required manual gate, and explicit acceptance criteria.

## 3. Naming, PR body and lifecycle

- Title: `feat(area): ...`, `fix(area): ...`, `docs(area): ...`, `test(area): ...`, or `agent: eXX-slug / wo-XX` for orchestrated workorders.
- Use [`.github/PULL_REQUEST_TEMPLATE.md`](../../../.github/PULL_REQUEST_TEMPLATE.md) or reproduce **all** its headings when using `gh pr create --body-file`. Fill in real evidence and explicit placeholders for what remains blocked.
- **Default to draft** if the workorder, source rights, CI, original baseline or verification remains incomplete; do not remove draft status without the PR owner's instruction and required gates.
- Workorder PRs created by `agent-execute.yml` must stay isolated: **the Grok coder cannot run `git push` or `gh pr create`**. Only the privileged `publish` job validates patch scopes, creates/updates draft PR, dispatches CI and (when eligible) agent reviewers.
- `feat/*` / manually created research PRs are **not** automatically processed by `Agent Dual Review`, which requires a trusted `agent/*` workorder branch. Do not fake the branch or bypass the guard. Human reviewers may independently run DeepSeek and GLM on the diff in read-only sessions, recording real findings in GitHub comments; no automated review should be claimed without its run URLs.
- No reviewer can auto-approve/merge own code. `main` merge stays a **human action** and rights gates are independent of test success.

## 4. Before submitting

- [ ] Scope and branch base verified; no parallel work overwritten.
- [ ] Test and negative-test commands recorded with observed results.
- [ ] CI job links (or **not yet executed**) provided.
- [ ] Browser/renderer evidence distinguished from gameplay/Angelica evidence.
- [ ] License, provenance and no-secret checks performed.
- [ ] Workorder/manual dependencies and rollout/rollback documented.
- [ ] PR review policy met, WIP explicitly marked, **no self-merge**.

**After creating/updating PR:** return link, title, branch, commit SHA, observed checks and remaining blockers to the requester. Never promise a subsequent review/CI pass that has not occurred.
