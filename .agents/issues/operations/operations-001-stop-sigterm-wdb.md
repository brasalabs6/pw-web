id: operations-001
title: boot --stop envia SIGTERM para gamedbd/uniquenamed — upstream usa SIGUSR1 (graceful); risco de CORRUPT no próximo boot
category: operations
type: bug
status: OPEN
severity: HIGH
owner: server153-ops (scripts)
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "scripts/boot-server153.sh:49-56 — stop_all() envia kill (SIGTERM) para TODOS os daemons, incluindo os de storage"
  - "src/server153/cnet/killgame (upstream) — killall -SIGUSR1 gamedbd.wdb uniquenamed.wdb; share/common/thread.cpp:110-117 — SIGUSR1 → OnQuit/stateQuitAtOnce (checkpoint+flush); SIGTERM não está no sigwait set (thread.cpp:168-177) = terminate imediato sem checkpoint"
  - "scripts/boot-server153.sh:139 (comentário do próprio script) — 'the WDB engine has no CORRUPT-state recovery, so state left by a hard kill can permanently break gamedbd startup'"
impact: "Todo --stop hoje é um hard kill para o banco: sem checkpoint final, o próximo boot pode encontrar estado inconsistente → CORRUPT fatal (data-006) → wipe --fresh = perda de personagens. É o gatilho operacional mais provável do pior cenário de dados."
proposed_action: "stop_all: SIGUSR1 para gamedbd e uniquenamed com timeout (ex.: 5s) e SIGKILL de fallback; SIGTERM para os demais. Documentar em docs/server/06-OPERATIONS.md."
validation_notes: "boot --stop seguido de boot: gamedbd sobe limpo com checkpoint no log (DbPolicy::OnQuit); nenhum CORRUPT."
