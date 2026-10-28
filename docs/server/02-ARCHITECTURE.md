# 02 — Arquitetura

**Data:** 2026-10-28 UTC · **Fontes:** código (`cnet/`, `cgame/`, `share/`), medição ao vivo de portas, relatórios de exploração da sessão

## Topologia

```
                          ┌────────────┐
        Cliente PW ◀─TCP──▶  glinkd    │ (29000 — única porta de clientes)
                          │            │──▶ gdeliveryd (29100)
                          │            │──▶ gfactiond  (29500)
                          │  GProvider │◀── gs (anuncia providers; 29301)
                          └────────────┘
                          ┌────────────┐
                          │ gdeliveryd │◀── gs (GProviderServer 29300)
                          │ (orquestra-│──▶ gauthd      (9200)
                          │  dor)      │──▶ gamedbd     (29400)
                          │            │──▶ uniquenamed (29401)
                          │            │──▶ gfactiond   (29500)
                          │            │──▶ gacd        (29702)
                          └────────────┘
   Persistência: gamedbd (storage WDB, 48 storages), uniquenamed (unamewdb)
   Logs: todos ──▶ logservice (11100 UDP / 11101 TCP → world2.*)
```

O `gs` (cgame) conecta-se como **provider** ao gdeliveryd (29300), ao glinkd
(29301) e ao gfactiond (29600), e fala com o gamedbd (29400) e o logservice
pelas libs `gdbclient`/`logclient`.

## Os 8 daemons (papel, portas, estado)

| Daemon | Papel | Escuta | Storage |
| --- | --- | --- | --- |
| **glinkd** | ponto de entrada do cliente: handshake, login, roles, roteamento de pacotes | 29000 (clientes), 29301 (gs) | nenhum (tudo em memória) |
| **gdeliveryd** | orquestrador central: UserContainer (players online), roteia glinkd↔gs↔gamedbd, ~20 subsistemas (battle, auction, stock, pshop, kingelection…) | 29100 (glinkd), 29300 (gs) | nenhum próprio (persistência é no gamedbd) |
| **gauthd** | autenticação terminal (ProxyRpc): UserLogin2, MatrixPasswd2; kickout de sessão duplicada | 9200 | nenhum (stateless nesta árvore — não chama `StorageEnv::Open`) |
| **gamedbd** | banco do jogo: roles, inventário, mail, leilões, facções, transações | 29400 | **WDB** (48 storages) + BDB |
| **uniquenamed** | unicidade de nomes (roles/factions/families) + alocação de LogicUID | 29401 | **WDB** (config, uidrole, unamefaction, unamerole) |
| **gfactiond** | facções: criação, membros, alianças, fortalezas | 29500, 29600 (gs) | cache em memória; persiste via gamedbd |
| **gacd** | anti-cheat: punições, códigos de verificação, padrões de memória/processo | 29702 (gdeliveryd), 29712 (iweb) | nenhum (config em `gacd.xml`) |
| **logservice** | coletor central de logs (UDP+TCP) | 11100 UDP, 11101 TCP | arquivos `world2.*`, `statinfo*` |

Binário do gs: **`gs`** (cgame) — mundo/instâncias por processo (ver
[`08-WORLDS-AND-INSTANCES.md`](08-WORLDS-AND-INSTANCES.md)).

## Fluxo de uma mensagem (cliente → mundo)

1. **TCP** no glinkd → `Session` GNET decodifica o framing
   (`CompactUINT(type) + CompactUINT(size) + corpo` — ver
   [`04-PROTOCOL.md`](04-PROTOCOL.md)).
2. O stub do tipo (`Protocol::GetStub(type)`) clona o protocolo
   (`Protocol::Create`) e o entrega ao `Manager` da sessão.
3. O `Manager` valida o tipo contra o **state array** da sessão
   (`state.hxx` de cada daemon — `Client::SendProtocol` falha em silêncio se
   o tipo não está listado) e faz `Dispatch` para o **Thread::Pool** com a
   prioridade do protocolo (`PriorPolicy`).
4. Um worker do pool executa `Protocol::Task::Run` → `Process()` do protocolo
   (ex.: `EnterWorld::Process` no gdeliveryd).
5. RPCs atravessam daemons via `Rpc`/`ProxyRpc` (sponsor → proxy → terminal).
6. Persistência via RPCs `db*` ao gamedbd (storage WDB).

## Threading

- Cada daemon tem um `Thread::Pool` configurado em `[ThreadPool]`:
  `threads = (prioridade,quantidade)…` — prioridade **0** = thread de I/O
  (`PollIO::Task`), **1** = workers normais, **100** = housekeeping,
  **101** = alta (RPCs de login/entrada: `EnterWorld`, `CreateRole`,
  `RoleList`).
- `max_queuesize` (default 1048576) limita a fila; `prior_strict=1` = fila
  estrita por prioridade.
- Runtime do gdeliveryd usa `(1,3)(100,1)(101,4)(0,1)` — 4 workers na 101
  (o conf upstream tem 1 e um worker bloqueado matava o fluxo inteiro —
  bug B13).
- Locks: `Thread::Mutex`/`RWLock` (`share/common/mutex.h`) com wrappers
  `WRScoped`/`RDScoped` que só destravam o que travaram (fix estrutural do
  bug B12). O `cgame` tem seu próprio `rwlock.h`/`spinlock.h`
  (`cgame/libcommon`).

## Bibliotecas

| Lib | Origem | Papel |
| --- | --- | --- |
| `share/io` (libgsio.a) | share/ | GNET: `Protocol`, `Session`, `PollIO`, `Rpc`, `ProxyRpc`, `Security`, compressão MPPC |
| `share/common` | share/ | `Octets`/`OctetsStream` (marshal), `Conf` (parser .conf), `Thread::Pool`, `Mutex/RWLock`, `Timer`, `Matcher` (filtro de palavras), conversão GBK/UCS2 |
| `share/perf` (libperf.a) | share/ | primitivas em assembly: AES, MD5, SHA1, RC4, MPPC, CRC32, base64 |
| `cnet/storage` | cnet/ | `StorageEnv` BDB + **WDB** (`storagewdb.h`), backup/recovery |
| `cnet/logclient` (liblogCli.a) | cnet/ | cliente de log (UDP/TCP) linkado por todos os daemons |
| `cnet/gamed` (libgsPro2.a) | cnet/ | protocolo do lado do gs: `gsp_if.h`, `GProviderClient`, syslibs (auction, mail, pshop, stock, kingelection, webtrade) |
| `cnet/gdbclient` (libdbCli.a) | cnet/ | interface gs↔gamedbd (`db_if.h`, máscaras `DBMASK_PUT_*`) |
| `cgame/libcommon.a` | cgame/ | alocadores, RNG, CRC, base64, conf, locks, threadpool, timer |
| `cgame/collision` (libTrace.a) | cgame/ | colisão: brushes, NMD tree, `trace_manager2` (map.bht, wmgj.nmd) |
| `cgame/gs/task` (libtask.so) | cgame/ | sistema de quests (`tasks.data`/`dyn_tasks.data`) |
| `cskill/skill` (libskill.a/.so) | cskill/ | skills: `Skill`/`SkillStub` + ~5.758 headers gerados |
| PWSL | PWSL/ | **Perfect World Serverside Library** — headers io/log/rpc + `rpcgen`; NÃO é linguagem de script |

## Código gerado (rpcgen)

- `cnet/rpcalls.xml` (~6.094 linhas) define protocolos, managers, states e RPCs.
- `rpcgen` (Perl, em `cnet/rpc/` e `PWSL/newlib/`) gera:
  - `cnet/rpcdata/` — 668 arquivos de estruturas de dados (arg/res);
  - `cnet/inl/` — 1.010 definições inline de protocolo/RPC;
  - os `.hpp`/`.hrp` incluem esses arquivos (`#include "enterworld"`).
- O CI valida que o `rpcdata` commitado bate com a saída do rpcgen (estágio
  soft do build).
- Os **724 arquivos `.hrp`** são implementações de RPC distribuídas em
  `cnet/` (gdeliveryd 232, gamedbd 214, gdbclient 192, gfaction 34,
  uniquenamed 15, glinkd 13, gauthd 11, gamed 9, gclient 4) — cada `.hrp`
  é uma classe `Rpc` com `Server()`/`Client()`.

## Registro de providers (gs ↔ gdeliveryd/glinkd/gfaction)

- O gs conecta aos `GProviderServer`s (gdeliveryd 29300, glinkd 29301,
  gfaction 29600 — 10 providers no `gmserver.conf`) e envia
  `AnnounceProviderID(gid, left, right, top, bottom, worldtag, edition)`.
- O gdeliveryd registra em `gameservermap[id] = {sid, region, worldtag}`,
  guarda a `edition`, devolve o `AnnounceProviderID` de confirmação, envia
  `PlayerStatusAnnounce` e libera login global (`ForbidLogin::AllowLoginGlobal`).
- **Comportamento pw-web**: gdeliveryd e glinkd **substituem** entradas
  stale em vez de fechar a sessão (fix do bug B06). O gfaction mantém o
  comportamento upstream de rejeitar com
  `"identical gameserver id(%d) find. check .conf file."`
  (`gfaction/announceproviderid.hpp:39`) — risco latente de flapping se dois
  gs usarem o mesmo ID.
- `FindGameServer(point, worldtag)` no gdeliveryd resolve o gs por posição do
  player + worldtag (é assim que o EnterWorld escolhe gameid 1 vs 101).

## Máquinas de estado de sessão

Cada daemon define seus estados em `state.hxx` (ex.: glinkd tem 22:
`state_GLoginServer`, `state_GKeyExchgSend/Succ`, `state_GRoleList`,
`state_GSelectRoleServer`, `state_GReadyGame`, `state_GDeliverClient`,
`state_GProviderLinkServer`, `state_GFactionLinkClient`…). O state array
filtra quais tipos de protocolo cada sessão aceita — a ausência de um tipo
no array é o bug clássico B05 (RPC some em silêncio).
