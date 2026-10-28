id: operations-003
title: Deploy Cloudflare cego — /health é JSON estático sempre ok:true (o CGI dinâmico nunca é usado) e /probe dá 404→503 "starting" permanente
category: operations
type: bug
status: OPEN
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
proposed_action: "1) Worker faz proxy de /health para /cgi-bin/health.sh (contagem dinâmica), ou o entrypoint substitui o arquivo estático por CGI. 2) Remover /probe do worker OU criar cgi-bin/probe.sh (nc -z 127.0.0.1 29000). Manter /tcpprobe (mediation) como probe forte."
validation_notes: "Matar um daemon no container de teste: /health deve virar ok:false em <60s; /probe (ou sua remoção) não deve mentir sobre o estado."
