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

- Container app `pw-server153`, política `durable_object`, imagem com os 8
  daemons **+ o gs (world server) com o mapa is61** — o pacote de dados do
  jogo (`gamed/`: config compartilhado + `maps/a61`) e nossos binários
  stripped vão na imagem; o `gs.conf`/`gsalias.conf` têm o `Root`
  normalizado para `./config/` (o pacote original aponta para
  `/home/pwserver/gamed`, que só existe no runtime local).
- **Instância custom** via `start({instance: {vcpu, memoryMib, diskMb}})`:
  2 vCPU / 2048 MiB / 8192 MB — o is61 mede ~650 MB de RSS e a instância
  `lite` (256 MiB) não o comportaria. `GS_INSTANCES` (env do container,
  default `is61`) escolhe os mundos; `gs01 is61` precisa de ~3072 MiB e do
  mapa `world/` no stage (`PW153_CF_MAPS="world a61"`).
- Worker `pw-server153` (URL gerenciada) com endpoints (whitelist; resto 404):
  - `GET /health` — **dinâmico**: proxy ao CGI que conta os 8 daemons + o gs
    (`{"ok":true,"daemons_alive":8,"gs_alive":1,…}`); daemon ou gs morto
    vira `ok:false`.
  - `GET /worldcheck` — prova direta do mundo: `exec()` no container lê o
    announce do gdeliveryd
    (`"gameserver 101 … worldtag is 161"`) e a contagem de gs.
  - `GET /tcpprobe` — TCP de mediação ao glinkd:
    `getTcpPort(29000).connect()` do próprio DO.
- `INACTIVITY_TIMEOUT_MS = 30 min`; `enableInternet: false` no start.

**Evidência pública (2026-10-28, imagem digest `d00dda0f…`, worker
`bc194134`):** `/health` → `{"ok":true,"daemons_alive":8,"gs_alive":1}`;
`/worldcheck` → `world_up:true` com o announce do worldtag 161 lido do log
do gdeliveryd dentro do container em produção; `/tcpprobe` → 200. A imagem
idêntica foi validada localmente com o announce `gameserver 101 … worldtag
is 161` e `world2.log` ativo (`gamed … world:161`).

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

- **Instância custom** 2 vCPU / 2048 MiB (o is61 mede ~650 MB de RSS; a
  `lite` de 256 MiB não o comportaria — por isso o deploy usa
  `start({instance: {vcpu, memoryMib, diskMb}})`).

## Staging da imagem (`scripts/cloudflare-server153.sh`)

```
stage   — monta o staging DENTRO de cloudflare/ (server153/ + gamed/):
          o wrangler containers build usa o diretório do wrangler.jsonc
          como contexto (o PATH positional não é honrado), então o staging
          vai para cloudflare/server153 (daemons) e cloudflare/gamed
          (dados do jogo + binários stripped) — ambos gitignored
local   — docker build + teste local (health + announce do gdeliveryd + gs)
deploy  — wrangler containers build + wrangler deploy
```

Regras críticas do staging:

1. **`Root` do gs normalizado**: `gs.conf` E `gsalias.conf` do pacote vêm com
   `[Template] Root = /home/pwserver/gamed/config/` (o alias é *appendado*
   ao mesmo Conf no boot e vence) — o stage reescreve ambos para `./config/`.
2. **Mapas em camadas separadas** (`gamed/maps/`, linkados em `config/` pelo
   entrypoint): uma camada única de dados grande (~490 MB) falhou o push do
   registry com 401 durante o upload longo; camadas menores passam.
   Default `PW153_CF_MAPS=a61` (is61); `world` adiciona o mapa principal.
3. **Remove TODO o storage de runtime** — cópia parcial (cp falha em
   arquivos root-owned) quebra a inicialização ("Initialize storage
   environment failed").
4. **gshopsev convertido no host** (196→220 bytes) — o container não precisa
   de python3; os `.old` não entram na imagem.
5. No host, antes do stage: `sudo chown -R nonexphere:nonexphere src/server153`
   (o stage tenta best-effort; logs root-owned do container local abortam o
   cp no meio da árvore).

## Caminho para clientes de jogo (estado e plano)

- **Hoje:** clientes TCP externos não conectam direto ao container — a
  plataforma não expõe TCP inbound público. O caminho provado é a mediação
  (`/tcpprobe`).
- **Plano (ADR-0006):** gateway no Worker — cliente browser (WebSocket hoje,
  WebTransport quando o runtime suportar) ↔ Worker ↔ `getTcpPort(29000)` ↔
  glinkd, preservando o framing GNET byte a byte. O `/tcpprobe` já valida a
  perna TCP dessa arquitetura.
