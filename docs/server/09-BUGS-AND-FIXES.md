# 09 — Catálogo de bugs e correções

**Data:** 2026-10-28 UTC · **Fonte:** `docs/SERVER153-REPORT.md` §4 (findings 1–16), commits do branch `feat/pw-source-1.5.3-ci-build`

Cada entrada: **sintoma → diagnóstico → causa raiz → fix (commit) → evidência**.
Estes bugs foram todos encontrados ao vivo, com o stack rodando e o cliente
real conectando. Eles são a razão de várias regras operacionais desta doc.

## B01 — Locks stub vs. reais (ODR, segfault na inicialização)

- **Sintoma:** segfault na inicialização de daemons (ex.: `iconv_close(1)` no gdeliveryd).
- **Causa raiz:** sem `_REENTRANT_`, `Mutex`/`RWLock` (`share/common/mutex.h`)
  viram stubs no-op de 1 byte; unidades de tradução com layouts divergentes
  (`gamed` multi-thread construía `Matcher::instance` com `cinput` em +32;
  `gdeliveryd.cpp` single-thread lia em +4 — dentro do estado do rwlock).
- **Fix:** `-D_REENTRANT_` nos dois branches de `share/mk/gcc.defs.mk`.

## B02 — `gnet_map`: hash_map vs. std::map (ODR, crash na thread de poll)

- **Sintoma:** crash ao iterar `PollIO::ionew` (buckets NULL).
- **Causa raiz:** daemons definem `USE_HASH_MAP`, objetos de `share/`
  (compilados via logclient) não; `gnet_map` (`share/common/map.h`) era
  `std::map` de um lado e `hash_map` do outro.
- **Fix:** `-DUSE_HASH_MAP` global no build.

## B03 — Wrappers ausentes no gauthd (não compilava)

- **Sintoma:** gauthd não compilava; 6 esqueletos `.hrp` ProxyRpc referenciavam
  `GAuthClient`/`GameDBClient` inexistentes.
- **Causa raiz:** `certrequest.hpp`/`certkey.hpp` nunca foram commitados upstream.
- **Fix:** wrappers criados seguindo o padrão dos vizinhos.

## B04 — Version/edition do challenge rejeitadas pelo cliente real

- **Sintoma:** cliente real mostra "servidor suspenso" na seleção de servidor.
- **Causa raiz:** o `gamesys.conf` commitado não tem a chave `version` e o
  `edition` só era setado via announce do gs; o cliente PWBR 2555 exige
  `version == 0x010503` e a edition exata que ele computa dos próprios data
  files. Além disso o conf upstream carrega `version = 10204` que o parser
  resolve DEPOIS da chave corrigida.
- **Fix:** `version=10503` substituída in-place + `edition` pinada no conf de
  runtime (patch no `glinkd.cpp`; o announce respeita o pin).

## B05 — Cadeia MatrixPasswd incompleta (login não saía do lugar)

- **Sintoma:** login falha com retcode 21 (sponsor) e 8 (hop ao gauthd).
- **Causa raiz (3 lacunas):** os state arrays de sessão não listavam
  `RPC_MATRIXPASSWD/2/TOKEN` (IDs 550/8066/8070) nem no glinkd
  (`_state_GDeliverClient`) nem no gdeliveryd (`_state_GAuthClient`) —
  `Client::SendProtocol` falha silenciosamente quando o tipo falta; e o
  gauthd, nó terminal, nunca teve handler (`ProxyRpc::Process()` base só
  encaminha via `Delivery()`).
- **Fix:** tipos adicionados aos state arrays (commit `db85fa5`) +
  `Process()` terminal no gauthd com dev-auth.
- **Evidência:** `[auth] receive matrixpasswd. retcode=0,userid=1000000`.

## B06 — Loops de re-anúncio de provider (registro oscilando)

- **Sintoma:** clientes viam "servidor suspenso" intermitente; registro do
  gameserver oscilava no glinkd/gdeliveryd.
- **Causa raiz:** o gs abre múltiplas conexões provider e re-anuncia após
  reconexões; o check upstream "Identical game server id" fechava as
  sessões.
- **Fix:** substituir o mapeamento stale em vez de fechar — no glinkd **E**
  no gdeliveryd (ao tocar nesses daemons, confira que a correção permanece).

## B07 — `gshopsev.data` em formato antigo (196 vs. 220 bytes)

- **Sintoma:** gs falha ao carregar os arquivos de mall do pacote v145.
- **Causa raiz:** o pacote v145 usa `GSHOP_ITEM` de 196 bytes (sem
  `min_vip_level` no `buy[]`, sem `buy_times_limit/mode`); a source espera 220.
- **Fix:** conversor em `scripts/gs-server153.sh` (originais preservados como
  `.old`; idempotente — reexecutar é seguro; guard para container sem python3).

## B08 — serverlist da patcher (formato específico)

- **Sintoma:** cliente com `startbypatcher` não lista o servidor.
- **Causa raiz:** ele lê `patcher/server/serverlist.txt` (não o do element) e
  exige cabeçalho de grupo `grupo\r\n\r\nnome\tporta:host\tid\r\n` em
  **UTF-16LE com BOM**. Há dois serverlists no cliente instalado (`patch/` e
  `element/userdata/server/`) — confira os dois ao mudar o realm.
- **Fix operacional:** manter os dois serverlists consistentes.

## B09 — Wire `RoleInfo` divergente (bloqueava criação no cliente real)

- **Sintoma:** cliente real: Decode Error ao criar personagem
  (`[decode] type=84 size=1704`); gclient passava.
- **Causa raiz:** o commit upstream `8aa72d3` adicionou o 24º campo
  `Octets rank` a `RoleInfo` para um cliente chinês mais novo; o binário
  PWBR 2018 envia 23 campos (1704 bytes; o layout de 24 prevê 1705 — um
  Octets vazio a mais). O servidor lia `rank` na posição do byte de
  comprimento de `referid` e lançava exceção.
- **Fix:** remoção de `rank` de `RoleInfo` + 3 sítios de referência (commit
  `0c445e35`); `GRoleStatus.rank` (renomeado de `reserved2`) permanece —
  compatível no wire enquanto vazio.
- **Evidência:** teste de conformança `tests/server153_createrole_wire.cpp`
  (1704 bytes, prefixo de 64 bytes idêntico à captura real, unmarshal limpo),
  integrado ao `verify.sh`.

## B10 — Spawn de personagem novo no mundo paralelo (design, não bug)

- **Sintoma:** personagem novo nasce no is61 (worldtag 161), não no mundo principal.
- **Causa raiz:** o template da classe vem de `GameDBManager::GetClsDetail`
  (storage `clsconfig`, importado a cada boot de `cnet/gamedbd/clsconfig`);
  tanto o vendored (90 KB) quanto o original PWBR (61 KB, md5 `dd08cebb…`)
  spawnam no is61 — design do reino original (`parallelworld_server=1`).
- **Fix:** substituído pelo arquivo original do pacote PWBR (spawns corretos
  para todas as classes). Ver [`08-WORLDS-AND-INSTANCES.md`](08-WORLDS-AND-INSTANCES.md).

## B11 — Crash do pai do gs no fork (SIGFPE cosmético)

- **Sintoma:** o processo pai do `gs` morre com SIGFPE ao sair.
- **Causa raiz:** ordem de destruição entre DSOs — destrutores de `ClassInfo`
  do `libskill.so` apagam do registro de classes já destruído (divisão por
  zero em `hashtab::get_hash`). Os processos das instâncias (netos) sobrevivem.
- **Fix:** guards de tabela vazia no hashtab (B15); o crash do pai permanece
  **cosmético** — não afeta as instâncias.

## B12 — Corrupção do rwlock do UserContainer (bloqueava entrada no mundo)

- **Sintoma:** EnterWorld trava para sempre; gdb mostra todas as threads em
  `pthread_rwlock_wrlock` sem holder; `__readers = -6` ao vivo.
- **Causa raiz:** `EnterWorld::Process` segurava o write lock e chamava
  `AnnouncePlayerOnline → WalkUser`, que pega `RDScoped` **aninhado no mesmo
  rwlock** → glibc `EDEADLK` → o destrutor aninhado destrava → o externo
  destrava DE NOVO → contador corrompido → todos os writers morrem de fome.
- **Fix (duas frentes):** (a) side-effects fora da região travada
  (`enterworld.hpp` reestruturado, commit `74df6d67`); (b) **estrutural**:
  `WRLock()`/`RDLock()` retornam int e `WRScoped`/`RDScoped`
  (`share/common/mutex.h`) só destravam o que conseguiram travar — aquisição
  aninhada vira no-op em vez de envenenar o contador (commit `bc2e616b`).
  A classe inteira de bug está morta.
- **Evidência:** traces `[enterworld] lock acquired → dispatching to
  gameid=101`; entrada no mundo com cliente real.

## B13 — Fila de auth (101) com 1 worker + EnterWorld inline na thread de poll

- **Sintoma:** TaskQueueSize preso em 7 com workers idle em futex;
  `EnterWorld::Process` nunca entrava (trace ausente).
- **Causa raiz (dois efeitos do mesmo rwlock corrompido de B12):** os RPCs
  de auth (`userlogin2.hrp:30`, `matrixpasswd2.hrp:29`, `gquerypasswd.hrp:27`,
  `matrixtoken.hrp`) despacham com `PriorPolicy=101` e o conf commitado tem
  UM worker nessa prioridade — travado no lock, acumulava a fila inteira de
  auth. `EnterWorld`/`CreateRole`/`RoleList` herdam prioridade 0 e rodam
  **inline na thread de poll** (`protocol.h:455-468`), que travava no mesmo
  lock — o daemon inteiro parava de processar I/O.
- **Fix:** o rwlock estrutural de B12 matou a causa; o runtime conf
  `(1,3)(100,1)(101,4)(0,1)` dá headroom na fila de auth (4 workers).
- **Nota:** a primeira redação deste finding atribuía prioridade 101 ao
  `EnterWorld` — corrigido após auditoria de 2026-10-28
  (`PriorPolicy()` default = 0 em `protocol.h:573`; override 101 existe só
  nos RPCs de auth).

## B14 — Use-after-free logout↔re-login (derrubava o gdeliveryd)

- **Sintoma:** usuário fecha o cliente e re-loga rápido → SIGSEGV no
  gdeliveryd (`hash<Octets>` ← `hash_map::find` ← `EraseName` ←
  `RoleLogout`) → todos os links entre servidores caem → "erro de
  comunicação entre servidores" para todos.
- **Causa raiz:** o logout antigo (`RoleLogout`) ainda roda quando o
  re-login (`RoleLogin`) faz `delete user->role`; `role->name` vira memória
  liberada e o `EraseName` hasheia o ponteiro morto.
- **Fix (duas camadas, commit `f6e0abb7`):** `UserLogout` segura o write
  lock do UserContainer durante todo o logout (serializa contra re-login;
  `EraseUser` vira `EraseUserNoLock`) + `RoleLogout` copia o nome da role
  (`Octets role_name_copy`) antes do caminho lento.
- **Evidência:** re-login do operador contra o build fixado sem segfault;
  dmesg sem crash novo (o único segfault é o pré-fix).

## B15 — Crash no caminho `erase` do hashtab (matava a instância is61)

- **Sintoma:** instância is61 morre em runtime; login da role (spawn no 161)
  falha `result=31, gameid=-1`.
- **Causa raiz:** o guard de tabela vazia cobria só o `find`; o `erase`
  (destrutores de `ClassInfo` apagando do registro destruído) ainda dividia
  por zero em `get_hash`.
- **Fix:** guard espelhado no `erase` (`cgame/include/hashtab.h`,
  commit `a4fc35aa`).

## B16 — worldtag 0 na criação (re-login não achava servidor)

- **Sintoma:** role criada com worldtag 0 não re-entra (nenhum gameserver atende).
- **Fix:** `if (status.worldtag == 0) status.worldtag = 1;` no
  `cnet/gamedbd/dbcreaterole.hrp`.

## Operacionais (não são bugs do código, são armadilhas do processo)

| # | Armadilha | Regra resultante |
| --- | --- | --- |
| O1 | `make` sem `-B` não viu mudanças de header → binário velho rodando com fix "aplicado" | Após editar headers: `make -B` + conferir mtime ([`03-BUILD.md`](03-BUILD.md)) |
| O2 | Watchdog reiniciava glinkd sem o argumento de instância `1` → morria na hora | glinkd sempre com `1` (já corrigido no script) |
| O3 | Zombies (state Z) enganavam o `alive()` (39 zombies gs-ours) | `alive()` filtra estado Z |
| O4 | Staging Cloudflare copiava storage `unamewdb` PELA METADE (`cp` falha em arquivos root) → uniquenamed não iniciava | staging remove TODO o storage ([`10-CLOUDFLARE.md`](10-CLOUDFLARE.md)) |
| O5 | Logs append-only → linhas antigas confundiam diagnóstico | contar delta de linhas por probe |
| O6 | `cd dir && nohup cmd &` backgrounda a cadeia inteira deixando o cwd do shell | usar subshell `(cd dir && nohup cmd &)` |
| O7 | Restart individual de daemon quebra links inter-daemon | restart completo ordenado ([`06-OPERATIONS.md`](06-OPERATIONS.md)) |

## Pendências conhecidas (não corrigidas)

- **Spawn Y abaixo do terreno** no is61 — cosmético, contorno via teleport GM.
- **Padrão latente em `PlayerLogin_Re`** (caminho cross-server) análogo ao
  rwlock aninhado do EnterWorld — documentado para follow-up.
- **Builds TSan/ASan** para caçar races restantes — milestone separado.
