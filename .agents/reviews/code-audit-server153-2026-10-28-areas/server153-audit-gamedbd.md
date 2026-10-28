# Auditoria de Código — gamedbd / storage (server153)

**Repo:** brasalabs6/pw-web, branch feat/pw-source-1.5.3-ci-build
**Área:** `src/server153/cnet/storage/` + `src/server153/cnet/gamedbd/` + engine `src/server153/share/common/db.h`
**Modo:** READ-ONLY, nenhum arquivo do repositório foi alterado.
**Data:** 2026-10-28

---

## Resumo executivo

O gamedbd usa o storage WDB (`USE_WDB`, Makefile:11): um engine proprietário em `share/common/db.h` (PageCache em memória + GlobalLogger de checkpoint-journal, sem WAL por operação). As "transações" são um journal de rollback **em memória** (prepared/new_key em storagewdb.h); não há marcador de commit no log. Consequência central: **crash perde tudo que não passou pelo checkpoint de 60s**, e **estado CORRUPT no boot é fatal** (GlobalLogger::truncate() retorna CORRUPT — db.h:2057 — e o gamedbd sai com `exit(-1)`).

---

## Findings

### [CRITICAL|Confirmed] Crash perde até 60s de escritas (checkpoint_interval=60s, sem WAL por operação)
- Type: data-consistency
- Evidence:
  - `src/server153/share/common/db.h:1507-1527` — `PageCache::put` só altera páginas em memória; nenhum fsync, nenhuma escrita de log por `put`.
  - `src/server153/cnet/storage/storagewdb.h:695-755` — `BackupThread`: único disparo de `StorageEnv::checkpoint()` em operação normal, a cada `checkpoint_interval` (60s em io.conf:9 e gamedbd.conf:53).
  - `src/server153/share/common/db.h:2944-2956` — o log (GlobalLogger) só recebe o snapshot no `prepare()`/`commit()` do checkpoint; `PageFile::sync()` (db.h:429) só é chamado no checkpoint.
- Failure Scenario: processo morre (SIGKILL via quit_lockfile, segfault, power loss, OOM kill) entre dois checkpoints. Todas as escritas desde o último checkpoint bem-sucedido — dinheiro, inventário, XP, posição, saveplayerdata, dbcreaterole — desaparecem. A resposta RPC já foi enviada ao gdeliveryd antes do crash, então o servidor acredita que o dado foi persistido.
- Impact: perda de até 60s de progresso de jogo por crash; no fluxo cross-server (saveplayerdata), o role pode "sumir" no destino enquanto a origem acha que migrou (data_timestamp não bate na re-tentativa).
- Validation (read-only): conferir que `Storage::insert` → `db->put` (storagewdb.h:258-276) não chama `sync()`; confirmar que nenhuma outra thread chama `StorageEnv::checkpoint()` em loop (grep confirma só BackupThread + OnQuit + ferramentas CLI).
- Fix Direction: reduzir `checkpoint_interval` (ex. 5-10s) e/ou adicionar fsync periódico do log; ou adicionar um WAL por operação com fsync batched; documentar o RPO (recovery point objective) de 60s como limite aceito.

### [CRITICAL|Confirmed] Estado CORRUPT no boot é fatal: GlobalLogger::truncate() retorna CORRUPT e não há recovery
- Type: operational-risk | data-consistency
- Evidence:
  - `src/server153/share/common/db.h:2057` — `State truncate() { return CORRUPT; }` (GlobalLogger).
  - `src/server153/share/common/db.h:2941-2956` — `DBCollection::init()`: `if (state == PageLogger::CORRUPT) state = logger->truncate();` → permanece CORRUPT → `return state == Logger::CLEAN` → false.
  - `src/server153/cnet/storage/storagewdb.h:580-589` — `StorageEnv::Open()` retorna false se `env->init()` falha.
  - `src/server153/cnet/gamedbd/gamedbd.cpp:87-90` — `if(!StorageEnv::Open()){ Log::log(LOG_ERR,"Initialize storage environment failed.\n"); exit(-1); }`.
  - Caminhos que produzem CORRUPT em `integrity_verify` (db.h:2015-2053): `load_log_file(max_logger_id)` falha ("some dbfile miss" — log atual sumiu/truncado), key_page do log com escrita rasgada, ou page 0 de uma tabela corrompida.
- Failure Scenario: crash durante o `commit()` do checkpoint (escrita de page 0 da tabela ou do key_page do log rasgada), disco com setor corrompido, ou `removeoldlogs`/backup removendo o log errado → no próximo boot `integrity_verify` retorna CORRUPT → boot trava com "Initialize storage environment failed". Não há `truncate` de recuperação para o GlobalLogger (ao contrário do PageLogger usado pelos DBStandalone, db.h:2790-2803, que recupera via abort()).
- Impact: indisponibilidade total do banco; sem caminho de auto-recuperação; depende de restore manual de backup (que por sua vez não tem ferramenta funcional — ver finding do recover.cpp).
- Validation (read-only): `grep truncate db.h` confirma os dois comportamentos; o boot path é rastreável em gamedbd.cpp:87.
- Fix Direction: implementar `GlobalLogger::truncate()` real (descartar checkpoint PREPARED e truncar o log — análogo ao `PageLogger::abort()`, db.h:1789-1798) e/ou, no boot, detectar CORRUPT e restaurar do último backup válido automaticamente.

### [HIGH|Confirmed] Tratamento de erro de disco desativado: `abort()` comentado em todos os catch de PageFile::Exception → perda silenciosa
- Type: data-consistency | bug
- Evidence: `src/server153/cnet/storage/storagewdb.h` — todos os `catch (GNET::__db_helper::PageFile::Exception)` têm `//abort();` comentado:
  - insert:276-279, find:309-312/332-335, del:362-365, checkpoint:245-248, walk:424-427/438-441/453-456/468-471, StorageEnv::checkpoint:639-642, BackupThread:720-723.
- Failure Scenario: um erro de I/O (ENOSPC, disco removido, quota) durante `insert` → a exceção é engolida, o RPC continua e retorna sucesso (ex. DBCreateRole retorna ERR_SUCCESS mesmo sem gravar). Durante `find` → retorna Octets vazio → unmarshal falha → transação aborta (menos grave, mas mascara o erro). Durante checkpoint → retorna false, loga, e o processo segue com dados não persistidos.
- Impact: corrupção/perda silenciosa de dados sob falha de disco; operadores não percebem até o próximo boot; o `//abort()` era a proteção original (comportamento upstream, não regressão pw-web — arquivo não rastreado no git).
- Validation (read-only): `grep -n "abort();" storagewdb.h` — confirmar que todos estão comentados; comparar com `storagebdb.h` (que propaga exceções BDB).
- Fix Direction: reabilitar o abort (ou equivalente: propagar exceção ao RPC e marcar o storage como read-only) para que falha de disco nunca seja reportada como sucesso.

### [HIGH|Confirmed] Caminho de restore inexistente: recover.cpp é um stub e o backup não valida erros
- Type: operational-risk
- Evidence:
  - `src/server153/cnet/storage/recover.cpp:28-30` — `StorageEnv::Open(); // StorageEnv::recover( srcdir.c_str() );` — o restore está comentado e `StorageEnv::recover` **não existe** (grep em cnet/ não acha nenhum `StorageEnv::recover`).
  - `src/server153/cnet/storage/storagewdb.h:665-688` — `backup()` usa `system("/bin/cp -r ...")`; `mkdir(destdir, 0755)` e o retorno do `system()` não são verificados; backup incremental copia só o logdir.
  - `src/server153/share/common/db.h:2969-2970` — `DBCollection::restore(timestamp)` existe mas não é exposto pelo StorageEnv nem por nenhuma ferramenta CLI.
- Failure Scenario: datadir corrompido/perdido → os backups em `./backupwdb/` existem mas não há ferramenta para restaurá-los (o `recover` não faz nada; o restore do GlobalLogger não é alcançável). Um backup que falha silenciosamente (cp retorna != 0) só é descoberto na hora do restore.
- Impact: a garantia de backup é nominal; em desastre real, não há procedimento operacional funcional no repo.
- Validation (read-only): `grep -rn "StorageEnv::recover\|\.restore(" src/server153/cnet/` — confirmar ausência.
- Fix Direction: implementar um comando `gamedbd ... restore <backupdir>` que faça: parar escritas, copiar datadir+logdir do backup, e chamar `DBCollection::restore()`/`redo()` para replay dos checkpoints incrementais; checar retorno de `system()`/`mkdir`.

### [MEDIUM|Confirmed] `InitGUID` com ponto-e-vírgula após `while` → hang no boot ou GUID sem re-checagem
- Type: bug
- Evidence: `src/server153/cnet/gamedbd/gamedbmanager.cpp:33` — `while(plog->find(key,value,txn));` — o corpo do while é vazio (ponto-e-vírgula); o bloco `{ key.clear(); guid += 0x100000000LL; key << guid; }` (linhas 34-37) executa **uma vez incondicionalmente**, fora do loop.
- Failure Scenario: (a) se a chave `guid` (timestamp<<32) já existir na tabela syslog (ex. reboot no mesmo segundo — Timer::GetTime() com resolução de 1s — ou entrada residual), `find` retorna true para sempre → loop infinito → gamedbd não sobe. (b) se não existir, o guid é incrementado uma vez sem verificar se o novo valor existe — a intenção (incrementar até achar livre) não é cumprida.
- Impact: hang de boot raro (mesmo segundo) ou colisão não detectada de GUID (syslog sobrescreve a mesma chave — PutSyslog em utilfunction.cpp:47-68 usa GetGUID() como chave).
- Validation (read-only): reler gamedbmanager.cpp:20-37; confirmar que `find` (bool) retorna true quando a chave existe.
- Fix Direction: remover o ponto-e-vírgula: `while(plog->find(key,value,txn)) { key.clear(); guid += 0x100000000LL; key << guid; }`.

### [MEDIUM|Likely] `Storage::find` (versão que retorna Octets) aborta a transação inteira em NOTFOUND
- Type: data-consistency | maintainability-risk
- Evidence:
  - `src/server153/cnet/storage/storagewdb.h:289-307` — `find(key, txn)` lança `WDB_NOTFOUND` **e chama `txn.abort(e)`** (set_cancel no ThreadContext). A versão bool `find(key, val, txn)` (linhas 318-341) retorna false **sem** abortar — as duas convivem.
  - Usos da versão que lança em caminhos onde "não encontrado" é possível: `saveplayerdata.hrp:107-115` e `:139-141` (`Marshal::OctetsStream(pbase->find(...)) >> tmp_base` dentro do scan do rolelist), `dbdeleterole.hrp:176` (`puser->find`), `gamedbmanager.cpp:365-370` (GetClsDetail).
- Failure Scenario: um slot do bitmap rolelist sem registro correspondente em `base` (inconsistência pós-crash/merge/delzoneplayers) → o `find` lança NOTFOUND → `txn.abort` → **todas as escritas já feitas na transação são revertidas** e o RPC inteiro falha (ERR_EXCEPTION). O comportamento é "all-or-nothing", mas o gatilho é um dado ausente que poderia ser tratado como caso normal.
- Impact: operações cross-server (saveplayerdata) falham inteiras por causa de um registro órfão; difícil de diagnosticar (o log só mostra "what=NOTFOUND").
- Validation (read-only): `grep -n "find(Marshal::OctetsStream\|pbase->find( key, txn\|->find(.*txn )"` nos .hrp para listar usos da versão Octets.
- Fix Direction: usar sempre a versão bool nos caminhos onde NOTFOUND é esperado; ou fazer a versão Octets lançar sem abortar (deixar o abort para o caller).

### [MEDIUM|Confirmed] `BackupThread` usa `kill(0, SIGKILL)` — mata o grupo de processos inteiro
- Type: operational-risk
- Evidence: `src/server153/cnet/storage/storagewdb.h:728` — `if( 0 != kill( 0, SIGKILL ) )	exit(0);` após detectar `quit_lockfile`.
- Failure Scenario: se o gamedbd for iniciado por um script/wrapper no mesmo process group (comum em init.d/systemd antigos ou `start.sh &`), o SIGKILL derruba também o script e outros serviços do grupo. Além disso, SIGKILL não passa por `DbPolicy::OnQuit` (gamedbd.cpp:32-35) — embora o checkpoint já tenha acabado de rodar na mesma iteração, o `StorageEnv::Close()` é pulado.
- Impact: shutdown não-graceful e colateral em outros processos; em ambientes de orquestração modernos (PID 1 em container), `kill(0)` pode matar o container inteiro.
- Validation (read-only): `ps -o pid,pgid` num ambiente de teste; conferir o código em storagewdb.h:722-729.
- Fix Direction: usar `kill(getpid(), SIGKILL)` ou, melhor, um sinal tratável (SIGTERM) com handler que faça checkpoint+Close.

### [MEDIUM|Likely] Page cache por tabela em páginas → ~1GB de RAM num processo 32-bit (risco de exaustão de address space)
- Type: performance-risk | operational-risk
- Evidence:
  - `src/server153/cnet/gamedbd/gamedbd.conf:55-63` — `cache_high_default=4096` (páginas), `base/status/equipment/inventory_cache_high=16384`.
  - `src/server153/share/common/db.h:52` — `PAGESIZE 4096`. 16384 páginas = 64MB por tabela; 4096 = 16MB.
  - `src/server153/cnet/gamedbd/Makefile:11` — build 32-bit (`-DMPPC_4WAY`, sem -m64).
- Failure Scenario: com a conf runtime de 48 tabelas (44×16MB + 4×64MB ≈ 960MB) o processo 32-bit (limite ~3GB de user space) consome ~1GB só de page cache, mais heap, buffers de rede (accumulate=16MB) e Octets temporários → OOM/address-space exhaustion sob carga, especialmente com dados grandes (mailbox, inventory).
- Impact: crash do gamedbd (que por sua vez expõe a janela de perda de 60s do finding #1).
- Validation (read-only): `grep cache_high io.conf gamedbd.conf`; calcular `sum(cache_high)*4096`.
- Fix Direction: dimensionar `cache_high` por tabela conforme o volume real (as tabelas quentes são poucas); considerar build 64-bit; monitorar RSS.

### [LOW|Confirmed] `times_incbackup` sem configuração → divisão por zero (SIGFPE)
- Type: bug | operational-risk
- Evidence: `src/server153/cnet/storage/storagewdb.h:738` — `if( !((times-1)%times_incbackup) )`; `times_incbackup = atol(conf->find("storagewdb","times_incbackup"))` (linha 710); `atol("") == 0`.
- Failure Scenario: conf sem `times_incbackup` (ou com 0) e `backup_lockfile` presente → `%0` → SIGFPE → crash do gamedbd no meio do ciclo de backup.
- Impact: crash (com a janela de perda do finding #1). Config atual (24) é segura.
- Validation (read-only): conferir gamedbd.conf:19/54 — valor presente.
- Fix Direction: `if (times_incbackup <= 0) times_incbackup = 1;` (ou validar no Open).

### [LOW|Confirmed] RPCs `transactionacquire`/`transactioncommit`/`transactionabort` são stubs vazios
- Type: maintainability-risk
- Evidence: `src/server153/cnet/gamedbd/transactionacquire.hrp:22-38`, `transactioncommit.hrp:22-38`, `transactionabort.hrp:22-38` — `Server()`, `Client()`, `OnTimeout()` todos vazios/comentados.
- Failure Scenario: se algum peer (gdeliveryd) enviar esses RPCs esperando semântica de transação distribuída (2PC), a resposta é um no-op silencioso. As transações reais de venda (sellpoint) usam `TransMan` (transman.cpp) com máquina de estados no storage `translog` — não estes RPCs.
- Impact: confusão de protocolo; se o gdeliveryd depender de acquire/commit/abort para algum fluxo, esse fluxo não tem efeito.
- Validation (read-only): `grep -rn "TransactionAcquire\|TransactionCommit\|TransactionAbort" cnet/` — verificar quem envia (aparentemente ninguém no repo).
- Fix Direction: remover os stubs ou implementar/documentar como vestigiais.

### [LOW|Confirmed] `DBRawRead` expõe dados raw (descomprimidos) de qualquer tabela configurada
- Type: security (defense-in-depth)
- Evidence: `src/server153/cnet/gamedbd/dbrawread.hrp:86-130` — valida o nome da tabela contra `tables` do conf (linhas 92-99), mas devolve pares chave-valor descomprimidos de qualquer tabela (user, base, mailbox, etc.) a qualquer peer conectado (gdeliveryd/linkd); browse limitado a 32 entradas/128KB (BrowseQuery:64-74), RPC maxsize=262144 (rpcalls.xml:1816).
- Failure Scenario: um gdeliveryd comprometido (ou um peer não autorizado que consiga conectar na porta 9402) pode despejar dados de jogadores inteiros (contas, inventários, nomes) sem autenticação adicional — o protocolo não distingue "ferramenta de inspeção" de "servidor de jogo".
- Impact: exfiltração de dados de jogadores se a rede interna for comprometida; é by-design (ferramenta de inspeção), mas não há controle de acesso.
- Validation (read-only): rpcalls.xml:11070 (registrado no protocolo do gamedbd); `grep -rn "DBRawRead" cnet/gdeliveryd` — nenhum chamador no repo (usado por ferramentas externas).
- Fix Direction: exigir flag de "admin" na sessão (ex. primeiro handshake) ou restringir a tabelas não sensíveis; documentar o risco.

### [LOW|Possible] `Uncompress` com DB corrompido → `len_src` gigante → malloc/segfault
- Type: bug | security (robustness)
- Evidence: `src/server153/share/io/compress.h:60-95` — `len_src` vem de CompactUINT no cabeçalho do valor; `os_src.reserve(len_src)` (linha 74) sem limite; `Octets::frob_size` (octets.h:92-98) com `while (size > tmp) tmp <<= 1;` — em 32-bit, `size > 2GB` faz `tmp` estourar para 0 → loop infinito; malloc de tamanho gigante → NULL → segfault em `Rep::create` (octets.h:108).
- Failure Scenario: um registro corrompido (bit rot, escrita rasgada) com cabeçalho de compressão inválido → crash do gamedbd ao ler esse registro. Não é explorável remotamente (dados vêm do DB), mas agrava o finding #2 (boot/leitura de dados corrompidos).
- Impact: crash; janela de perda de 60s.
- Validation (read-only): revisar compress.h:60-95 e octets.h:92-108.
- Fix Direction: limitar `len_src` ao tamanho do buffer de origem (ou a um máximo sanável) antes do `reserve`.

---

## Otimizações e melhorias

1. **Checkpoint mais frequente ou fsync assíncrono**: `checkpoint_interval=60` é o RPO. Reduzir para 5-10s reduz a perda máxima; o custo é I/O (cada checkpoint faz `restore_page` + 2-3 fsyncs por tabela). Alternativa: fsync do log a cada N segundos sem checkpoint completo.
2. **`cachesize` não é usado no WDB**: o campo `cachesize` do `[storage]` (io.conf:7) é do path BDB; no WDB o controle é `cache_high/low` por tabela. Ajustar as 4 tabelas quentes (base/status/inventory/equipment) e deixar o resto baixo (ex. 1024) reduz RSS.
3. **Backup**: checar retorno de `system()`/`mkdir` e logar falha; considerar `cp -a` e excluir `dblogs` do backup full (o log é recriado no boot); documentar o procedimento de restore (hoje inexistente).
4. **`removeoldlogs`** roda a cada 60 checkpoints (storagewdb.h:731-733) e antes do backup full; confirmar que o log corrente (`log.<max_logger_id>`) nunca é removido — `remove_logs()` (db.h:2977-2988) usa `ts < time_stamp` (log atual), correto, mas o `unlink` em storagewdb.h:655 não checa erro.
5. **`ThreadContext::lock()` ordena e trava TODAS as storages** (storagewdb.h:759-767): qualquer transação serializa o banco inteiro. Para RPCs de leitura pura (getrole, getuser), um lock compartilhado (rwlock) por storage reduziria contenção — mas exige cuidado com o snapshot do checkpoint.
6. **`snapshot_create` libera o lock do storage antes do `checkpoint_prepare`** (storagewdb.h:205-209): o design depende do snapshot COW do PageHash (db.h:1075-1103); é correto, mas significa que o checkpoint não é "instantâneo" — escritas durante o commit vão para o próximo checkpoint (janela de perda efetiva pode ser < 60s se o commit for lento).

## Fatos verificados vs inferências

**Verificados (lidos diretamente no código):**
- `PageCache::put` não faz fsync nem escreve log (db.h:1507-1527); checkpoint é o único flush; `checkpoint_interval=60` (io.conf:9, gamedbd.conf:53); BackupThread é o único loop de checkpoint (storagewdb.h:695-755).
- `GlobalLogger::truncate()` retorna CORRUPT (db.h:2057); `DBCollection::init()` propaga (db.h:2941-2956); gamedbd sai com exit(-1) (gamedbd.cpp:87-90).
- Transações WDB = journal de rollback em memória (prepared/new_key, storagewdb.h:225-249); `Transaction::commit()` é no-op; abort via set_cancel; checkpoint não intercala com transações (locks globais em ordem ordenada).
- `abort()` comentado em todos os catch de PageFile::Exception do storagewdb.h (upstream; arquivo não rastreado no git — `git ls-files` vazio).
- `InitGUID` com `while(...);` (gamedbmanager.cpp:33) — bug de ponto-e-vírgula confirmado.
- `recover.cpp` stub com restore comentado; `StorageEnv::recover` inexistente.
- RPCs transactionacquire/commit/abort vazios; transações reais de venda via TransMan (transman.cpp) com status no storage `translog`.
- Marshal faz bounds-check (`pop_byte` lança Exception, marshal_i386.h:43-47; Octets unmarshal checa `len > size-pos`, :361-368) — RPCs de gs/gdeliveryd não têm overrun de memória, mas dados corrompidos do DB podem causar crash no Uncompress.
- `kill(0, SIGKILL)` no quit_lockfile (storagewdb.h:728); `times_incbackup %0` (storagewdb.h:738).
- 48 storages confirmados em `dbhomewdb/dbdata/`; conf do repo tem 24 tabelas (gamedbd.conf:52) — a conf runtime com 48 é o fix pw-web já aplicado (não reportado).
- DBRawRead: valida tabela contra conf, devolve dados descomprimidos, browse limitado a 32/128KB.

**Inferências (não executadas — ambiente read-only):**
- "Crash durante commit do checkpoint → CORRUPT no boot" é inferido da análise de `integrity_verify` (db.h:2015-2053) e das escritas rasgadas possíveis; a probabilidade exata depende de timing de fsync. O caminho de código está confirmado.
- "~1GB de page cache com 48 tabelas" é aritmética sobre `cache_high`×PAGESIZE; o consumo real depende do padrão de acesso (o cache só cresce até `cache_high` sob carga).
- O gatilho do loop infinito do `InitGUID` (mesmo segundo de reboot) depende da resolução de `Timer::GetTime()` — não verificado; o defeito do código é certo.
- Nenhum teste/execução foi feito (auditoria read-only); todas as validações acima são por leitura de código e grep.

## Nota de escopo
- Não reportados (fixes pw-web já aplicados): conf runtime com 48 tables + accepted_zone_list + cross_zone_pos + CASHVIP; dbcreaterole.hrp worldtag=1; clsconfig original PWBR.
- Nenhum arquivo do repositório foi modificado; documento salvo em `/tmp/server153-audit-gamedbd.md`.