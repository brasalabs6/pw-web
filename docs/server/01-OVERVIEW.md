# 01 — Visão geral do servidor

**Data:** 2026-10-28 UTC · **Fonte primária:** `src/server153/PROVENANCE.md`, `docs/SERVER153-REPORT.md`

## O que é

O `src/server153` é a nossa cópia de trabalho versionada do **servidor
comunitário de Perfect World 1.5.3** — um stack completo de backend de MMORPG
em C++ **32-bit**, composto por 8 daemons de rede + 1 game server:

```
glinkd · gdeliveryd · gauthd · gamedbd · uniquenamed · gfaction · gacd ·
logservice · gs (game server)
```

Ele conversa o protocolo **GNET** com o cliente Perfect World, persiste estado
em um storage embutido estilo Berkeley DB (WDB) e orquestra entrada no mundo,
personagens, facções, nomes únicos, anti-cheat e logs.

## Proveniência e autorização

- **Origem:** `github.com/hrace009/PW-Source-1.5.3` — fork de
  `WolfsCore/PW-Source-1.5.3`, raiz da cadeia `wladsoncedraz/Source-Server`.
- **Classificação:** derivada comunitária, **não oficial**. O primeiro commit
  disponível (2024-10-25) já descreve um servidor adaptado; correspondência
  com a versão oficial 1.5.3 não é demonstrável (análise do operador,
  2026-10-25).
- **Revisão pinada:** `7dde52fa03d3` (2024-11-05) — pós "Upgrade para
  1.5.3v145" (`8aa72d3`) e **anterior** à "modernização" de jan/2025
  (`d35f7de`, `cc74d74`, `1b7c9fd`), que substituiu Makefiles funcionais por
  stubs não-construíveis. O HEAD upstream (`c283446`) não é usado.
- **Referência congelada:** `references/server-153-source` (gitlink na mesma
  revisão) serve de base de diff:
  `git diff --no-index references/server-153-source src/server153`.
- **Autorização:** `docs/LEGAL.md` (decisão 2026-10-25) — port contratado pela
  companhia detentora dos direitos; compilar, usar, modificar e integrar são
  permitidos no escopo. Headers de copyright Archosaur/Perfect World são
  preservados em toda modificação. Distribuição segue contrato do operador.

## Capacidades provadas (com o cliente real)

Estado em 2026-10-28, tudo com evidência em logs/capturas
(`docs/SERVER153-REPORT.md` §0 e scratch do goal):

| Capacidade | Status | Evidência |
| --- | --- | --- |
| Compilação integral (libs + gs + 8 daemons) | **PROVADO** | `scripts/build-server153.sh` verde; CI `server153-build.yml` 5/5; BUILD-REPORT com todos os estágios PASS |
| Handshake GNET com cliente real | **PROVADO** | traces `[glinkd]`: challenge → Response → MatrixPasswd2 retcode=0 → KeyExchange → userlogin2 retcode=0 → OnlineAnnounce → GRoleList |
| Criação de personagem (cliente real) | **PROVADO** | wire 1704 bytes decodifica limpo após remoção do campo `rank` (commit `0c445e35`); teste de conformança `tests/server153_createrole_wire.cpp` |
| Entrada no mundo (cliente real) | **PROVADO** | `[enterworld] roleid=1024 … dispatching to gameid=101`; personagem no is61; `world2.log` do logservice |
| Movimento persistido | **PROVADO** | posição inicial (-712.87, 35.02, -364.39) → (172.0, 31.8, -165.2) gravada no storage do gamedbd |
| Re-login após logout rápido | **PROVADO** | re-login contra gdeliveryd fixado (commit `f6e0abb7`) sem segfault; dmesg sem crash novo |
| Estabilidade assistida | **PROVADO** | watchdog derrubado/renascido: glinkd morto → renasce ~10 s → probe GNET OK |
| Deploy Cloudflare Containers | **PROVADO** | `/health` 200, `/cgi-bin/health.sh` 200 com `daemons_alive:8`, `/tcpprobe` 200 |

## O que a source contém

- `share/` — framework GNET: I/O, protocolo, RPC, threads, storage, perf.
- `cnet/` — os 8 daemons + ferramentas (`gclient` cliente virtual de teste,
  `dbtool`, `gdbclient`, `gamed`, `logclient`).
- `cgame/` — o game server `gs` + libs de gameplay (`libtask`, `libcommon`,
  colisão).
- `cskill/` — sistema de skills (`libskill.a/.so`, ~5.758 headers de skill
  gerados por um skill editor Java).
- `PWSL/` — **Perfect World Serverside Library**: bibliotecas serverside
  (io/log/rpc + `rpcgen`), não uma linguagem de script.
- `cnet/rpcdata/` — 668 estruturas de RPC geradas por `rpcgen` (Perl) a partir
  de `rpcalls.xml`; validação de consistência no CI. Os 724 arquivos `.hrp`
  (implementações de RPC) ficam distribuídos em `cnet/`.

## O que NÃO está na source (e é milestone separado)

1. **Data files do jogo**: `elements.data`, `tasks.data`, `npcgen.data`,
   `*.sev`, `movemap/`, `config/` — usamos os do pacote PWBR v145 e do cliente
   real instalado (ver [`07-DATA-AND-STORAGE.md`](07-DATA-AND-STORAGE.md)).
2. **Ferramentas administrativas** (iweb/java/php) e seus schemas — o MySQL
   citado no readme upstream é para essas ferramentas; **nenhum daemon usa
   MySQL/MariaDB**.
3. `sysauctionlist.txt` — leitura opcional do gdeliveryd (loga erro e segue).

## Limitações conhecidas atuais

- **Spawn abaixo do terreno**: personagens novos podem nascer com Y abaixo
  do chão no is61 (contornado com teleport GM; issue cosmética em aberto).
- **Crash cosmético do pai do gs**: o processo pai do `gs` morre com SIGFPE
  no exit (destrutores de `ClassInfo` do `libskill.so`); os processos das
  instâncias sobrevivem (ver [`08-WORLDS-AND-INSTANCES.md`](08-WORLDS-AND-INSTANCES.md)).
- **Bugs latentes de concorrência**: a classe de bug do rwlock foi fechada
  estruturalmente (`share/common/mutex.h`), mas builds TSan/ASan para caçar
  races restantes ainda não foram feitos; o watchdog cobre o intervalo.
- **TCP inbound direto na Cloudflare**: a plataforma só expõe o container
  via mediação Worker/DO (`getTcpPort`) — clientes de jogo não conectam
  direto; o gateway do ADR-0006 é o caminho para o browser.

## Onde tudo roda

- **Local:** container Docker `pw153-runtime` (Debian 12, bind mount da
  worktree em `/work`), 8 daemons + gs + watchdog; glinkd publicado em
  `127.0.0.1:29000`.
- **CI:** GitHub Actions `server153-build.yml` (debian:12 + multilib), gera
  pacote `pw-server153-*.tar.gz` (~120 MiB) como artifact.
- **Cloudflare:** container app `pw-server153` (política `durable_object`,
  instância lite), Worker publicando `/health`, `/probe`, `/tcpprobe`.
