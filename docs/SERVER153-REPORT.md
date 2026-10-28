# Relatório do servidor 1.5.3 (`src/server153`) — completude, tamanhos e descobertas de runtime

**Data:** 2026-10-26. **Build revision:** `7dde52fa03d3` (ver `src/server153/PROVENANCE.md`).
**Autorização:** `docs/LEGAL.md` (decisão 2026-10-25, escopo do port contratado).

## 0. RESULTADO PRINCIPAL (atualizado 2026-10-27)

**Dois resultados distintos, com autoria precisa:**

1. **Cadeia completa do servidor provada com o cliente virtual in-tree (`gclient`),
   2026-10-26 22:22–22:30** (`src/server153/cnet/logservice/logs/world2.log`):
   login → criação de role (roleid=1040, via `gclient/makechoice.h`, prompt
   interativo `c` + `0 3 100000`) → enterworld →
   `gameserver 101, 用户数目:1` (1 jogador no mundo por 4+ min). Isso prova
   glinkd → gdeliveryd → gamedbd → gs com os data files v145.
2. **Cliente real PWBR 1.5.3 v145 (Build 2555, via Wine): login comprovado;
   criação de personagem estava bloqueada** por divergência de wire format
   (§4.2 item 9), **corrigida em 2026-10-27** (commit 0c445e35) e comprovada por
   teste de conformância byte a byte (`tests/server153_createrole_wire.cpp`).
   A entrada no mundo com o cliente real **pende do teste do operador**.

Login do cliente real com evidência (traces glinkd, 2026-10-27 09:23):
challenge → Response → MatrixPasswd2 retcode=0 → KeyExchange → userlogin2
retcode=0 → OnlineAnnounce → ChangeState(GRoleList) → RoleList_Re (lista
vazia decodifica) → cliente abre a tela de criação.

Conta de teste: `1000000` / `1000000` (numérica, múltiplo de 16 — exigência do
dev-auth desta árvore; ver §4).

## 1. Classificação da fonte

- **Derivada comunitária, não oficial.** Origem: `hrace009/PW-Source-1.5.3`
  (fork de `WolfsCore/PW-Source-1.5.3`, raiz `wladsoncedraz/Source-Server`).
  O primeiro commit disponível (25/10/2024) já descreve um servidor adaptado;
  correspondência com a versão oficial 1.5.3 **não é demonstrável** (análise do
  operador). HEAD `c283446` (2025-01-18) contém Makefiles stub não-construíveis
  e **não** é usado; a revisão de build é a `7dde52f` (pós-v145, Makefiles reais).
- **Conteúdo:** apenas o **servidor** em C++ (32-bit). Nada de cliente, nada de
  assets do jogo.

## 2. Completude — o que existe

| Componente | Estado | Observações |
| --- | --- | --- |
| Framework `share/` (io, rpc, storage, threads, perf) | completo | storage embutido (estilo Berkeley DB); **nenhum daemon usa MySQL** |
| `cnet/` — 8 daemons + ferramentas | completo | glinkd, gdeliveryd, gamedbd, gauthd, uniquenamed, gfaction, gacd, logservice; gclient (cliente virtual de teste) e dbtool |
| `cgame/` — servidor de jogo (`gs`) | código completo | binário constrói e inicia; **falha sem os data files** |
| `cskill/` — sistema de skills | completo | libskill.a/.so com ~5.758 headers de skill gerados (`skills/skillXXXX.h`, skill editor Java em `cskill/gen/`) |
| `PWSL/` — bibliotecas serverside | presente | "Perfect World Serverside Library" (io/log/rpc + rpcgen), não linguagem de script; não integrado ao build dos daemons |
| RPC gerado (`rpcdata/`, 668 arquivos) | commitado e consistente | `rpcgen` (Perl) valida igual em CI |
| Configs de runtime (`gamesys.conf`, `*.conf`) | presentes | com mistmatches entre vintages (ver §4) |
| Ferramentas admin (java/php/iweb) | ausentes | o MySQL do readme é para essas ferramentas, não para os daemons |

## 3. Completude — o que falta (para um realm jogável)

1. **Data files do jogo** (não estão nesta fonte): `elements.data`, `tasks.data`,
   `dyn_tasks.data`, `aipolicy.data`, `npcgen.data`, `world_targets.sev`,
   `extra_drops.sev`, `precinct.sev`, `region.sev`, `path.sev`, `gshopsev.data`,
   `gshopsev1.data`, `global_api.lua`, `map.bht`, `wmgj.nmd`, além de
   `movemap/`, `watermap/`, `airmap/` e o diretório `config/` — referenciados por
   `cgame/gs/gs.conf` `[Template]`. O `gs` inicia, carrega substâncias e sai com
   código -300 ao não encontrar `./config/aipolicy.data`. **Realm jogável é
   milestone separado** (assets autorizados pelo gate jurídico, mas não presentes
   nesta fonte).
2. **Ferramentas administrativas** (iweb/java/php) e o schema delas.
3. `sysauctionlist.txt` (leitura opcional do gdeliveryd; loga erro e continua).

## 4. Descobertas de runtime (bugs reais corrigidos por nós)

### 4.1 Violações de ODR (crashes na inicialização)

O upstream nunca compilou a árvore inteira com flags consistentes entre unidades
de tradução. Cada uma causava segfault na inicialização:

1. **Locks stub vs. reais** (`share/common/mutex.h`): sem `_REENTRANT_`, Mutex/RWLock
   viram stubs no-op de 1 byte. `matcher.o` (via `gamed`, multi-thread) construía
   `Matcher::instance` com `cinput` em +32; `gdeliveryd.cpp` (SINGLE_THREAD=true)
   lia `cinput` em +4 — dentro do estado do rwlock — e `iconv_close(1)` segfaultava.
   Fix: `-D_REENTRANT_` nos dois branches de `share/mk/gcc.defs.mk`.
2. **`gnet_map` = hash_map vs. std::map** (`share/common/map.h`): daemons definem
   `USE_HASH_MAP`, objetos share (compilados primeiro via logclient) não;
   `PollIO::ionew` era construído como `std::map` e iterado como `hash_map` pela
   thread de poll (buckets NULL). Fix: `-DUSE_HASH_MAP` global.
3. **Wrappers ausentes no gauthd**: `certrequest.hpp`/`certkey.hpp` nunca foram
   commitados (criados seguindo o padrão); 6 esqueletos `.hrp` ProxyRpc
   referenciavam `GAuthClient`/`GameDBClient` inexistentes no gauthd.

### 4.2 Compatibilidade com o cliente real v145 (handshake e login)

4. **Version/edition do challenge**: o cliente compara `challenge.version` com
   `GAME_VERSION` (0x010503 no binário PWBR 2555) e `challenge.edition` com a
   string que ele computa dos próprios data files (`%x%x%x%x%x` de
   elements/tasks/gshop). O `gamesys.conf` commitado não tem a chave `version`
   e o `edition` só era setado via announce do gs. Fix: `version=10503` +
   `edition` pinado no conf (patch no glinkd.cpp) + o announce respeita o pin.
5. **Cadeia MatrixPasswd incompleta** (3 lacunas): os estados de sessão não
   listavam `RPC_MATRIXPASSWD/2/TOKEN` nem no glinkd (`_state_GDeliverClient`)
   nem no gdeliveryd (`_state_GAuthClient`) — o RPC não saía do sponsor (21) e
   não chegava ao gauthd (8). E o gauthd, nó terminal, nunca teve handler: a
   base `ProxyRpc::Process()` só encaminha via `Delivery()`. Fix: tipos
   adicionados aos estados + `Process()` terminal no gauthd com dev-auth
   (contas numéricas múltiplas de 16, hash MD5(conta‖conta), resposta
   HMAC_MD5(desafio, hash)). Evidência: `retcode=0,userid=1000000`.
6. **Loops de re-anúncio de provider**: o gs abre múltiplas conexões provider
   e re-anuncia após reconexões; o check "Identical game server id" do upstream
   fechava as sessões → registro oscilava → clientes viam "servidor suspenso".
   Fix no glinkd E no gdeliveryd: substituir o mapeamento stale em vez de fechar.
7. **gshopsev.data em formato antigo**: o pacote de dados v145 usa `GSHOP_ITEM`
   de 196 bytes (sem `min_vip_level` no `buy[]` e sem `buy_times_limit/mode`);
   esta fonte espera 220. Fix: conversor em `scripts/gs-server153.sh`
   (originals preservados como `.old`).
8. **serverlist da patcher**: o cliente com `startbypatcher` lê
   `patcher/server/serverlist.txt` (não o do element) e exige cabeçalho de
   grupo: `grupo\r\n\r\nnome\tporta:host\tid\r\n` em UTF-16LE com BOM.
9. **Divergência de wire format em `RoleInfo` (bloqueava a criação de
   personagem no cliente real)**: o commit upstream 8aa72d3 adicionou o 24º
   campo `Octets rank` a `RoleInfo` para um cliente chinês mais novo; o binário
   PWBR 2018 envia o formato de 23 campos. Evidência: pacote real capturado
   `[decode] type=84 size=1704` (o layout de 24 campos prevê 1705 — exatamente
   um Octets vazio a mais); o servidor lia `rank` na posição do byte de
   comprimento de `referid` e lançava exceção ao ler `referid` além do fim →
   `Decode Error`. Confirmado pela fonte de era correspondente (árvore
   `20141231`, `rpcalls.xml`: 23 campos) e pela source completa do cliente
   (`Source Cliente.rar`, `EC_LoginUIMan.cpp`: 39 slots de equipment, nome
   UTF-16, custom_data de 176 bytes). Fix: remoção de `rank` (commit 0c445e35)
   + teste de conformância de wire byte a byte
   (`tests/server153_createrole_wire.cpp`, integrado ao `verify.sh`).
   Nota: `GRoleStatus.rank` (renomeado de `reserved2` pelo mesmo commit
   upstream) é compatível no wire enquanto vazio e permanece.
10. **Spawn de personagem novo no mundo paralelo (design original, não
    bug)**: a criação carrega o template da classe via
    `GameDBManager::GetClsDetail` do storage `clsconfig`, importado a cada
    boot do arquivo `cnet/gamedbd/clsconfig`. Tanto o arquivo vendored
    (hrace009, 90KB) quanto o original do pacote PWBR (61KB, md5
    dd08cebb…) fazem o spawn no **is61** (worldtag 161, `parallelworld_server
    = 1` no gs.conf) — o novo personagem nasce no mundo paralelo por design
    do reino original. Substituímos pelo arquivo original do pacote.
11. **Arquitetura de fork do gs**: o binário `gs` faz fork de um processo
    por instância (cmdline `gs01 … is61`) e o processo pai sai. A saída do
    pai crasha com SIGFPE (backtrace gdb: `~ClassInfo` do `libskill.so`
    apaga do registro de classes já destruído — ordem de destruição entre
    DSOs; divisão por zero em `hashtab::get_hash`). Guard adicionado ao
    `find` do hashtab (tabela vazia → end()); o caminho do `erase` na
    saída permanece crashando de forma cosmética — os processos das
    instâncias (netos) sobrevivem.
12. **Corrupção do rwlock do UserContainer (bloqueava a entrada no mundo
    — corrigido em DUAS frentes)**: (a) `EnterWorld::Process` segurava o
    write lock e chamava `AnnouncePlayerOnline → WalkUser`, que pega
    `RDScoped` aninhado no MESMO rwlock → glibc EDEADLK → o destrutor
    aninhado destrava → o destrutor externo destrava DE NOVO → contador
    corrompido (dump gdb ao vivo: `__readers = -6`; todos os writers —
    EnterWorld, GetRoleBase::Client, SynMutaData, StatusAnnounce —
    bloqueados em `pthread_rwlock_wrlock` para sempre). Fix 1: side-effects
    fora da região travada (74df6d67). Fix 2 (estrutural): os wrappers
    `WRScoped`/`RDScoped` do `share/common/mutex.h` agora só destravam o
    que conseguiram travar — uma aquisição aninhada (EDEADLK) vira no-op
    em vez de envenenar o contador. A classe inteira de bug está morta.
13. **Fila de prioridade 101 do gdeliveryd (auth) + EnterWorld inline**:
    os RPCs de autenticação (`userlogin2.hrp:30`, `matrixpasswd2.hrp:29`,
    `gquerypasswd.hrp:27`, `matrixtoken.hrp`) despacham com `PriorPolicy=101`;
    o conf commitado tem UM worker nessa prioridade — travado no rwlock
    corrompido (item 12), acumulava a fila de auth (TaskQueueSize=7 ao
    vivo). `EnterWorld`/`CreateRole`/`RoleList` herdam prioridade 0 e rodam
    inline na thread de poll (`protocol.h:455-468`), que travava no mesmo
    lock. Runtime conf agora usa `(101,4)`. (Correção factual 2026-10-28:
    a redação original atribuía 101 ao EnterWorld.)
14. **Use-after-free logout↔re-login (derrubava o gdeliveryd — corrigido)**:
    o usuário fecha o cliente e re-loga rápido; o logout antigo
    (`RoleLogout`) ainda roda quando o re-login (`RoleLogin`) faz
    `delete user->role` — o `role->name` vira memória liberada e o
    `EraseName` hasheia o ponteiro morto → SIGSEGV no gdeliveryd (coredump
    com backtrace resolvido: `hash<Octets>` ← `hash_map::find` ←
    `EraseName` ← `RoleLogout`) → todos os links entre servidores caem
    junto. Fix em duas camadas: `UserLogout` segura o write lock do
    UserContainer durante todo o logout (serializa contra o re-login; o
    `EraseUser` interno vira `EraseUserNoLock`) + `RoleLogout` copia o nome
    da role antes do caminho lento. Commit f6e0abb7.
15. **Crash do gs no caminho `erase` do hashtab (matava a instância is61
    em runtime — corrigido)**: o guard anterior cobria só o `find`; o
    `erase` (destrutores de `ClassInfo` do `libskill.so` apagando do
    registro de classes destruído) ainda dividia por zero. A instância
    is61 morria em runtime, o provider era apagado do gameservermap, e o
    login da role (spawn no mundo 161) falhava com `result=31,
    gameid=-1`. Guard espelhado no `erase`. Commit a4fc35aa.
16. **Watchdog**: `scripts/watchdog-server153.sh` mantém os 8 daemons + gs
    vivos (reinicia na ordem documentada; glinkd com o argumento de
    instância; zombies não contam como vivos). Validado ao vivo: glinkd
    morto → renasce em ~10s → probe GNET OK. Um crash eventual é um blip
    de segundos, não um outage.

### 4.3 Config/ambiente (documentados no `scripts/boot-server153.sh`)

- confs `gamesys.conf` de vintages diferentes discordam em portas (gauthd 9200
  vs. 29200; gacd 29702 vs. 29701/29709) — o boot script gera confs de runtime
  com os fixes documentados; os confs commitados não são alterados.
- `filters` (tabela de palavras sensíveis) é GBK, mas os confs declaram
  `table_charset=UCS2` — fix no conf de runtime.
- gamedbd: `accepted_zone_list`, `cross_zone_pos` (4 entradas), `[CASHVIP]` e a
  lista `tables` completa (48 storages referenciados pelo código; o conf lista
  26 — `GetStorage("syslog")` retornava NULL e o InitGUID segfaultava).
- chaves de criptografia gdeliveryd↔gauthd cruzadas (o gauthd.conf commitado
  tem placeholders 456/123).
- `gacd` carrega `libxml2.so.2` do próprio diretório se `LD_LIBRARY_PATH`
  apontar para ele (binário antigo commitado) — o boot só define
  `LD_LIBRARY_PATH` para o gs.
- `gs` precisa de `LD_LIBRARY_PATH` no próprio diretório (`libtask.so`,
  `libskill.so`) e roda do `gamed` do pacote de dados
  (`scripts/gs-server153.sh`).
- WDB (gamedbd) não tem recuperação de estado CORRUPT (`truncate()` retorna
  CORRUPT): estado de shutdown sujo trava o boot; `--fresh` limpa o storage
  vazio para validação.
- **Makefiles do upstream não rastreiam dependências de headers**: editar
  `cnet/rpcdata/*` NÃO suja os `.o` dos daemons — `make` não reconstrói nada
  (o estágio glinkd passou em 0s com binário antigo; o cliente real continuou
  recebendo Decode Error após a correção de `RoleInfo`). Após editar
  `rpcdata/` ou qualquer header compartilhado, reconstrua com `make -B` os
  daemons afetados (glinkd, gdeliveryd, gamedbd, gclient, gdbclient para
  `roleinfo`) e confira o mtime do binário.

## 5. Tamanhos medidos (re-medidos 2026-10-27 20:32 UTC)

| Item | Tamanho |
| --- | --- |
| Árvore de trabalho `src/server153` (COM artefatos de build: .o, binários) | **520 MB** |
| Por diretório (com build): `cnet` 279 MB, `cgame` 119 MB, `cskill` 111 MB, `share` 12 MB | — |
| Upstream no GitHub (repo completo, API) | ~37 MB |
| Pacote de build `pw-server153-*.tar.gz` (binários+libs+configs+relatório) — gerado por `scripts/build-server153.sh` (estágio package) e pelo CI (artifact do workflow) | ~122–128 MB |
| Binários atuais (relink 2026-10-27, com `-g`): `gs` 33 MB, `gdeliveryd` 37 MB, `libskill.so` 34 MB | — |
| Imagem Docker de runtime (debian:12 + libs i386 + árvore de daemons) | **480 MB** |
| Monorepo: pack do git (`git count-objects -vH`) | 114,95 MiB |

Nota: binários mantêm debug info (`-g -O0`); `strip` reduzirá
drasticamente para a imagem de produção. Medição anterior (2026-10-26,
141 MB de fonte sem build) permanece como referência do tamanho de
source puro.

## 6. Cloudflare Containers — VALIDADO (health público + probe TCP de mediação)

**Deploy funcional** (2026-10-27 22:53 UTC): Worker em
`https://pw-server153.guibelongtovi-a22.workers.dev`, imagem de 480MB no
registry da conta, container app `pw-server153` (política
`durable_object`), instância **lite** (1/16 vCPU, 256 MiB — uso medido: 26
MiB para os 8 daemons + httpd).

**Evidência pública** (capturada via curl, ver
`cf-health-evidence.txt` no scratch do goal):
- `GET /health` → 200 `{"ok":true,"app":"pw-server153",...}`
- `GET /cgi-bin/health.sh` → 200 `{"ok":true,"daemons_alive":8,"expected":8}`
  — **os 8 daemons rodando DENTRO do container Cloudflare**
- `GET /tcpprobe` → 200 `{"glinkd_29000_mediation_tcp":true}` — o glinkd
  aceita TCP pelo caminho de mediação do Worker/DO
  (`getTcpPort(29000).connect()`), exatamente o caminho que o gateway do
  ADR-0006 usará para clientes.

**Causa raiz do bloqueio anterior (resolvida)**: o entrypoint python3
(http.server). Bissecção com 8 apps-probe na mesma conta: entrypoints
busybox-httpd provisionam instantaneamente em qualquer tamanho de imagem
(testado até 202MB); entrypoints python3 NUNCA provisionam (lite e
standard-1, caminho direto do binário, via script) — a plataforma recusa
com "There is no container instance that can be provided to this
Durable Object". O health server agora é busybox httpd: `/health`
estático + CGI shell contando os daemons; o probe TCP do glinkd é feito
pelo Worker pela camada de mediação (mais forte que um connect local).
Nota: a conta está no Workers Paid (o Worker usa Durable Object com
storage SQLite, que a doc exige no plano Paid; o deploy passa).

**Fontes oficiais (modelo de tráfego e APIs)**:
- Containers — visão geral e modelo de acesso:
  https://developers.cloudflare.com/containers/ ("Container instances are
  spun up on-demand and controlled by code you write in your Worker") —
  clientes não conectam direto ao container; o tráfego passa pelo Worker/DO.
- Durable Object Container API (`getTcpPort` — o caminho de mediação usado
  pelo nosso Worker):
  https://developers.cloudflare.com/containers/api/durable-object-container/
  ("getTcpPort() returns a TCP port from the container. Use it to
  communicate with the container over TCP or HTTP.")
- Workers Runtime APIs (o runtime expõe Fetch, TCP sockets e WebSockets;
  sem API WebTransport server-side — base do ADR-0006):
  https://developers.cloudflare.com/workers/runtime-apis/
- Limits and Instance Types (a instância lite: 1/16 vCPU, 256 MiB, 2 GB):
  https://developers.cloudflare.com/containers/platform/limits/

## 7. Reprodução

Ver `scripts/build-server153.sh` (build + pacote) e `scripts/boot-server153.sh`
(boot + verificação de portas). Ambos rodam em container Debian 12 com toolchain
i386 (ver workflow `.github/workflows/server153-build.yml` para a lista exata de
pacotes). Evidências de execução: PR do branch `feat/pw-source-1.5.3-ci-build`.
