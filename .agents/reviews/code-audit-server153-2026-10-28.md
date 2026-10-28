# Code Audit — server153 (PW 1.5.3) — 2026-10-28

## Resumo Executivo

- **Alvo:** `src/server153` (servidor Perfect World 1.5.3 comunitário derivado, C++ 32-bit) + scripts de operação + deploy Cloudflare, na worktree `~/workspace/pw/pw-source-153`, branch `feat/pw-source-1.5.3-ci-build`.
- **Escopo:** 8 áreas auditadas em paralelo por subagentes read-only (share/, glinkd, gdeliveryd, gamedbd+storage, daemons menores [gauthd/uniquenamed/gfaction/gacd/logservice], cgame/gs, cskill+PWSL, infra/scripts/Cloudflare) + varredura transversal (funções inseguras, `system()`).
- **Evidence level:** medium-high — código + confs + medições de runtime disponíveis; sem specs formais do upstream; sem execução de exploits (read-only).
- **Top risks:**
  1. **Wire DoS de 5 bytes no framework GNET** (`Octets::frob_size` loop infinito + `Protocol::Decode` reserve sem cap) — um único pacote trava a thread de poll de qualquer daemon, incluindo o glinkd exposto ao cliente.
  2. **Use-after-free sistêmico no UserContainer do gdeliveryd** — `FindUser`/`FindRole` retornam ponteiro cru sem lock para ~60 handlers; login/logout/kickout mutam os maps sem lock. Mesma família do crash real já corrigido (logout↔re-login), agora mapeada por completo.
  3. **Perda silenciosa de dados no gamedbd** — checkpoint de 60 s como único flush (RPO 60 s) + `catch(PageFile::Exception)` engolido: um RPC retorna sucesso sem ter gravado.
- **Release-blocking?** Para produção real com jogadores externos: **sim** — SEC-001/002/003, DATA-001/002/005 e OPS-004 são bloqueadores. Para o estágio atual (validação local com cliente real, deploy CF de prova): não bloqueiam o andamento, mas o watchdog atual cobre crashes, não exploits.

## Como este relatório se organiza

- Cada finding foi materializado como issue durável em `.agents/issues/<categoria>/<ID>-<título>.md` (36 issues, deduplicadas por root cause a partir de ~114 findings brutos).
- Os 8 relatórios brutos de área (com failure scenarios, validation e fix directions completos) estão em
  [`code-audit-server153-2026-10-28-areas/`](code-audit-server153-2026-10-28-areas/):
  `server153-audit-{share,glinkd,gdeliveryd,gamedbd,daemons,cgame,cskill,infra}.md`.
- Convencão de severidade/confidence: Critical/High/Medium/Low/Info × Confirmed/Likely/Possible/Speculative (definições no skill `code-audit`).

## Mapa Arquitetural Relevante (contexto de onde os bugs vivem)

- **Trust boundary #1:** cliente → glinkd:29000 (parsing GNET, state machine, halfloginset). Tudo que chega aqui é hostil.
- **Framework GNET** (`share/`): `Protocol::Decode` → stub → `Task::Dispatch` (prioridade 0 = inline na thread de poll; ≠0 = pool) → `Process()`. RPCs atravessam daemons via `Rpc`/`ProxyRpc`.
- **Orquestrador:** gdeliveryd — `UserContainer` (usermap/rolemap) + ~15 subsistemas de gameplay/economia.
- **Persistência:** gamedbd — engine WDB embutido (PageCache em memória + checkpoint-journal de 60 s; sem WAL por operação; CORRUPT é fatal no boot).
- **Gameplay:** gs (cgame) — fork por mundo; skills em `libskill.so` (~2.879 headers gerados); colisão/movemap carregados mas walkability NÃO validada no movimento do player.

## Achados Critical / High (resumo — detalhe nas issues e relatórios de área)

### Security
- **[Critical|Confirmed] SEC-001** — Wire DoS: `frob_size` (octets.h:93-98) loop infinito com `size=0xFFFFFFFF`; `Protocol::Decode` (protocol.h:528) `reserve(size)` sem cap; `Rep::create` placement-new sem check NULL. Pacote de 5 bytes trava qualquer daemon.
- **[High|Confirmed] SEC-002** — glinkd sem validação de posse em DeleteRole/UndoDeleteRole/SelectRole/SetCustomData/CollectClientMachineInfo (o padrão correto existe em createrole/getcustomdata).
- **[High|Confirmed] SEC-003** — Auth end-to-end fraca: dev-auth sem segredo (conta múltiplo de 16 = senha), zero rate-limit/lockout, `matrixtoken` aceita qualquer token, `QueryUserPrivilege` concede GM sem validar solicitante, `gquerypasswd` ignora challenge, nonce 8/16 bytes previsível, accounting MD5+memcmp.
- **[High|Confirmed] SEC-004** — Chaves de sessão reais commitadas no git (gamesys.conf do gdeliveryd/gacd, io.conf, boot script, entrypoint CF).
- **[High|Confirmed] SEC-005** — gacd: `ACReloadConfig` sem auth (ClearRules antes de validar = desativa toda punição), `xmlParseMemory` sem NULL-check (crash), `ACRemoteExe` injeta código no cliente sem auth.
- **[Medium|Confirmed] SEC-006** — logservice sem auth (UDP/TCP): forja de trilha de auditoria (priority/hostname/msg controlados), msg sem sanitização de newlines.
- **[Medium|Confirmed] SEC-007** — Command injection via `system()` com concatenação de paths no gamedbd (~20 sítios; `mergedb.cpp:895` concatena `d_name` de scandir).
- **[Medium|Confirmed] SEC-008** — Deploy CF: `/tcpprobe` público sem rate limit (backlog=10 do glinkd), Worker faz proxy de paths arbitrários, container roda como root.
- **[Medium|Confirmed] SEC-009** — Validação de input de economia ausente (AuctionOpen: item_num/baseprice/binprice/elapse_time/deposit) + `DBRawRead` expõe dados raw de qualquer tabela.
- **[Medium|Confirmed] SEC-010** — Dump `[decode]` de todos os pacotes em stderr sem `#ifdef` (débido pw-web): info leak + custo por pacote.

### Data / concorrência / integridade
- **[Critical|Confirmed] DATA-001** — UserContainer sem lock sistêmico: `FindUser(int)`/`FindRole(int)` crus em ~60 handlers; `RoleLogin`, `OnPlayerLogout`, kickouts, `RemoteLoginQuery_Re`, `UserLogin` overwrite, `CheckTimeoutUser`, `ClearRemoteUsers` — UAF/races da mesma família do crash real já corrigido.
- **[Critical|Confirmed] DATA-002** — `PlayerLogin_Re` aplica resposta stale sem validar `roleid/status/gameid` contra a sessão corrente.
- **[Critical|Confirmed] DATA-005** — WDB: RPO de 60 s (PageCache sem flush por operação) + `catch(PageFile::Exception)` engolido (`//abort()` comentado) = perda silenciosa reportada como sucesso.
- **[Critical|Confirmed] DATA-008** — cgame hashtab: guards só em find/erase; `get/nGet/put_noresize/find_or_insert/begin/elems_in_bucket` dividem por zero em tabela vazia (causa raiz: ordem de destruição DSOs do registro de ClassInfo).
- **[High|Confirmed] DATA-003** — ~15 subsistemas do gdeliveryd sem NENHUM lock (KingElection, StockExchange, Reference, Reward, WaitQueue, AutoTeam, PShop, Force, SoloChallenge, CountryBattle, TankBattle, CrossSystem, Friendext, PlayerProfile) + BattleManager declara locker e não usa + `GetAuctionID` sem lock.
- **[High|Confirmed] DATA-004** — gfaction muta vetor de membros sob **read-lock** (OnLeave/OnJoin/UpdateUser/UpdateFactionCache).
- **[High|Confirmed] DATA-006** — WDB CORRUPT fatal no boot sem recovery; `recover.cpp` é stub; backup `cp -r` sem checar retorno; `kill(0,SIGKILL)` no BackupThread; `times_incbackup %0`.
- **[High|Confirmed] DATA-007** — uniquenamed: nomes ENGAGED nunca reclamados (código morto), OBSOLETE nunca liberados → exaustão gradual de nomes + name-squat.
- **[High|Confirmed] DATA-009** — cgame: `CheckMove` (walkability) comentado — wall-walk dentro dos limites de velocidade; movemap carregado e não usado na validação.
- **[High|Confirmed] DATA-012** — cskill: `SkillWrapper::Swap` typo (`cd_adjust_count` ↔ `sw.cd_adjust`) — cooldowns corrompidos em troca de forma.
- **[High|Likely] DATA-013** — cskill: `GetState(1)` sem bounds; 389 skills de estado único = crash determinístico se atribuídos a NPC.
- **[High|Confirmed] DATA-014** — share/: `Timer::Update` itera observers sem lock (UAF) + `Conf::find` usa `operator[]` sob read-lock (data race em std::map + crescimento).
- **[Medium] DATA-010/011** — parallel_world Heartbeat sem lock vs PlayerSwitchWorld; `InitGUID` com `while(...);` (hang de boot).

### Operations / deploy
- **[High|Confirmed] OPS-001** — `stop_all` manda SIGTERM para gamedbd/uniquenamed (upstream usa SIGUSR1 graceful) — risco de CORRUPT no próximo boot.
- **[High|Confirmed] OPS-003** — Deploy CF cego: `/health` estático sempre `ok:true` (o CGI dinâmico nunca é referenciado) + `/probe` 404→503 "starting" permanente.
- **[High|Confirmed] OPS-004** — `entrypoint.sh` não aplica `version=10503`/`edition` do glinkd → o container Cloudflare rejeita o cliente real ("servidor suspenso"); o health não detecta.
- **[Medium|Confirmed] OPS-002/005/006** — watchdog sem backoff/coordenação; logs sem rotação; gs inicia com mall não convertido se python3/.old ausentes.
- **[Medium|Confirmed] OPS-008** — Build cskill inconsistente: `skill/makefile` sem `-m32` produz `.a` 64-bit; o path documentado não produz o `libskill.so` que o gs linka; stubs sem tracking de dependência de headers.
- **[Enhancement] OPS-007/009/010** — build de produção `-O0 -g` sem strip, `-w` suprime warnings, imagem 480 MB (multistage+strip→~250 MB), CI sem cache/jobs=2/artifact 120 MB sempre; performance transversal (PollIO single-thread, 2+ allocs por pacote, `Represent::Map` mutex global, broadcasts O(N), `GetOsbytes` WR por leitura, page cache ~1 GB em 32-bit, `Skill::Create` por operação).

## Gaps de Teste

- **TST-001** — Sem builds TSan/ASan no CI: toda a classe de UAF/races (DATA-001/003/004/010/014) seria capturada estaticamente em carga sintética.
- **TST-002** — Conformança de wire cobre só CreateRole: faltam challenge/login chain, pacote `frob` (regressão de SEC-001), e teste de `GetState(1)`.

## Gaps de Spec / Contrato

- Os confs upstream divergem entre vintages (portas, chaves, tables) — o boot script é a "spec" de runtime real; não há documento que fixe o contrato de confs (parcialmente resolvido em `docs/server/06-OPERATIONS.md`).
- `transactionacquire/commit/abort` são stubs vazios — se algum fluxo esperar 2PC, é silêncio.

## Riscos Arquiteturais e Operacionais

- O framework GNET processa pacotes inline na thread de poll (prioridade 0) — qualquer travamento ali (loop infinito, lock, fprintf) para o daemon inteiro; combinado com SEC-001/SEC-010, a thread de poll é o single point of failure de cada daemon.
- A persistência é um engine proprietário sem WAL, sem recovery e sem restore funcional — o RPO de 60 s e o CORRUPT fatal são riscos estruturais até DATA-005/006 serem endereçados.
- O watchdog converte crash em blip, mas não cobre exploits (DoS que não mata o processo) nem corrupção silenciosa.

## Segurança (síntese)

Confiança de rede: o glinkd é a única superfície do cliente, e o protocolo confia no cliente para posse de role (SEC-002), faixas de economia (SEC-009) e movimento (DATA-009). Internamente, os daemons confiam uns nos outros sem auth (gacd control-port, logservice, QueryUserPrivilege) — aceitável em localhost, mas as portas precisam nunca ser expostas (o deploy CF usa 127.0.0.1 interno — manter).

## Validação Recomendada (segura)

1. SEC-001: unit test `Octets::reserve(0xFFFFFFFF)` com timeout (não rodar contra o stack vivo).
2. DATA-001/002/003: build ASan do gdeliveryd + carga sintética de re-login/kickout (ambiente descartável).
3. OPS-004: `docker run` da imagem CF + `grep version $RUN/glinkd.conf` (deve dar 10204 hoje — o bug).
4. SEC-005: enviar XML vazio ao gacd em ambiente de teste (crash esperado — confirmar antes de fixar).

## Handoff Sugerido

- Todas as 36 issues → `.agents/issues/` (materializadas nesta sessão).
- SEC-001, DATA-001/002, OPS-004 → primeiros candidatos a `@implementation-loop` (maior ganho de estabilidade/segurança por esforço).
- TST-001 (TSan/ASan) → pré-requisito natural antes de fixar a família DATA-001/003/004.
- Decisões de produto (economia, auth real, rate limits) → user-only.

## Referências

- Relatórios de área: `code-audit-server153-2026-10-28-areas/server153-audit-{share,glinkd,gdeliveryd,gamedbd,daemons,cgame,cskill,infra}.md`
- Issues: `.agents/issues/{security,data,operations,testing}/`
- Fixes pw-web anteriores (contexto): `docs/server/09-BUGS-AND-FIXES.md` (B01–B16), `docs/SERVER153-REPORT.md` §4
- Commits de referência: `f6e0abb7` (logout↔re-login), `bc2e616b` (RWLock wrappers), `74df6d67` (enterworld), `a4fc35aa` (hashtab find/erase)
