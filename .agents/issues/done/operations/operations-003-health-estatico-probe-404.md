id: operations-003
title: Deploy Cloudflare cego — /health é JSON estático sempre ok:true (o CGI dinâmico nunca é usado) e /probe dá 404→503 "starting" permanente
category: operations
type: bug
status: DONE
severity: HIGH
owner: server153-deploy
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "cloudflare/entrypoint.sh:21 — echo '{\"ok\":true,...}' > /www/health (arquivo ESTÁTICO); o CGI dinâmico /www/cgi-bin/health.sh (conta daemons) existe mas o worker.js:77-83 faz proxy de /health para o arquivo estático — nunca referencia o CGI"
  - "cloudflare/worker.js:78 — !response.ok → 3 retries → 503 {\"starting\":true}; /probe não existe no busybox httpd (404) → /probe SEMPRE 503 'starting'"
impact: "Daemon morto no container: /health continua 200 ok:true — monitoramento e orquestração recebem falso positivo permanente; /probe reporta 'iniciando' para sempre. A validação de deploy atual depende de alguém chamar /cgi-bin/health.sh manualmente."
proposed_action: "RESOLVIDO: worker agora faz proxy de /health para o CGI dinâmico /cgi-bin/health.sh (contando 8 daemons + gs-ours); o arquivo estático foi removido; /probe foi removido do worker (404); /tcpprobe segue como probe forte de mediação."
validation_notes: "VALIDADO 2026-10-28: GET /health publica responde {ok:true,daemons_alive:8,gs_alive:1,expected_daemons:8,expected_gs_min:1}; o CGI conta processos reais (zombies não contam), então daemon/gs morto reflete ok:false."

closure_reason: "Health dinâmico implantado: worker.js proxya /health ao CGI que conta daemons+gs; validado publicamente no deploy 3127296f/bc194134."
resolved_in: "commit do branch feat/pw-source-1.5.3-ci-build (deploy Cloudflare 2026-10-28, worker version bc194134)"
