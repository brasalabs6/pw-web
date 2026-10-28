id: security-008
title: Deploy Cloudflare — /tcpprobe público sem rate limit, Worker faz proxy de paths arbitrários, container roda como root
category: security
type: gap
status: OPEN
severity: MEDIUM
owner: server153-deploy
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "cloudflare/worker.js:59-67 — /tcpprobe abre getTcpPort(29000).connect() por request sem auth/rate limit; glinkd.conf:58 listen_backlog=10 (satura fácil)"
  - "cloudflare/worker.js:77 — port.fetch(\"http://container\" + url.pathname) repassa QUALQUER path ao httpd do container (sem whitelist)"
  - "cloudflare/Dockerfile — sem USER; entrypoint.sh:13 `export USER=builder` é env var, não usuário — daemons e CGI rodam como root"
impact: "Atacante público satura o backlog do glinkd via /tcpprobe (DoS de login). Qualquer arquivo colocado em /www/ é servido publicamente. Uma vulnerabilidade em daemon/CGI dá root dentro do container."
proposed_action: "1) Rate limit no /tcpprobe (Cloudflare WAF rule ou contador no DO) + token de auth. 2) Whitelist de paths no Worker: só /health, /cgi-bin/health.sh, /tcpprobe. 3) USER builder no Dockerfile com chown de /opt,/www (portas >1024 dispensam root)."
validation_notes: "ab -n 200 -c 50 no /tcpprobe deve ser throttled; path fora da whitelist deve dar 404; docker exec id → uid != 0."
