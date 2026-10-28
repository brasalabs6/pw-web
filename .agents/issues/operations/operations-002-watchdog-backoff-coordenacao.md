id: operations-002
title: Watchdog sem backoff (crash loop = thrashing) e sem coordenação com boot --stop/--fresh (reinicia daemons durante manutenção)
category: operations
type: gap
status: OPEN
severity: MEDIUM
owner: server153-ops (scripts)
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "scripts/watchdog-server153.sh:60-67 — loop com sleep fixo de 10s, restart imediato sem backoff/jitter, sem contador de falhas"
  - "Sem lock/PID file/sentinel: boot-server153.sh --stop ou --fresh durante o watchdog → watchdog recria tudo em <10s (manutenção interrompida; --fresh pode ser revertido por daemons relidos); janela entre stop e start de cada daemon = segunda instância possível"
impact: "1) Daemon que crasha no boot (ex.: gamedbd com WDB CORRUPT) vira crash loop a cada 10s — CPU/logs desperdiçados, falha persistente mascarada de blip. 2) Operador não consegue parar o stack para manutenção sem matar o watchdog antes. 3) Duas instâncias do mesmo daemon competem por portas/storage."
proposed_action: "1) Backoff exponencial por daemon (10s→20s→…→cap 300s) + alerta após N falhas consecutivas. 2) Sentinel file ($RUN/.booting/.maintenance) que o boot cria e o watchdog respeita (pula o ciclo); ou flock compartilhado."
validation_notes: "boot --stop com watchdog ativo: nada deve renascer até o sentinel ser removido; daemon em crash loop deve desacelerar e alertar, não thrashar."
