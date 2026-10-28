# Auditoria de código — gdeliveryd (src/server153/cnet/gdeliveryd)

Repo: brasalabs6/pw-web, branch feat/pw-source-1.5.3-ci-build
Modo: READ-ONLY. Nenhum arquivo do repositório foi editado.
Data: 2026-10-28

## Resumo executivo

O gdeliveryd ainda carrega o padrão de bug que já derrubou o servidor uma vez
(lock aninhado no MESMO rwlock do UserContainer, corrigido em enterworld.hpp /
map.cpp / statusannounce.hpp). O problema sistêmico remanescente é o inverso e
igualmente grave: **a maioria dos caminhos de login/logout/kickout acessa o
UserContainer (usermap/rolemap) SEM nenhum lock**, via `FindUser(int)` e
`FindRole(int)` que retornam ponteiro cru para dentro dos maps. Isso produz
use-after-free (UserLogout → EraseUserNoLock destrói o UserInfo enquanto outro
thread o usa), data races em `rolemap` (RoleLogin sem lock) e TOCTOU entre
check e logout. Somam-se a isso ~15 subsistemas com estado compartilhado entre
IntervalTimer::Update() e handlers de protocolo sem nenhuma sincronização, e
uma resposta stale de PlayerLogin_Re que corrompe a sessão corrente.

---

## Findings

### [CRITICAL|Confirmed] PlayerLogin_Re aplica resposta "stale" à sessão corrente sem verificar correspondência
- Type: bug | data-consistency
- Evidence: `playerlogin_re.hpp:100-227` — `Process` valida apenas `FindUser(userid) != NULL`; NÃO verifica `pinfo->roleid == roleid`, `pinfo->status == _STATUS_SELECTROLE` nem `pinfo->gameid == src_provider_id`. Em `result==ERR_SUCCESS` chama `RealLogin(roleid, pinfo)` (linha 164) e envia `PlayerLogin_Re` para `pinfo->linksid` (linha 175); em `result==2||5` (linha 165-169) faz `pinfo->gameid = _GAMESERVER_ID_INVALID` + `ResetRequest`.
- Failure Scenario: o cliente pede login do role A; o PlayerLogin vai para o GS A. Enquanto o GS responde, o usuário é kickado (UserLogout) e re-loga (novo UserLogin/PlayerLogin) — ou loga outro role B. A resposta antiga do GS A chega depois: `FindUser` retorna a sessão NOVA, `RealLogin` executa side-effects (QueryRewardType para o GS atual, `role->name`/`cls` do role A, `PostOffice::OnRoleOnline`) e o `PlayerLogin_Re` é enviado ao link da sessão nova. Se o resultado for 2/5 (max player/instance), o `gameid` da sessão NOVA é zerado, quebrando um login em andamento ou até um personagem já em jogo.
- Impact: corrupção de estado de sessão, personagem "fantasma" (cliente acha que logou, EnterWorld falha), login/instância quebrados. Multiplos jogadores afetados sob re-login rápido.
- Validation: read-only — instrumentar `PlayerLogin_Re::Process` com log de `pinfo->roleid/status/gameid` vs `roleid/src_provider_id` e comparar com os logs de UserLogout/UserLogin do mesmo userid; ou revisar o fluxo com um teste de re-login concorrente.
- Fix Direction: antes de aplicar a resposta, validar sob o WR lock que `pinfo->roleid == roleid && pinfo->status == _STATUS_SELECTROLE && pinfo->gameid == src_provider_id`; caso contrário descartar a resposta (e devolver PlayerOffline ao GS de origem). Idealmente casar a resposta ao request (token/sequência).

### [CRITICAL|Confirmed] RoleLogin modifica rolemap e user->role sem nenhum lock (delete de PlayerInfo sob ponteiros crus)
- Type: bug | use-after-free
- Evidence: `mapuser.h:659-672` — `RoleLogin` faz `rolemap.erase(user->role->roleid); delete user->role;` e `rolemap[roleid] = role` sem lock. Chamado sem lock em `playerlogin.cpp:112` (DoLogin), `playerlogin.cpp:155` (TryRemoteLogin) e `waitqueue.cpp:266` (ContinueLogin). `FindRole`/`FindRoleOnline` (sem lock) devolvem `PlayerInfo*` crus usados em dezenas de handlers; `RoleLogout` (map.cpp:272-344) acessa o mesmo `rolemap` sob WR lock.
- Failure Scenario: thread T1 executa `PlayerLogin::Process` → `FindUser` (sem lock) → `DoLogin` → `RoleLogin` (sem lock: erase+delete+insert em rolemap). Thread T2 executa `UserLogout` → `RoleLogout` sob WR lock (erase+delete do mesmo role) ou um handler qualquer lê `FindRole(roleid)` cru. O `delete user->role` de T1 libera um `PlayerInfo` que T2 ainda está usando (ou vice-versa) → heap-use-after-free; o `rolemap` std::map é mutado concorrentemente → UB.
- Impact: crash do gdeliveryd (segfault/UAF) sob re-login/logout concorrente — mesmo padrão do bug já corrigido em RoleLogout, mas no lado do login.
- Validation: read-only — rodar com ASAN e forçar re-login rápido (kick + login imediato) em 2+ conexões; ou revisar todos os callers de RoleLogin e confirmar ausência de lock.
- Fix Direction: `RoleLogin` deve ser executada sob o WR lock do UserContainer (como RoleLogout), e os callers (DoLogin, TryRemoteLogin, ContinueLogin) devem obter `pinfo` sob o lock e revalidar estado antes de chamá-la.

### [CRITICAL|Confirmed] OnPlayerLogout executa RoleLogout (delete role) sem lock
- Type: bug | use-after-free
- Evidence: `map.cpp:346-360` — `OnPlayerLogout` chama `FindRole(cmd.roleid)` (sem lock), lê `role->user`, e chama `RoleLogout(user)` (sem lock) que faz `rolemap.erase(user->roleid); delete role;` (map.cpp:336-337). Chamado de `playerlogout.hpp:32` (protocolo do GS, thread do pool).
- Failure Scenario: GS manda PlayerLogout enquanto outro thread processa EnterWorld (que segura WR lock e usa `pinfo`), ou um handler lê `FindRole(roleid)`. O `delete role` roda sem lock → UAF no leitor concorrente; `rolemap.erase` concorre com `RoleLogin`/`FindRole`.
- Impact: crash; estado de rolemap inconsistente (role presente no map mas com `user->role` já nulo).
- Validation: read-only — ASAN + PlayerLogout concorrente com EnterWorld/PlayerLogin; grep dos callers de FindRole sem lock.
- Fix Direction: `OnPlayerLogout` deve tomar o WR lock do UserContainer para o FindRole + RoleLogout (ou reutilizar o caminho de UserLogout), e validar `role->user->linkid/localsid` sob o lock.

### [CRITICAL|Confirmed] FindUser(int)/FindRole(int) sem lock — ponteiro cru para usermap/rolemap usado em ~60 handlers
- Type: bug | use-after-free
- Evidence: `mapuser.h:591` (`UserInfo* FindUser(int)` sem lock), `mapuser.h:599` (`FindRole` sem lock). Callers sem lock: `playerlogin.hpp:31`, `kickoutuser.hpp:31`, `gmkickoutuser.hpp:27`, `ackickoutuser.hpp:24,139`, `remoteloginquery_re.hpp:31`, `waitqueue.cpp:233,318`, `map.cpp:423,474,482`, `autoteamman.cpp:130,139,157,183,328,615,662`, `countrybattleman.cpp:577,1118,2577,2622`, `cdcmnfbattleman.cpp:892`, `cdsmnfbattleman.cpp:1129,1690,1744,1829`, `addcash_re.hpp:27`, `webtraderoleprepost.hpp:36`, `rolelist.hpp:52`, `qpaddcash.hpp:25`, etc.
- Failure Scenario: handler H obtém `pinfo = FindUser(userid)` (cru). Thread concorrente executa `UserLogout` → `EraseUserNoLock` (map.cpp:276) que destrói o `UserInfo` e desaloca o nó do map. H lê `pinfo->localsid/linksid/status` → heap-use-after-free. Alternativamente `UserLogin` (map.cpp:199) sobrescreve o `UserInfo` via `operator=` enquanto H escreve campos → dados rasgados.
- Impact: crash intermitente do gdeliveryd (derruba todos os links inter-servidor), comportamento indefinido em kickout/login/logout concorrentes.
- Validation: read-only — ASAN com carga de re-login/kickout; ou contagem estática: todos os callers de `FindUser(int)`/`FindRole` que não estão sob `GetLocker()`.
- Fix Direction: ou tornar `FindUser(int)`/`FindRole` internamente lock-safe (retornar cópia / shared_ptr), ou — mudança mínima — fazer cada caller adquirir `RDScoped`/`WRScoped` do `GetLocker()` e revalidar o estado antes de usar o ponteiro, com o ponteiro nunca usado fora do escopo do lock.

### [HIGH|Confirmed] RemoteLoginQuery_Re::Process opera inteiramente sem lock (RealLogin + escrita de status + UserLogout)
- Type: bug | use-after-free
- Evidence: `remoteloginquery_re.hpp:31-72` — `FindUser(userid)` sem lock (linha 31), leitura de `pinfo->status` (32), `PlayerLogin_Re::RealLogin(roleid, pinfo)` sem lock (64), `pinfo->status = _STATUS_READYGAME` (65), `UserLogout(pinfo)` (68). Note que o caminho local `playerlogin_re.hpp:164` chama `RealLogin` DENTRO do WR lock; o caminho cross-server não.
- Failure Scenario: resposta cross-server (RemoteLoginQuery_Re) chega para um userid que acabou de ser deslogado/re-logado localmente → `pinfo` dangling ou sessão errada; `RealLogin` e `UserLogout` operam sobre estado corrompido.
- Impact: UAF/crash no caminho cross-server; sessão remota corrompida.
- Validation: read-only — ASAN + teste de cross-server login com kick concorrente; comparar com o caminho local que usa lock.
- Fix Direction: envolver o bloco inteiro (FindUser → RealLogin/UserLogout) no WR lock do UserContainer, revalidando `pinfo->status == _STATUS_REMOTE_LOGINQUERY` sob o lock.

### [HIGH|Confirmed] TOCTOU em KickoutUser/GMKickoutUser/ACKickoutUser/UserLogin2: FindUser sem lock seguido de UserLogout
- Type: bug
- Evidence: `kickoutuser.hpp:30-44` (`FindUser` → `gdsm->Send(pinfo->linksid,...)` → `UserLogout(pinfo, KICKOUT_LOCAL, true)`); `gmkickoutuser.hpp:27-31`; `ackickoutuser.hpp:23-28`; `userlogin2.hrp:36-60` (TryLogin) e `139-170` (PostProcess) — check sob RD lock, libera, depois `KickoutUser ku(...); ku.Process(NULL,0)` (que refaz FindUser sem lock) e `UserLogin`.
- Failure Scenario: entre o `FindUser` e a aquisição do WR lock dentro de `UserLogout`, outro thread erases o usuário → `pinfo` dangling dentro de `UserLogout` (lê `pinfo->status`, `pinfo->role`). Ou o usuário re-loga no intervalo → o kickout derruba a sessão NOVA, e o `UserLogin` subsequente do TryLogin sobrescreve/é sobrescrito.
- Impact: kickout da sessão errada; UAF; duplicidade de sessões (dois UserInfo para o mesmo userid em janela curta).
- Validation: read-only — ASAN + kickout e re-login simultâneos do mesmo userid; revisar a sequência TryLogin/PostProcess.
- Fix Direction: fazer o par FindUser+UserLogout atômico sob o WR lock (UserLogout já o toma; mover o FindUser para dentro). Em UserLogin2, refazer o check de "user online" sob o mesmo lock do UserLogin.

### [HIGH|Confirmed] StatusAnnounce: TOCTOU residual entre o check (WR lock) e o UserLogout (fora do lock)
- Type: bug
- Evidence: `statusannounce.hpp:36-73` — check de `pinfo->localsid==localsid` sob WR lock (linhas 39-64), libera o lock, e depois `FindUser(uid)` sem lock + `UserLogout(pinfo)` (linhas 69-73). O comentário pw-web explica que o UserLogout foi movido para fora do lock para evitar o deadlock de lock aninhado.
- Failure Scenario: glinkd desconecta (StatusAnnounce) enquanto o usuário re-loga via UserLogin2::PostProcess. O segundo `FindUser` retorna a sessão NOVA (ou dangling se o erase já ocorreu) → `UserLogout` derruba a sessão recém-logada.
- Impact: jogador derrubado logo após login bem-sucedido; UAF em janela estreita.
- Validation: read-only — ASAN + reconnect rápido do mesmo userid; log de userid/localsid nas duas janelas.
- Fix Direction: manter o check e o logout no mesmo WR lock (o deadlock original era do `Size()`/`EraseUser` aninhados — usar `SizeNoLock`/`EraseUserNoLock` dentro do lock, como já feito em map.cpp:276, elimina a necessidade de sair do lock).

### [HIGH|Confirmed] UserLogin sobrescreve UserInfo existente sem cleanup (leak + estado rasgado)
- Type: data-consistency
- Evidence: `map.cpp:199` — `usermap[userid] = ui;` sob WR lock. `UserInfo` não define `operator=` (mapuser.h:240-268), então o assignment member-wise sobrescreve os ponteiros `role` e `acstate` do registro antigo: o `PlayerInfo*` e o `AddictionControl*` antigos vazam (destrutor só roda no erase, não no assignment).
- Failure Scenario: dois `UserLogin` concorrentes para o mesmo userid (duplicidade de login, retry de gauthd, TryLogin+PostProcess simultâneos) → o segundo sobrescreve o primeiro; leitores com ponteiro para o registro antigo veem estado rasgado; `role`/`acstate` antigos vazam.
- Impact: leak de memória por duplicidade de login; leituras inconsistentes; possível perda do `role` ativo.
- Validation: read-only — revisar os callers de UserLogin (userlogin2.hrp:60,170; gauthd) e confirmar que não há serialização; valgrind/ASAN com login duplicado.
- Fix Direction: em `UserLogin`, se `userid` já existe, decidir explicitamente (kick da sessão antiga via UserLogout antes de inserir, ou rejeitar) em vez de sobrescrever silenciosamente.

### [HIGH|Confirmed] RemoteLoggingUsers::CheckTimeoutUser e ForbiddenUsers::CheckTimeoutUser iteram/erase set sem o locker
- Type: bug
- Evidence: `map.cpp:66-87` (ForbiddenUsers::CheckTimeoutUser itera `set` e faz `set.erase` sem `Thread::Mutex::Scoped l(locker)`); `map.cpp:415-431` (RemoteLoggingUsers::CheckTimeoutUser itera `user_map.set` sem lock e chama `FindUser` sem lock + `UserLogout`).
- Failure Scenario: o timer RemoveForbidden (HouseKeeper) roda concorrente com `Push`/`Pop` de ForbiddenUsers (feitos sob lock) → mutação concorrente do std::map (UB). No caso RemoteLoggingUsers, `FindUser` sem lock + `UserLogout` com lock → TOCTOU/UAF.
- Impact: crash; usuários em timeout de login remoto não limpos corretamente (sessão presa).
- Validation: read-only — ASAN + login remoto com timeout curto; revisar se `CheckTimeoutUser` deveria tomar o locker.
- Fix Direction: tomar `Thread::Mutex::Scoped l(locker)` (ou o lock do user_map) durante a iteração+erase; mover o FindUser+UserLogout para um único WR lock do UserContainer.

### [HIGH|Confirmed] ClearRemoteUsers / DisconnectZoneUsers iteram usermap sem lock
- Type: bug
- Evidence: `map.cpp:439-451` (`ClearRemoteUsers` itera `usermap` sem lock e chama `UserLogout(&(itmp->second),0,true)` que toma WR lock e erase o mesmo registro) e `map.cpp:452-467` (`DisconnectZoneUsers`, mesmo padrão).
- Failure Scenario: iteração sem lock concorre com UserLogin/UserLogout de outros threads; `UserLogout` erase o registro `itmp` enquanto a iteração continua → iteração sobre map mutado (UB), ponteiro para registro destruído.
- Impact: crash durante shutdown de zona / limpeza de usuários remotos.
- Validation: read-only — revisar os callers (desconexão de zona, shutdown) e confirmar se há lock externo; ASAN em teste de desconexão de zona com login concorrente.
- Fix Direction: iterar sob o WR lock do UserContainer (ou coletar userids primeiro sob RD lock e depois logar cada um).

### [HIGH|Confirmed] ~15 subsistemas com estado compartilhado entre IntervalTimer::Update() e handlers sem NENHUM lock
- Type: bug
- Evidence: sem `Thread::Mutex`/`RWLock` em: `kingelection.h:70-80` (_status/_king/_candidate_list/_max_serial_num), `stockexchange.h:139-220` (accounts/orders/volumes/pending/nextid; `Lock()/UnLock()` em stockexchange.h:82-103 é flag lógico, não mutex), `referencemanager.h:105-160` (referrers/referrals/transmap/refcodemap; comentário "Update方法里写入和WithdrawBonusTrans写库可能存在时序问题 liuguichen 20121210" confirma o problema conhecido), `rewardmanager.h:75-130` (rewards/wb_cursor), `waitqueue.h:40-100` (_users/_queues/_average_waittime), `autoteamman.h` (_players/goals), `friendextinfomanager.h`, `playerprofileman.h:73-95` (PROFILE_ENTRY_MAP/_state), `pshopmarket.h`, `forcemanager.h:33-45` (FORCE_LIST/FLAG_LIST), `solochallengerank.h:34-70` (RankMap), `countrybattleman.h`, `tankbattlemanager.h`, `crosssystem.h:44-57` (identity_map). Todos têm `Update()` no IntervalTimer (ex.: battlemanager.cpp:855, countrybattleman.cpp:1452, forcemanager.cpp:49, autoteamman.cpp:351, solochallengerank.cpp:154, tankbattlemanager.cpp:208, waitqueue.cpp:330, stockexchange.cpp, kingelection.cpp:125) e handlers de protocolo no pool de threads.
- Failure Scenario: `KingElection::Update()` (timer) muta `_candidate_list` enquanto `TryCandidateApply`/`TryVote`/`OnLogin` (pool) leem/escrevem o mesmo vetor → data race em std::vector/map (UB). `WaitQueueManager::Update()` (timer) chama `IniviteUser` → `ContinueLogin` → `FindUser`+`RoleLogin` sem lock enquanto `OnPlayerLogin`/`OnLogout` (pool) mutam `_users`. `StockExchange::Update()`/`AutoBalance` (timer) concorrem com `PlaceOrder`/`PriceMatching` (pool) sobre `nextid` e `orders`.
- Impact: corridas em eleição de rei, fila de espera, bolsa, referência/recompensa, autoteam, pshop, ranking — crashes e corrupção de dados econômicos sob carga.
- Validation: read-only — TSAN com carga sintética (login/logout + votos + ordens de bolsa); contagem estática de `IntervalTimer::Attach` vs uso de lock.
- Fix Direction: cada subsistema com estado mutável precisa de um mutex próprio (padrão já usado por AuctionMarket/WebTradeMarket/FactionFortressMan/UniqueDataServer/SysAuctionManager), cobrindo Update() e handlers; ou serializar o Update() do timer com os handlers via fila.

### [HIGH|Confirmed] BattleManager declara RWLock locker mas nunca o usa
- Type: bug
- Evidence: `battlemanager.h:60` declara `Thread::RWLock locker;` (inicializado em `battlemanager.h:80`), mas `battlemanager.cpp` só usa `UserContainer::GetInstance().GetLocker()` (linhas 462, 478). `status`, `rand_num` e os dados de domínio são acessados de `Update()` (battlemanager.cpp:855) e dos handlers sem sincronização.
- Failure Scenario: `Update()` do timer (sorteio de domínio, limpeza) concorre com handlers de battle (battleenter/battlestart) → corrida nos dados de domínio.
- Impact: estado de batalha inconsistente; crash sob carga de batalha.
- Validation: read-only — grep confirma zero usos do `locker` próprio; TSAN com batalhas simultâneas.
- Fix Direction: usar o `locker` declarado em todos os métodos públicos (Update + handlers), ou removê-lo se o design for single-thread (documentar).

### [MEDIUM|Confirmed] AuctionMarket::GetAuctionID sem lock — IDs de leilão duplicados
- Type: bug
- Evidence: `auctionmarket.cpp:348-364` — `++m_maxAuctionID_` fora de `m_lockermap_` (todos os outros métodos usam o lock). Único caller: `auctionopen.hpp:102` (`AuctionOpen::Process`, que segura o RD lock do UserContainer, não o do AuctionMarket).
- Failure Scenario: dois `AuctionOpen` concorrentes (threads diferentes) geram o mesmo auctionid → `m_mapAuctionID_[id]` sobrescrito (AddAuction/UpdateAuction sob lock) → um leilão some; referências em `m_mapRoleAuction_`/`m_mapTime_` inconsistentes.
- Impact: leilões perdidos/duplicados; dinheiro/items presos.
- Validation: read-only — TSAN ou contagem de IDs sob 2 AuctionOpen simultâneos; grep confirma ausência de lock no método.
- Fix Direction: tomar `Thread::Mutex::Scoped l(m_lockermap_)` em GetAuctionID (ou um atomic).

### [MEDIUM|Confirmed] AuctionOpen valida mal entrada do cliente (item_pos/item_num/baseprice/binprice/elapse_time/deposit)
- Type: security
- Evidence: `auctionopen.hpp:38-60,84-118` — `CheckCondition` valida apenas categoria/limite de leilões/mailbox; `item_pos`, `item_num`, `baseprice`, `binprice`, `elapse_time`, `deposit` são repassados sem validação a `DBAuctionOpenArg` (linhas 43-57). `AuctionGet` (auctionget.hpp:31-49) aceita `auctionid` arbitrário (fallback para DB).
- Failure Scenario: cliente (ou glinkd comprometido) envia `item_num` negativo/huge, `baseprice` negativo, `elapse_time` gigante → valores propagados ao gamedbd e ao cálculo de end_time (`elapse_time+now+300`); leilão com preço/duração inválidos.
- Impact: economia corrompida (preços negativos, durações absurdas), carga indevida no gamedbd.
- Validation: read-only — revisar os limites em gamedbd (dbauctionopen) e comparar; teste de fuzz de protocolo.
- Fix Direction: validar faixas em gdeliveryd antes de QueryDB (item_num>0, item_pos>=0, baseprice/binprice>0 e <= teto, elapse_time em [min,max], deposit>=0), no mesmo padrão de pshopbuy.hpp:66-69.

### [MEDIUM|Confirmed] ReferenceManager::OnLogin fora do lock vs OnLogout dentro do lock (sem lock interno)
- Type: bug
- Evidence: `enterworld.hpp:117` chama `ReferenceManager::GetInstance()->OnLogin(user)` FORA do WR lock do UserContainer (pós-fix pw-web); `map.cpp:308` chama `ReferenceManager::GetInstance()->OnLogout(user)` DENTRO do WR lock (RoleLogout). ReferenceManager não tem lock interno (referencemanager.h:105-160); `Referral::CheckBonusWithdrawToday` (referencemanager.cpp:64-80) chama `FindUser` sem lock.
- Failure Scenario: OnLogin (thread do pool, sem lock) muta `referrals`/`transmap` enquanto OnLogout (thread com WR lock) lê o mesmo mapa, ou o Update() do timer roda `WithdrawBonusTrans` → corrida nos dados de referência/transação (o comentário liuguichen 20121210 já documenta o timing issue).
- Impact: bônus de referência incorretos, transações de saque duplicadas/perdidas.
- Validation: read-only — TSAN; revisar o fluxo OnLogin/OnLogout/Update.
- Fix Direction: dar a ReferenceManager (e RewardManager, mesmo padrão) um mutex interno cobrindo OnLogin/OnLogout/Update/transações, independente do lock do UserContainer.

### [LOW|Confirmed] Network sends sob o WR/RD lock do UserContainer
- Type: performance-risk
- Evidence: `playerlogin_re.hpp:175` (`dsm->Send(pinfo->linksid,this)` sob WR lock), `kickoutuser.hpp:38` (`gdsm->Send` sob... sem lock, pior), `auctionopen.hpp:106-112` (QueryDB+SyncGameServer sob RD lock), `tradestart.hpp:90` (`dsm->Send` sob RD lock), `webtradeprepost.hpp:50-66` (TryPrePost+SyncGameServer sob RD lock).
- Failure Scenario: envios de rede (potencialmente bloqueantes) seguram o lock global do UserContainer; sob carga, o lock fica retido e todos os logins/EnterWorld (que precisam do WR lock) atrasam.
- Impact: latência de login/logout; risco de inanição do lock (mesma família do incidente do rwlock corrompido).
- Validation: read-only — medir tempo de retenção do lock com instrumentação; revisar os caminhos que enviam sob lock.
- Fix Direction: coletar os dados sob o lock e fazer os sends fora dele (padrão já aplicado em enterworld.hpp).

### [LOW|Confirmed] Broadcasts O(N) por evento de player (login/logout)
- Type: performance-risk
- Evidence: `map.cpp:282-287` (RoleLogout: `GProviderServer::BroadcastProtocol(psa)` + `GFactionClient::SendProtocol(psa)` por logout), `enterworld.hpp:40-41` (AnnouncePlayerOnline: BroadcastProtocol por login), `switchserversuccess.hpp:40-42` (BroadcastProtocol por switch).
- Failure Scenario: com N jogadores online e M logins/logouts por minuto, cada evento gera O(N) mensagens para todos os GS → picos de CPU/rede no pico de login/logout (manutenção, reinício de zona).
- Impact: degradação de throughput do gdeliveryd em horários de pico.
- Validation: read-only — contar mensagens por evento no tcpdump/estatísticas do GS.
- Fix Direction: broadcast seletivo por interesse (GSs que realmente têm o player), ou batch de anúncios.

---

## Otimizações e melhorias

1. **Granularidade do lock do UserContainer**: o WR lock global serializa login/logout/EnterWorld/Switch. Com os fixes de lock aninhado já aplicados, o próximo passo é reduzir a seção crítica: copiar o `UserInfoBrief`/dados necessários sob RD lock e executar side-effects (sends, managers) fora.
2. **`FindUser(int)`/`FindRole` lock-safe**: mudar a API para retornar cópia ou exigir lock explícito no caller (compilar com a versão com lock `FindUser(int, UserInfo&)` de mapuser.h:520) eliminaria a classe inteira de UAF sem revisar os ~60 callers um a um.
3. **Cópias de Octets**: `RoleLogout` já copia `role->name` (fix pw-web). O mesmo padrão de cópia-defensiva deveria valer para `pinfo->account`, `iseckey`/`oseckey` usados em caminhos assíncronos (ex.: SendDataAndIdentity em playerlogin.cpp:170-177 lê `pinfo->account` e keys sem lock).
4. **Unificar os caminhos de logout**: hoje há 4+ caminhos que fazem FindUser+UserLogout (KickoutUser, GMKickoutUser, ACKickoutUser, StatusAnnounce, RemoteLoggingUsers, ClearRemoteUsers, DisconnectZoneUsers, PlayerLogin_Re). Um único helper `LogoutUser(userid, kicktype)` que toma o WR lock e revalida o estado reduziria os TOCTOUs.
5. **Timer único para os subsistemas sem lock**: os `Update()` de KingElection/StockExchange/WaitQueue/Reference/Reward/etc. rodam no mesmo thread do IntervalTimer; se os handlers de protocolo desses subsistemas fossem enfileirados no mesmo thread (ou cada subsistema ganhasse seu mutex), as corridas desapareceriam sem redesenho.
6. **`GetAuctionID` atômico** (atomic<int> ou lock) é mudança de uma linha com impacto direto na integridade do leilão.
7. **Validação de entrada centralizada**: os checks de faixa de pshopbuy.hpp:66-69 são o bom padrão; aplicá-lo a auctionopen (item_num/baseprice/binprice/elapse_time/deposit) e webtradeprepost (money/price/sellperiod) fecha a categoria "input do cliente".

## Fatos verificados vs inferências

**Verificado (leitura direta do código):**
- `FindUser(int)`/`FindRole`/`FindRoleOnline` (mapuser.h:591,599,607) não tomam lock; dezenas de callers fora de qualquer `GetLocker()` (grep confirmou ~60 ocorrências).
- `RoleLogin` (mapuser.h:659-672) não toma lock; todos os 3 callers (playerlogin.cpp:112,155; waitqueue.cpp:266) estão sem lock.
- `OnPlayerLogout` (map.cpp:346-360) chama `RoleLogout` (que faz `delete role`) sem lock.
- `PlayerLogin_Re::Process` (playerlogin_re.hpp:100-227) não valida roleid/status/gameid contra a sessão corrente.
- `RemoteLoginQuery_Re::Process` (remoteloginquery_re.hpp:19-77) roda FindUser+RealLogin+UserLogout sem lock (diferente do caminho local).
- `StatusAnnounce` (statusannounce.hpp:36-73) refaz FindUser sem lock após liberar o WR lock (residual do fix pw-web de lock aninhado).
- `AuctionMarket::GetAuctionID` (auctionmarket.cpp:348-364) é o único método do AuctionMarket sem `m_lockermap_`.
- `BattleManager::locker` (battlemanager.h:60) é declarado e nunca usado no .cpp.
- KingElection, StockExchange, ReferenceManager, RewardManager, WaitQueueManager, AutoTeamMan, FriendextinfoManager, PlayerProfileMan, PShopMarket, ForceManager, SoloChallengeRank, CountryBattleMan, TankBattleManager, CrossSystem: zero `Thread::Mutex`/`RWLock` (grep nos .h/.cpp).
- `ForbiddenUsers::CheckTimeoutUser` e `RemoteLoggingUsers::CheckTimeoutUser` iteram/erase sem o `locker` próprio.

**Inferências (não verificáveis read-only, precisam de confirmação dinâmica):**
- A frequência real dos UAFs depende do modelo de dispatch do framework GNET (quantos threads do pool processam protocolos concorrentemente). O comentário em enterworld.hpp:52-59 confirma dispatch multi-thread (pool worker PriorPolicy=101 vs poll thread inline), o que valida a premissa de concorrência, mas a probabilidade de colisão é empírica.
- O impacto econômico dos leilões duplicados (GetAuctionID) depende de o gamedbd validar auctionid na inserção; não foi possível confirmar a validação no lado do DB (fora do escopo desta área).
- O TOCTOU do StatusAnnounce é residual do fix pw-web já aplicado; a correção ideal (manter check+logout sob o mesmo WR lock usando EraseUserNoLock) não conflita com o fix, mas não foi aplicada.
- Os limites de validação corretos para auctionopen (faixas de preço/duração) dependem das regras de jogo do gamedbd; os valores exatos precisam ser confirmados com o responsável pela economia.
- Não foi possível confirmar se `ClearRemoteUsers`/`DisconnectZoneUsers` são chamados apenas em contexto single-thread (shutdown); os callers estão fora desta área (gdeliveryserver.hpp / centraldelivery).
