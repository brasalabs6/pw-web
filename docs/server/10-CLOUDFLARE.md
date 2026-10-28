# 10 — Deploy em Cloudflare Containers

**Data:** 2026-10-28 UTC · **Fontes:** `cloudflare/worker.js`, `cloudflare/entrypoint.sh`, `cloudflare/Dockerfile`, `scripts/cloudflare-server153.sh`, ADR-0006, `docs/SERVER153-REPORT.md` §6

## Modelo da plataforma (com fontes oficiais)

- Containers Cloudflare recebem tráfego **somente via mediação** de um
  Worker/Durable Object — clientes externos não conectam TCP direto
  ([overview](https://developers.cloudflare.com/containers/): "Container
  instances are spun up on-demand and controlled by code you write in your
  Worker").
- A API de acesso é a **Durable Object Container API**
  ([doc](https://developers.cloudflare.com/containers/api/durable-object-container/)):
  `this.ctx.container.getTcpPort(port)` devolve um `RemoteSocket` para TCP ou
  `fetch()` para HTTP dentro do container.
- O runtime Workers expõe Fetch, TCP sockets e WebSockets — **sem API
  WebTransport server-side** (base do ADR-0006: o gateway do cliente browser
  usa WebSocket/WebTransport no Worker → TCP de mediação → glinkd).
- Instâncias: `lite` (1/16 vCPU, 256 MiB, disk 2 GB) até `standard-1..4`
  ([limits](https://developers.cloudflare.com/containers/platform/limits/)).
  Em `container.start()` o tamanho é omitido → default `lite` ("basic" é nome
  só de config do wrangler e faz `start()` lançar TypeError).
- Conta: **Workers Paid** (exigido pelo Durable Object com storage SQLite).

## O que está deployado

- Container app `pw-server153`, política `durable_object`, imagem ~480 MB
  (debian:12 + libs i386 + árvore de daemons com binários **stripped**).
- Worker `pw-server153` (URL gerenciada) com endpoints:
  - `GET /health` — JSON estático (via busybox httpd no container, porta 8080).
  - `GET /probe` e `GET /cgi-bin/health.sh` — CGI shell que conta os 8
    daemons em `/proc`: `{"ok":true,"daemons_alive":8,"expected":8}`.
  - `GET /tcpprobe` — **TCP de mediação** ao glinkd:
    `getTcpPort(29000).connect()` do próprio DO — o mesmo caminho que o
    gateway ADR-0006 usará para clientes.
- `INACTIVITY_TIMEOUT_MS = 30 min`; `enableInternet: false` no start
  (o container não precisa de internet).

**Evidência pública (2026-10-27 22:53 UTC):** os três endpoints responderam
200 na primeira tentativa após o cold start (`cf-health-evidence.txt` no
scratch do goal).

## A saga do provisionamento (lição registrada)

- **Bloqueio original:** "There is no container instance that can be
  provided to this Durable Object" — a plataforma recusava provisionar a
  instância, em qualquer tamanho (lite e standard-1).
- **Bissecção com 8 apps-probe** na mesma conta: entrypoints **busybox httpd
  provisionam instantaneamente em qualquer tamanho** (testado até 202 MB de
  imagem); entrypoints **python3 http.server NUNCA provisionam** (caminho
  direto do binário ou via script). Conclusão: o health server em python3
  era o bloqueio.
- **Fix:** health server = **busybox httpd** (estático `/www/health` + CGI
  `/www/cgi-bin/health.sh` contando daemons); `exec busybox httpd -f -p 8080
  -h /www` no final do entrypoint. Provisionou na primeira.
- **Edge corta conexões seguradas em ~181 s** → o Worker não espera o cold
  start (pull + boot + 8 daemons excede isso): faz poucos retries curtos e
  responde `503 + Retry-After` para o cliente pollar.

## Recursos medidos

- Instância **lite**: 1/16 vCPU, 256 MiB. Uso medido: **26 MiB** para os 8
  daemons + httpd — cabe folgado; o limite prático é o cold start, não RAM.

## Staging da imagem (`scripts/cloudflare-server153.sh`)

```
stage   — monta dist/server153/cf-context/: árvore de daemons (binários
          strips, .o/.a/.so-fonte/removidos) + Dockerfile/entrypoint
local   — docker build + teste local (health + probe + logs dos daemons)
deploy  — npx wrangler containers build + wrangler deploy
```

Regras críticas do staging:

1. **Remove TODO o storage de runtime** (`dbhome/dbdata/dblogs/backup/
   dbhomewdb/unamewdb/backupwdb`). Uma cópia parcial (o `cp`/`docker cp`
   falha em arquivos root-owned) quebra a inicialização do storage
   ("Initialize storage environment failed") — foi o que derrubou o
   uniquenamed na primeira imagem.
2. No host, antes de stage: `sudo chown -R nonexphere:nonexphere
   src/server153` (docker-created root-owned dirs abortam o `docker cp`).
3. O entrypoint gera os mesmos confs de runtime do boot local (mesmos fixes
   de portas/keys/pool 101) e sobe os 8 daemons + httpd.

## Caminho para clientes de jogo (estado e plano)

- **Hoje:** clientes TCP externos não conectam direto ao container — a
  plataforma não expõe TCP inbound público. O caminho provado é a mediação
  (`/tcpprobe`).
- **Plano (ADR-0006):** gateway no Worker — cliente browser (WebSocket hoje,
  WebTransport quando o runtime suportar) ↔ Worker ↔ `getTcpPort(29000)` ↔
  glinkd, preservando o framing GNET byte a byte. O `/tcpprobe` já valida a
  perna TCP dessa arquitetura.
