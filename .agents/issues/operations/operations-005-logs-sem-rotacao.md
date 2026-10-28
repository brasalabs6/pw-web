id: operations-005
title: Logs sem rotação nem limite — logservice só reabre no SIGHUP; no container CF o append cresce até ENOSPC
category: operations
type: gap
status: OPEN
severity: MEDIUM
owner: server153-ops
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/logservice/logservice.cpp:48-57 — OnSIGHUP faz closefiles+openfiles (reabre os MESMOS arquivos; sem rename/truncate); logdispatch.h:28 — O_APPEND|O_NONBLOCK; h:81 — retorno de write() ignorado"
  - "cloudflare/entrypoint.sh:79-80 e scripts/watchdog-server153.sh:49-57 — >> append sem rotação; nenhum logrotate no repo"
impact: "Container CF rodando dias/semanas: logs enchem o storage efêmero → ENOSPC → daemons falham escritas silenciosamente (write ignorado) e a plataforma reinicia o container. Local: world2.* crescem sem limite. Falha de open() também é silenciosa (diretório ausente = logs perdidos sem alerta)."
proposed_action: "1) Rotação por tamanho no logservice (reabrir+rename quando > N MB) ou logrotate externo + SIGHUP documentado. 2) No entrypoint: truncador periódico (tail -c) ou coleta via stdout da plataforma. 3) Checar retorno de write()/open() e logar falha."
validation_notes: "Rodar 1h com tráfego e medir du dos logs; forçar disco cheio em teste → erro logado, não silêncio."

## Progresso
- Progresso 2026-10-28: rotação copytruncate implementada no cloudflare/entrypoint.sh (logs > 20MB truncados para os últimos 10MB a cada 30min, preservando os fds de append). Pendente: rotação do logservice interno (SIGHUP) e checagem do retorno de write().
