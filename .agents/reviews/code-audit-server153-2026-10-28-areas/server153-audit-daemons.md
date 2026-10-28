# Auditoria de daemons menores — src/server153/cnet/

**Escopo:** gauthd, uniquenamed, gfaction, gacd, logservice (READ-ONLY)
**Data:** 2026-10-28
**Repo:** brasalabs6/pw-web, branch feat/pw-source-1.5.3-ci-build

## Metodologia
- grep/sed dirigido, leitura de código-fonte, sem edição do repositório.
- Prioridade: gauthd (auth), uniquenamed (races), logservice (parsing de rede).

## Resumo
25 findings: 1 Critical, 7 High, 7 Medium, 7 Low, 2 Info, 1 Possible.
Nenhuma edição no repositório. Arquivo: /tmp/server153-audit-daemons.md

## Progresso
- [x] Exploração da estrutura
- [x] gauthd (7 findings)
- [x] uniquenamed (5 findings)
- [x] gfaction (3 findings)
- [x] gacd (4 findings)
- [x] logservice (5 findings)
- [x] Otimizações e fatos verificados

## gauthd — autenticação

### [Critical|Confirmed] Dev-auth: a "senha" é MD5(conta‖conta), sem salt — a conta é a credencial
- Type: security
- Evidence:
  - `gauthd/matrixpasswd.hrp:54-58` — `md5.Update(arg.account); md5.Update(arg.account); md5.Final(digest);` depois `HMAC_MD5Hash(challenge, digest)`.
  - `gauthd/matrixpasswd2.hrp:55-59` — idem; `res.response` = digest cru (MD5(conta‖conta)).
  - `gauthd/gquerypasswd.hrp:39-43` — `md5.Update(arg->account); md5.Update(password)` onde `password = arg->account` (linha 33); o HMAC sobre o challenge está **comentado** (linhas 48-50), então `res.response` é o hash cru, ignorando o challenge.
  - Validação única: `userid % 16 == 0 && userid != 0` (matrixpasswd.hrp:46, matrixpasswd2.hrp:49, gquerypasswd.hrp:29). **Não há consulta a base de contas** — qualquer múltiplo de 16 é aceito.
- Failure Scenario: Um atacante que saiba (ou adivinhe) qualquer número múltiplo de 16 computa `MD5(ascii(conta)‖ascii(conta))` localmente e responde a qualquer challenge HMAC-MD5. Não há segredo algum além do número da conta, que é enumerável (múltiplos de 16). Em `gquerypasswd` o challenge é totalmente ignorado — o hash cru viaja pela rede interna. Não existe lockout nem rate-limit (ver finding seguinte).
- Impact: Qualquer peer que alcance a porta do gauthd (9200) autentica como qualquer conta múltipla de 16. Comprometimento total de conta; o "fator de conhecimento" é nulo.
- Validation: read-only — confirmar que nenhuma consulta a storage/DB existe em matrixpasswd*.hrp/gquerypasswd.hrp (grep `storage\|db\|Storage\|find`); confirmar ausência de salt (grep `salt` no diretório gauthd = 0).
- Fix Direction: Substituir o backend dev por um real: (a) senha armazenada como hash saltado (Argon2/bcrypt ou ao menos HMAC-SHA256 com salt por conta); (b) lookup de existência da conta antes de devolver o hash; (c) o challenge deve participar do MAC em todos os caminhos (restaurar o HMAC em gquerypasswd). Para o dev-auth atual, documentar explicitamente que NÃO há autenticação e isolar a porta.

### [High|Confirmed] Ausência total de rate-limiting / lockout nas RPCs de autenticação
- Type: security
- Evidence: `gauthd/matrixpasswd.hrp`, `matrixpasswd2.hrp`, `matrixtoken.hrp`, `gquerypasswd.hrp`, `userlogin.hrp`, `userlogin2.hrp` — nenhum contador de tentativas, nenhum backoff, nenhum bloqueio por IP/sessão. `gauthserver.hpp` não expõe nenhum mapa de tentativas. `gauthd.conf` não tem limite de taxa.
- Failure Scenario: Um atacante envia milhares de RPC_MATRIXPASSWD2 por segundo com contas múltiplos de 16; o servidor computa MD5+HMAC e responde a todas sem throttling. Combinado com o finding Critical (conta = credencial), qualquer múltiplo de 16 entra instantaneamente; mesmo com um backend real, brute-force online seria irrestrito.
- Impact: Viabilidade de credential stuffing / brute force online sem custo de tempo.
- Validation: grep `attempt\|lockout\|throttle\|rate\|backoff\|fail.*count` em `gauthd/` (0 hits esperado).
- Fix Direction: Contador de falhas por (userid, ip-origem) com janela deslizante e backoff exponencial; bloqueio temporário após N falhas; rejeitar contas inexistentes antes de computar o MAC (evitar oráculo).

### [High|Confirmed] matrixtoken aceita qualquer token para qualquer conta múltipla de 16
- Type: security
- Evidence: `gauthd/matrixtoken.hrp:39-53` — `res.userid = atoi(account)`; checa só `userid % 16`; **o campo `arg.token` nunca é lido nem verificado**; retorna `algorithm=0, retcode=ERR_SUCCESS`. O próprio código marca "SECURITY: with token_login_support=true this accepts any token".
- Failure Scenario: Se `gamesys.conf` (gdeliveryd) tiver `token_login_support=true`, um cliente envia `Response.use_token=1` com um token arbitrário para uma conta múltipla de 16 e é aceito — bypass do fluxo de senha via token.
- Impact: Bypass de autenticação quando o suporte a token está ligado.
- Validation: confirmar `token_login_support` ausente em `gdeliveryd/gamesys.conf` (default false). O risco é latente até alguém ligar a flag.
- Fix Direction: Rejeitar RPC_MATRIXTOKEN (retcode de erro) enquanto não houver backend de token; ou validar o token contra um emissor real.

### [Medium|Confirmed] QueryUserPrivilege concede GM completo a qualquer userid sem validar o solicitante
- Type: security
- Evidence: `gauthd/queryuserprivilege.hpp:18-49` — `Process()` monta `QueryUserPrivilege_Re` com auth = {0..11, 100..105, 200..206} (privilégios máximos) e envia de volta, **sem checar** que a sessão `sid` pertence a um gdeliveryd autorizado nem que `userid` está logado/validado. Não há chamada a `ValidUser`.
- Failure Scenario: Qualquer peer conectado à porta 9200 envia `QueryUserPrivilege` com um userid arbitrário e recebe a lista completa de privilégios GM, que o gdeliveryd aplica em `ProcessUserPrivilege`.
- Impact: Escalação de privilégio para GM por qualquer peer que alcance o gauthd.
- Validation: confirmar ausência de `ValidUser`/`zoneid`/autoriação em `queryuserprivilege.hpp`.
- Fix Direction: Verificar `ValidUser(sid, userid)` antes de responder; restringir privilégios ao que a conta realmente possui (backend real). Para dev, pelo menos checar que `sid` está em `zonemap`.

### [Medium|Confirmed] AccountingRequest: MAC com shared_key fraca + comparação não-constante-tempo
- Type: security
- Evidence:
  - `gauthd/accountingrequest.hpp:20-21` — `VerifyAuth()` = `MD5(stamp‖userid‖attributes.size()‖shared_key) == authenticator`. A comparação usa `Octets::operator==` que é `memcmp` (`PWSL/include/octets.h:50`) — não constante-tempo.
  - `gauthd/gauthd.conf:9` — `shared_key = 123456` (placeholder). O boot script (`scripts/boot-server153.sh:134`) substitui por uma chave aleatória em runtime — mas se o gauthd for lançado sem o boot script, a chave default é trivialmente adivinhável.
  - `GenerateFee` (accountingrequest.hpp:33-72) aceita `_ACCOUNT_START/STOP/ELAPSE` sem validar que `userid` corresponde à sessão `sid` (só `ValidUser` é checado **depois**, linha 82, e só para kick).
- Failure Scenario: Com a shared_key default (123456), um atacante forja `AccountingRequest` com `authenticator` correto e corrompe a contabilização de tempo (zerar elapse, reiniciar start). A comparação `memcmp` permite timing oracle teórico sobre o MAC (praticamente difícil em LAN, mas é falha de higiene cripto).
- Impact: Manipulação de billing/accounting; negação de kickout (se `ValidUser` falha, kicka o usuário legítimo).
- Validation: confirmar `shared_key` default em `gauthd.conf`; confirmar `memcmp` em `octets.h:50`.
- Fix Direction: HMAC-SHA256 ao invés de MD5-MAC; comparação constante-tempo; validar `userid == sessão` antes de processar; nunca commitar chaves default (gerar no boot e 0600).

### [Low|Confirmed] OnDelSession não limpa usermap — entradas stale de sessões mortas vazam
- Type: data-consistency
- Evidence: `gauthd/gauthserver.cpp:20-24` — `OnDelSession` apaga só `zonemap[sid]`; **não** percorre `usermap` para remover entradas cujo `UserInfo.sid == sid`. `StatusAnnounce` (statusannounce.hpp:22-28) limpa `usermap[userid]` só se `it->second.sid == sid`, mas isso exige o cliente enviar o logout — uma queda de conexão não envia.
- Failure Scenario: Sessão A (userid X) cai sem StatusAnnounce. `usermap[X]` aponta para sid morto indefinidamente. O próximo login de X encontra a entrada stale, envia `KickoutUser` para o sid morto (falha silenciosa) e sobrescreve — auto-cura, mas vaza memória e mantém estado inconsistente no intervalo. `accntmap` (accounting) também vaza a entrada START sem STOP correspondente.
- Impact: Leak de memória e estado de accounting inconsistente em quedas anormais; kick inútil para sessão já morta.
- Validation: confirmar que `OnDelSession` não toca `usermap`/`accntmap` (grep em gauthserver.cpp).
- Fix Direction: Em `OnDelSession`, varrer `usermap` (sob `locker_map`) e remover entradas com `sid` correspondente; limpar `accntmap` também.

### [Low|Confirmed] gquerypasswd ignora o challenge e devolve o hash cru
- Type: security
- Evidence: `gauthd/gquerypasswd.hrp:48-50` — o bloco `HMAC_MD5Hash(challenge, digest)` está comentado; `res->response = digest` (MD5 cru). Em matrixpasswd (v1) o HMAC é feito; em matrixpasswd2 o hash cru é enviado intencionalmente (gdeliveryd faz o HMAC). gquerypasswd diverge: devolve cru sem challenge.
- Failure Scenario: O hash MD5(conta‖conta) cru transita pela rede interna sem estar vinculado ao challenge da sessão — reuso do hash em outra sessão/challenge é possível se o consumidor não re-HMAC.
- Impact: Exposição do segredo efetivo (hash) sem binding de sessão; depende de o consumidor aplicar HMAC.
- Validation: comparar as três RPCs (gquerypasswd vs matrixpasswd vs matrixpasswd2) — só gquerypasswd tem o HMAC comentado.
- Fix Direction: Padronizar: ou sempre devolver HMAC(challenge, hash) ou sempre devolver hash cru e garantir que TODO consumidor re-aplica HMAC. Documentar a semântica por RPC.

### [Info|Confirmed] Billing/cash RPCs são stubs (sem validação porque sem implementação)
- Type: maintainability-risk
- Evidence: `addcash.hpp`, `transbuypoint.hpp`, `billingrequest.hpp`, `billingconfirm.hpp`, `billingcancel.hpp`, `billingbalance.hpp`, `billingbalancesa.hpp`, `ssogetticketreq.hpp`, `verifymaster.hpp`, `getaddcashsn.hrp`, `cashserial.hrp` — `Process()` vazio (`// TODO`) ou `Delivery()` retorna `ERR_DELIVER_SEND`.
- Failure Scenario: Funcionalidade de cash/billing/SSO/verifymaster inexistente — qualquer chamada falha silenciosamente ou é ignorada. Não há risco de validação ausente porque não há código que valide; o risco é a ausência.
- Impact: Funcionalidade de billing não operacional (esperado em dev-auth).
- Validation: grep `// TODO` nos arquivos listados.
- Fix Direction: N/A para dev; ao implementar, aplicar validação de sessão + MAC como em accounting.

## uniquenamed — nomes únicos

### [High|Confirmed] Nomes ENGAGED nunca são reclamados — ambos os ramos do duplicate-check retornam ERR_DUPLICATRECORD
- Type: bug / data-consistency
- Evidence:
  - `uniquenamed/precreaterole.hrp:46-59` — `if(!(ENGAGED==status && time>300)) return DUPLICATE; else return DUPLICATE;` — o ramo `else` (ENGAGED com mais de 300s) deveria reclamar o nome stale mas também retorna DUPLICATE.
  - Mesmo padrão em `preplayerrename.hrp:44-58`, `precreatefaction.hrp:55-66`, `precreatefamily.hrp:55-70`.
- Failure Scenario: PreCreateRole/PrePlayerRename/PreCreateFaction insere o nome como `UNIQUENAME_ENGAGED` (reserva). O Post* correspondente (confirmação) nunca chega (gdeliveryd crasha, rede cai, timeout). O nome fica travado como ENGAGED para sempre. Qualquer nova tentativa de criar/renomear para esse nome acha a entrada e retorna DUPLICATE em ambos os ramos — o ramo de reclaim (ENGAGED>300s) é código morto. Com o tempo, nomes vão se esgotando.
- Impact: Nomes de personagem/facção/família permanentemente inutilizáveis após falhas de criação. DoS de namespace por exaustão gradual; também um vetor de "name squat" — um atacante dispara PreCreateRole para nomes desejados e nunca confirma, travando-os.
- Validation: ler os 4 arquivos Pre* e confirmar que o `else` retorna DUPLICATE (não deleta nem prossegue).
- Fix Direction: No ramo `else` (ENGAGED && time>300), apagar a entrada stale e prosseguir com a reserva (re-INSERT como ENGAGED para o novo solicitante). Adicionar limpeza periódica de ENGAGED>300s.

### [Medium|Confirmed] Nomes OBSOLETE (renomeados) nunca são liberados para reuso
- Type: data-consistency
- Evidence:
  - `uniquenamed/postplayerrename.hpp:39-46` — marca `oldname` como `UNIQUENAME_OBSOLETE` (não deleta).
  - `uniquenamed/postfactionrename.hpp:48-54` — idem para facções.
  - `uniquenamed/precreaterole.hrp:46-59` — qualquer entrada encontrada (inclusive OBSOLETE) retorna DUPLICATE; não há ramo que libere OBSOLETE.
  - Contraste: `postdeleterole.hrp:44` — `punamerole->del(rolename)` — só a deleção de personagem remove o nome.
- Failure Scenario: Um jogador renomeia de "Velho" para "Novo". "Velho" fica OBSOLETE. Outro jogador tenta criar "Velho" — PreCreateRole acha a entrada OBSOLETE e retorna DUPLICATE. O nome fica bloqueado indefinidamente mesmo que o personagem original não o use mais.
- Impact: Vazamento gradual de nomes do pool; nomes renomeados nunca voltam à circulação.
- Validation: confirmar ausência de limpeza de OBSOLETE (grep `OBSOLETE` em uniquenamed — só aparece em post*rename, nunca em pre* nem em cleanup).
- Fix Direction: Tratar OBSOLETE como reutilizável no Pre* (deletar e prosseguir) ou ter um coletor periódico que purga OBSOLETE com mais de N dias.

### [Medium|Confirmed] case_insensitive desativado por padrão — nomes case-sensitive; TransformName só cobre ASCII A-Z
- Type: maintainability-risk / security
- Evidence:
  - `uniquenamed/uniquenamed.conf` — não contém `case_insensitive` (ausente); `uniquenamed.cpp:232-233` lê o flag e `atoi("")` = 0 → case-sensitive.
  - `uniquenamed/uniquenameserver.cpp:33-41` — `TransformName` só lowercases `A-Z` em UCS-2 (`*(data+1)==0 && *data>='A' && *data<='Z'`); não cobre maiúsculas acentuadas/Unicode.
  - `scripts/boot-server153.sh` — não injeta `case_insensitive` no conf runtime.
- Failure Scenario: Com case_insensitive=0 (default), "Admin" e "admin" são nomes distintos — um atacante registra "Admin" para se passar por "admin" visualmente. Se alguém ligar case_insensitive=1, `TransformName` só normaliza ASCII; nomes com caracteres CJK/acentuados não são normalizados, permitindo duplicatas visuais (ex.: "Ａdmin" vs "Admin").
- Impact: Impersonação por look-alike de nomes; inconsistência entre o que o jogador vê e o que o DB considera único.
- Validation: confirmar ausência de `case_insensitive` no conf; confirmar o range A-Z em TransformName.
- Fix Direction: Definir case_insensitive explicitamente no conf; estender TransformName para normalização Unicode completa (NFKC casefold) ou ao menos cobrir o charset do jogo.

### [Low|Possible] Race de read-modify-write no rolelist do uidrole depende de isolamento serializable do BDB
- Type: data-consistency
- Evidence: `uniquenamed/precreaterole.hrp:61-91` — `puidrole->find(userid)` (read) → `rolelist.AddRole()` → `puidrole->insert(userid, novo_rolelist)` (write), tudo num `CommonTransaction`. Dois PreCreateRole concorrentes para o mesmo userid fazem find→AddRole→insert; sem re-leitura após bloqueio, o segundo pode sobrescrever o rolelist do primeiro (lost update) se o BDB não re-ler após o upgrade de lock.
- Failure Scenario: Dois gdeliveryd/threads processam criação de role para o mesmo userid simultaneamente. Ambos leem rolelist vazio, ambos AddRole(slot 0), ambos escrevem. O rolelist final tem 1 bit em vez de 2; um logicuid+slot fica órfão (nome ENGAGED mas rolelist não reflete). Depende de o BDB aplicar isolamento serializable (default com DB_INIT_TXN) — se aplicar, o segundo find bloqueia no write-lock do primeiro e re-ler; se não, lost update.
- Impact: Inconsistência de rolelist / slot duplicado para o mesmo userid em concorrência; requer chamador não-serializado por userid.
- Validation: confirmar flags `DB_INIT_TXN|DB_INIT_LOCK` em `storagebdb.h:843` (serializable default); confirmar que gdeliveryd serializa criação por userid.
- Fix Direction: Usar `get` com `DB_RMW` (read-modify-write) no find do uidrole para adquirir write-lock desde a leitura, eliminando a janela; ou serializar PreCreateRole por userid no chamador.

### [Info|Confirmed] DB_TXN_NOSYNC — commits sem fsync, perda de dados confirmados em crash
- Type: operational-risk
- Evidence: `share/storage/storagebdb.h:864` — `env->set_flags(DB_TXN_NOSYNC, 1)`.
- Failure Scenario: Em crash de processo/SO, transações BDB "committed" desde o último checkpoint de log podem ser perdidas (sem fsync). Nomes reservados (ENGAGED) ou confirmados (USED) podem desaparecer, permitindo reuso ou duplicação.
- Impact: Janela de perda de dados em crash; trade-off de performance escolhido pelo upstream.
- Validation: confirmar `DB_TXN_NOSYNC` em storagebdb.h.
- Fix Direction: Avaliar `DB_TXN_WRITE_NOSYNC` (fsync do log sem sync de data) para melhor durabilidade sem custo total; documentar o trade-off.

## gfaction — facções

### [High|Confirmed] factiondb: OnLeave/OnJoin/UpdateUser/UpdateFactionCache modificam dados sob read-lock (RDScoped) — data race
- Type: bug / data-consistency
- Evidence:
  - `gfaction/factiondb.cpp:638` — `OnLeave`: `Thread::RWLock::RDScoped l(locker);` seguido de `pf->info.member.erase(it); pf->version++;` (modifica vetor sob read-lock).
  - `gfaction/factiondb.cpp:660` — `OnJoin`: `RDScoped` + `pf->info.member.add(user); pf->version++;`.
  - `gfaction/factiondb.cpp:676` — `UpdateUser`: `RDScoped` + `it->froleid = user.role; it->nickname.swap(...); pf->version++;`.
  - `gfaction/factiondb.cpp:699` — `UpdateFactionCache`: `RDScoped` + `pf->info.level++; pf->info.announce = arg->announce;`.
  - `gfaction/factiondb.h:100` — `Thread::RWLock locker;` — confirmado RWLock; `RDScoped` = `ReadLock()` (compartilhado), `WRScoped` = `WriteLock()` (exclusivo) (`PWSL/include/mutex.h:55,63,70`).
  - Contraste: `OnLogin` (linha 713) e `OnAllianceApply` (linha 1213) usam `WRScoped` corretamente.
- Failure Scenario: O thread pool do gfaction tem múltiplas threads (`uniquenamed.conf`-style: `(0,1)(1,5)...`). Duas respostas de RPC (AddMember/DelMember) chegam simultaneamente para a mesma facção e disparam OnJoin/OnLeave concorrentes. Ambas seguram o read-lock (compartilhado) e ambas modificam `pf->info.member` (std::vector) — erase + add concorrentes = UB (iterator invalidation, double-free, vetor corrompido). A versão do faction cache fica inconsistente.
- Impact: Corrupção da lista de membros em memória; crash do gfaction; membros fantasma ou perdidos; broadcast de notificações incorreto.
- Validation: confirmar que OnLeave/OnJoin usam RDScoped (linhas 638,660) e que OnLogin usa WRScoped (linha 713) — inconsistência clara.
- Fix Direction: Trocar `RDScoped` por `WRScoped` em OnLeave, OnJoin, UpdateUser, UpdateFactionCache (e quaisquer outras funções que mutam sob read-lock). Auditar todas as 38 ocorrências de RDScoped no factiondb.cpp e garantir que funções que escrevem usem WRScoped.

### [Medium|Possible] Privilege check usa froleid do cache em memória (playermap), não do DB autoritativo — TOCTOU
- Type: security / data-consistency
- Evidence:
  - `gfaction/operations/opexpelmember.inl:13-16` (e todos os `op*.inl`) — `PrivilegeCheck()` faz `GetPlayer(oper_roleid, player)` → usa `player.froleid` do `playermap` em memória do `GFactionServer`.
  - `gfaction/gfactionserver.cpp:49-80` — `RegisterPlayer`/`UpdatePlayer` atualizam `froleid` no playermap quando gdeliveryd anuncia o status ou quando uma operação de faction o atualiza. Não há leitura do DB de faction no momento do privilege check.
  - `gfaction/factionoprequest.hpp:19` — `FactionOPRequest::Process` cria o wrapper com `roleid` do protocolo; `glinkd/factionoprequest.hpp:23` valida `ValidRole(sid,roleid)` (o cliente é dono do roleid) — então o roleid é autêntico, mas o froleid usado para autorização vem do cache.
- Failure Scenario: Um líder é rebaixado a membro por outro líder (operação Appoint). O `UpdatePlayer` propaga o novo froleid ao playermap, mas há uma janela entre o rebaixamento no DB (gamedbd) e a chegada do `UpdatePlayer` ao gfaction. Nessa janela, o jogador rebaixado ainda tem `_R_MASTER` no cache e pode executar operações de líder (expel, dismiss, alliance). A janela é curta mas realista sob latência de rede entre gamedbd→gfaction.
- Impact: Escalonamento temporária de privilégio após rebaixamento; operação de facção indevida em janela de race.
- Validation: confirmar que PrivilegeCheck lê só do playermap (grep `GetPlayer` em op*.inl); confirmar que UpdatePlayer é assíncrono (via RPC/announce).
- Fix Direction: Para operações de alto privilégio (dismiss, alliance, masterresign), re-validar o froleid contra o factiondb (DB) no momento do Execute(), não só contra o cache. Ou usar um lock que sincroniza o cache com o DB.

### [Low|Confirmed] CreateFactionFortress despacha sem validar posse/permissão da facção
- Type: security
- Evidence: `gfaction/createfactionfortress.hpp:20-26` — `Process()` chama `FindFactionInCache(info)` (resultado ignorado), checa só `factionid > 0`, e `DispatchProtocol(0, this)` para o gameserver. Sem PrivilegeCheck, sem verificar que a facção existe ou que o solicitante tem permissão.
- Failure Scenario: Um peer envia CreateFactionFortress com `factionid` arbitrário > 0; o gfaction repassa ao gameserver sem validar que a facção existe ou que o solicitante tem permissão.
- Impact: Criação de fortaleza para facção inexistente/não autorizada (depende do gameserver validar downstream).
- Validation: confirmar ausência de privilege check e de validação de existência em createfactionfortress.hpp.
- Fix Direction: Validar `FindFactionInCache` retorna true e que a sessão tem autoridade sobre a facção antes de despachar.

## gacd — anti-cheat

### [High|Confirmed] ACReloadConfig: parser XML sem NULL-check em xmlDocGetRootElement — crash (DoS) com XML malformado
- Type: security / bug
- Evidence:
  - `gacd/acreloadconfig.hpp:21-22` — `ConfigManager::Reload((const char*)config.begin(), config.size())` onde `config` é Octets do protocolo de rede.
  - `gacd/configmanager.cpp:43-44` — `m_config.Reload(szBuffer, uSize)` → `gacd/xmlconfig.cpp:106-108` — `xmlDoc *pDoc = xmlParseMemory(szBuffer, uSize); m_pRoot = Build(pDoc, xmlDocGetRootElement(pDoc));` — **sem verificar** se `pDoc` é NULL ou se `xmlDocGetRootElement` retorna NULL.
  - `gacd/xmlconfig.cpp:76-77` — `Build(pDoc, pXmlNode)` faz `pNode->m_name = std::string((char*)pXmlNode->name)` — deref de `pXmlNode` sem NULL-check.
- Failure Scenario: Um peer envia ACReloadConfig com XML vazio/malformado (ex.: `""` ou `<`). `xmlParseMemory` retorna NULL ou doc sem root. `xmlDocGetRootElement` retorna NULL. `Build(pDoc, NULL)` deref `NULL->name` → SIGSEGV. O gacd cai. Como o gacd é singleton e sem restart automático (exceto watchdog), toda a proteção anti-cheat cai.
- Impact: DoS do anti-cheat por crash; toda a pipeline de detecção para.
- Validation: confirmar ausência de NULL-check entre `xmlParseMemory` e `Build` (xmlconfig.cpp:106-108); enviar XML vazio em ambiente de teste e observar crash.
- Fix Direction: Após `xmlParseMemory`, checar `pDoc != NULL`; após `xmlDocGetRootElement`, checar `!= NULL`; retornar erro se inválido. Adicionar `XML_PARSE_NOENT | XML_PARSE_NOERROR` flags apropriadas e limitar tamanho do buffer.

### [High|Confirmed] ACReloadConfig sem autenticação — qualquer peer do control-port desativa toda a punição anti-cheat
- Type: security
- Evidence:
  - `gacd/acreloadconfig.hpp:20` — `Process` não valida a sessão `sid` nem autentica o conteúdo.
  - `gacd/gaccontrolserver.cpp:55-62` — `SetControlSID` aceita a primeira conexão como control-client; **sem auth**, só rejeita se já há um control_sid.
  - `gacd/punishmanager.cpp:71` — `OnUpdateConfig` chama `ClearRules()` ANTES de ler o novo config: se o XML novo não tiver `<kickrule>`, `m_bindMap` fica vazio e `FindBindRules` (punishmanager.hpp:86-106) retorna false → **nenhuma punição é aplicada**.
  - `gacd/gamesys.conf:39-42` — `[GACControlServer] port=29712 address=127.0.0.1`; `isec=6/osec=5` (compressão MPPC, **não criptografia**).
- Failure Scenario: Um processo local (ou gacdclient comprometido) conecta à 29712, envia ACReloadConfig com `<punishmanager/>` vazio. ClearRules apaga todas as regras existentes; nenhuma regra nova é carregada. A partir desse momento, `DeliverCheater` não encontra bind rules e NENHUM cheat é punido. O anti-cheat fica completamente neutralizado até reiniciar o gacd.
- Impact: Bypass total do sistema de punição; cheaters não são kickados/banidos.
- Validation: confirmar que ClearRules precede a leitura (punishmanager.cpp:71); confirmar ausência de auth em acreloadconfig.hpp e gaccontrolserver.cpp.
- Fix Direction: Autenticar ACReloadConfig (token/HMAC compartilhado entre gacd e gacdclient); NÃO limpar regras antes de validar que o novo config é completo e válido (atomic swap: parse em temp, só ClearRules+apply se parse OK); restringir o control-port a um socket de domínio Unix ou firewall.

### [Medium|Confirmed] ACRemoteExe: injeção de código executável no cliente sem autenticação
- Type: security
- Evidence: `gacd/acremoteexe.hpp:55-79` — `REMOTEEXE_MAKE` pega `exe` (Octets da rede) e envia como code-pieces ao cliente via `GAntiCheaterServer::SendProtocol`. `REMOTEEXE_RUN` monta código "hammer" e injeta `file_size`. Sem validação de `roleid` (qualquer roleid alvo) nem auth da sessão.
- Failure Scenario: Um peer no control-port envia ACRemoteExe com `ptype=REMOTEEXE_MAKE`, `roleid=<vítima>`, `exe=<payload>`. O gacd fragmenta e envia ao gacdclient do alvo, que executa o código injetado no processo do cliente.
- Impact: Execução de código arbitrário no cliente de jogo (RCE) via o canal anti-cheat; depende de alcançar o control-port (localhost).
- Validation: confirmar que ACRemoteExe::Process não valida sid nem roleid (acremoteexe.hpp:20-79).
- Fix Direction: Autenticar ACRemoteExe; restringir a operadores autorizados (GM); assinar o código injetado com chave compartilhada.

### [Low|Confirmed] xmlMemoryDump() chamado após todo parse — debug em produção
- Type: operational-risk / maintainability-risk
- Evidence: `gacd/xmlconfig.cpp:111` e `:128` — `xmlMemoryDump();` após cada `xmlReadFile`/`xmlParseMemory`.
- Failure Scenario: A cada reload de config, libxml2 despeja o estado de alocação de memória em stdout/arquivo — ruído de log, leve custo de performance, e potencial leak de informação de heap em produção.
- Impact: Menor; ruído operacional.
- Validation: grep `xmlMemoryDump` em xmlconfig.cpp (2 ocorrências).
- Fix Direction: Remover `xmlMemoryDump()` (e `xmlCleanupParser()` pode permanecer); compilar libxml2 sem debug em produção.

## logservice — coletor de logs

### [High|Confirmed] Sem autenticação em UDP 11100 / TCP 11101 — injeção e forja de logs
- Type: security
- Evidence:
  - `logservice/logservice.conf:1-20` — `[LogserviceServer] type=udp port=11100` e `[LogserviceTcpServer] type=tcp port=11101` — **nenhum** `isec`/`osec`/`iseckey`/`oseckey` (sem criptografia nem MAC).
  - `logservice/remotelog.hpp:22-24` — `Process` chama `LogDispatch::log(priority, msg, hostname, servicename)` diretamente dos campos do protocolo, **sem validar** `sid`, origem, `priority`, `hostname` ou `servicename`. Mesmo em `remotelogvital.hpp:20-22` e `statinfo.hpp:20-22`/`statinfovital.hpp`.
  - `logservice/logdispatch.h:149-163` — `priority == LOG_CHAT` escreve no `fd_chat`; `LOG_CASH` no `fd_cash`; `priority` controlado pelo remetente.
- Failure Scenario: Qualquer processo local (ou atacante remoto se a porta for exposta) envia um pacote UDP RemoteLog com `hostname="gdeliveryd"`, `servicename="GAuthServer"`, `priority=LOG_CASH`, `msg="addcash userid=16 amount=999999"`. O logservice grava a entrada no `world2.cash` (trilha de auditoria financeira) sem questionar. Com `priority=LOG_EMERG` e newlines no `msg`, forja entradas de log que parecem vir de qualquer servidor, criando trilhas de auditoria falsas ou ocultando ataques reais.
- Impact: Forja de logs/auditoria; poluição do log de cash; DoS por enchimento de disco (sem rate-limit); negação de confiança na trilha de logs.
- Validation: confirmar ausência de isec/osec no conf; confirmar que RemoteLog::Process não valida sid nem campos (remotelog.hpp:22-24).
- Fix Direction: Adicionar HMAC/MAC compartilhado entre logservice e clientes (logclient); rejeitar pacotes sem MAC válido; sanitizar `msg` (remover/escapar newlines e caracteres de controle); restringir `priority` a um conjunto permitido por origem; rate-limit por origem.

### [Medium|Confirmed] Sem rotação automática de logs — arquivos crescem indefinidamente
- Type: operational-risk
- Evidence:
  - `logservice/logdispatch.h:28` — `flags = O_WRONLY|O_CREAT|O_APPEND|O_NONBLOCK` — abre em append, sem truncar.
  - `logservice/logservice.cpp:48-57` — `LogPolicy::OnSIGHUP` faz `closefiles()` + `openfiles()` — reabre os mesmos arquivos (não renomeia nem trunca). Rotação depende de ferramenta externa (logrotate) renomear antes do SIGHUP.
  - `logservice/logservice.conf:28-37` — caminhos `./logs/world2.*` — sem política de tamanho/rotação.
- Failure Scenario: Sem logrotate configurado, os arquivos de log crescem até encher o disco. O `write()` com `O_NONBLOCK` falha silenciosamente quando o disco cheio (return value ignorado, logdispatch.h:81). O logservice não alerta nem para.
- Impact: Disk full → queda de todos os daemons que escrevem logs; perda silenciosa de logs.
- Validation: confirmar que OnSIGHUP só reabre (sem rename/truncate); confirmar ausência de logrotate config no repo.
- Fix Direction: Integrar rotação por tamanho (reabrir+rename quando > N MB) ou documentar a dependência de logrotate + SIGHUP; checar retorno de `write()` e logar erro de disco cheio.

### [Low|Confirmed] writefile ignora retorno de write() — short writes em O_NONBLOCK truncam logs
- Type: data-consistency
- Evidence: `logservice/logdispatch.h:81` — `return write( fd, buffer, pos );` — o retorno é ignorado pelo caller (`log()` não checa). Com `O_NONBLOCK`, `write()` pode retornar menos que `pos` (short write) ou -1 (errno=EAGAIN se buffer de socket cheio — mas é arquivo, então EAGAIN é improvante; em arquivo regular, O_NONBLOCK é geralmente ignorado).
- Failure Scenario: Em disco cheio ou NFS, `write()` retorna -1 ou parcial; a entrada de log é truncada ou perdida silenciosamente.
- Impact: Perda silenciosa de entradas de log críticas (cash, audit).
- Validation: confirmar que `log()` não checa retorno de `writefile()` (logdispatch.h:149-183).
- Fix Direction: Checar retorno de `write()` e logar falha; remover `O_NONBLOCK` para arquivos de log locais (não há benefício em nonblock para append em arquivo regular).

### [Low|Confirmed] msg não sanitizado — newlines permitem forja de entradas multi-linha
- Type: security
- Evidence: `logservice/logdispatch.h:75` — `strncpy(buffer+pos, msg.c_str(), ...)` — `msg` é escrito sem escapar `\n`, `\r`, ou caracteres de controle. `msg` vem diretamente do protocolo de rede (`remotelog.hpp:22`).
- Failure Scenario: Atacante envia `msg = "linha legítima\n[2026-10-28 12:00:00 gdeliveryd GAuthServer: INFO : login falsificado"`. O log grava duas linhas: a legítima e a forjada, sem distinção visual.
- Impact: Log forging — entradas falsas indistinguíveis das reais; compromete a confiabilidade da trilha de auditoria.
- Validation: confirmar ausência de sanitização de `msg` em writefile (logdispatch.h:75).
- Fix Direction: Substituir `\n`/`\r` em `msg` por espaços ou sequências de escape antes de escrever; prefixar cada linha com timestamp/origem.

### [Low|Confirmed] Falha de open() silenciosa — se diretório não existe, todos os logs são perdidos
- Type: operational-risk
- Evidence: `logservice/logdispatch.h:118-128` — `open(path, flags, mode)` sem checar retorno; se retorna -1, `fd_*` fica -1 e `writefile` escreve em fd=-1 (falha silenciosa). O boot script cria `logs/` (boot-server153.sh:171), mas se o CWD mudar ou o dir for apagado, os logs somem sem alerta.
- Failure Scenario: O diretório `./logs/` não existe quando o logservice inicia (CWD errado, dir não criado). Todos os `open()` falham. O logservice roda sem gravar nada; ninguém percebe até precisar dos logs.
- Impact: Perda total de logs sem diagnóstico.
- Validation: confirmar que openfiles não checa retorno de open (logdispatch.h:118-128).
- Fix Direction: Checar retorno de cada `open()` e `Log::log(LOG_ERR, ...)` se falhar; abortar se arquivos críticos não abrem.

## Otimizações e melhorias

### I/O síncrono no thread pool
Todos os 5 daemons processam RPCs em um thread pool fixo, e cada handler executa I/O síncrono (bloqueante) dentro da thread:
- **uniquenamed/gfaction**: operações BDB (`find`/`insert`/`del`) são síncronas e podem bloquear em locks de DB. O thread pool do uniquenamed tem 8 threads `(0,1)(1,5)(100,1)(101,1)`; o do gfaction tem 7 `(0,1)(1,4)(100,1)(101,1)`. Sob carga, se todas as threads bloquearem em BDB locks, o daemon para de processar novas RPCs até que uma thread se libere. Não há I/O assíncrono nem filas de prioridade para RPCs críticas.
- **gacd**: `xmlParseMemory` (parse XML síncrono, potencialmente lento para XML grande) roda na thread do RPC handler. O thread pool tem 12 threads `(1,9)(100,1)(101,1)(0,1)`, mas um XML de 2MB pode bloquear uma thread por segundos.
- **logservice**: `write()` síncrono para 9 arquivos em cada `RemoteLog::Process`. Com `O_NONBLOCK` em arquivo regular, o kernel geralmente ignora nonblock, mas o `write()` ainda é síncrono. Sob alto volume de logs, as 6 threads `(0,1)(1,5)` podem saturar.
- **Melhoria**: Para logservice, usar buffer em memória + flush periódico (batch write) em vez de write-per-message. Para uniquenamed/gfaction, considerar `DB_TXN_NOWAIT` ou timeout menor para evitar threads presas em deadlock de DB.

### Logs sem rotação automática
- **logservice**: O único mecanismo de rotação é SIGHUP (closefiles + openfiles), que reabre os mesmos arquivos sem renomear/truncar. Rotação real exige logrotate externo + SIGHUP. Sem isso, os 9 arquivos de log crescem indefinidamente.
- **gacd**: `xmlMemoryDump()` após cada parse de config despeja estado de memória libxml2 — ruído em produção.
- **uniquenamed/gfaction/gauthd**: Logs via `LOG_TRACE`/`DEBUG_PRINT` vão para stderr/syslog; sem rotação configurada no daemon.
- **Melhoria**: Integrar rotação por tamanho no logservice (reabrir+rename quando > N MB); remover `xmlMemoryDump()` do gacd.

### Durabilidade do BDB
- `DB_TXN_NOSYNC` (`storagebdb.h:864`) em uniquenamed e gfaction: commits sem fsync. Em crash, transações committed desde o último checkpoint de log podem ser perdidas. É um trade-off de performance do upstream.
- **Melhoria**: Avaliar `DB_TXN_WRITE_NOSYNC` (fsync do log de transações sem sync de data pages) para melhor durabilidade sem custo total.

### Criptografia e comparação
- **gauthd**: MD5 para hash de senha e MAC de accounting. MD5 é considerado quebrado para colisões; para MAC, HMAC-MD5 ainda é aceitável mas SHA-256 é o padrão moderno. A comparação de MAC em `accountingrequest.hpp:21` usa `memcmp` (não constante-tempo).
- **Melhoria**: Migrar para HMAC-SHA256; usar comparação constante-tempo (`CRYPTO_memcmp` ou equivalente).

### Thread pool e prioridade
- **gfaction/gacd**: `prior_strict = 0` — tarefas de baixa prioridade podem competir com as de alta. Em gfaction, uma operação de faction de baixa prioridade (ex.: listmember) pode atrasar uma crítica (ex.: createfaction). Em gauthd/uniquenamed, `prior_strict = 1` (correto).
- **Melhoria**: Avaliar `prior_strict = 1` em gfaction para garantir que RPCs de alta prioridade não sejam adiadas.

### Logicuid allocation
- **uniquenamed**: `LogicuidManager` usa um `busy` flag e `idset` em memória. O `LogicuidSeeker` roda a cada 5s (HouseKeeper) e sob demanda (quando idset ≤ 128). O scan do DB é síncrono e pode ser lento se o DB for grande. O `busy` flag impede seekers concorrentes, mas se o seeker travar, o idset pode drenar e `AllocLogicuid` retorna 0 (falha temporária de criação de role).
- **Melhoria**: Pré-alocar um pool maior; usar um thread dedicado para refill; persistir o `startid` com maior frequência.

## Fatos verificados vs inferências

### Verificados (lidos diretamente no código)
- gauthd dev-auth: `MD5(account‖account)`, sem salt, sem consulta a DB, validação só `userid % 16` (matrixpasswd.hrp:46-58, matrixpasswd2.hrp:49-59, gquerypasswd.hrp:29-43). **Confirmado.**
- gauthd sem rate-limit/lockout: nenhum contador de tentativas em nenhum handler de auth. **Confirmado por ausência.**
- matrixtoken não verifica o token (matrixtoken.hrp:39-53). **Confirmado.**
- QueryUserPrivilege concede GM completo sem validar solicitante (queryuserprivilege.hpp:18-49). **Confirmado.**
- AccountingRequest usa `memcmp` via `Octets::operator==` (octets.h:50) e `shared_key` default "123456" no conf template (gauthd.conf:9). **Confirmado.** Boot script substitui em runtime (boot-server153.sh:134).
- OnDelSession não limpa usermap (gauthserver.cpp:20-24). **Confirmado.**
- gquerypasswd tem HMAC comentado, devolve hash cru (gquerypasswd.hrp:48-50). **Confirmado.**
- uniquenamed: ambos os ramos do duplicate-check retornam ERR_DUPLICATRECORD em precreaterole/preplayerrename/precreatefaction/precreatefamily. **Confirmado.**
- uniquenamed: OBSOLETE nunca é liberado (só postdeleterole faz `del`). **Confirmado.**
- uniquenamed: case_insensitive ausente do conf (default 0 = case-sensitive); TransformName só cobre ASCII A-Z. **Confirmado.**
- BDB env: `DB_INIT_TXN|DB_INIT_LOCK` + `DB_TXN_NOSYNC` (storagebdb.h:843-864). **Confirmado.**
- gfaction: OnLeave/OnJoin/UpdateUser/UpdateFactionCache usam `RDScoped` (read-lock) enquanto modificam o member vector (factiondb.cpp:638,660,676,699). `locker` é `Thread::RWLock` (factiondb.h:100). **Confirmado.**
- gfaction: privilege check usa froleid do playermap em memória (op*.inl). glinkd valida `ValidRole(sid,roleid)` (glinkd/factionoprequest.hpp:23). **Confirmado.**
- gacd: ACReloadConfig chama `xmlParseMemory` sem NULL-check em `pDoc`/root (xmlconfig.cpp:106-108, acreloadconfig.hpp:21). **Confirmado.**
- gacd: PunishManager::OnUpdateConfig chama ClearRules antes de ler novo config (punishmanager.cpp:71). **Confirmado.**
- gacd: GACControlServer aceita primeira conexão sem auth (gaccontrolserver.cpp:55-62). **Confirmado.**
- gacd: ACRemoteExe envia `exe` da rede como code-pieces ao cliente sem validar roleid/sid (acremoteexe.hpp:55-79). **Confirmado.**
- logservice: sem isec/osec no conf (logservice.conf); RemoteLog/StatInfo passam campos da rede direto ao LogDispatch sem validação (remotelog.hpp:22-24). **Confirmado.**
- logservice: sem rotação automática; SIGHUP só reabre (logservice.cpp:48-57, logdispatch.h:118-128). **Confirmado.**
- logservice: writefile ignora retorno de write() (logdispatch.h:81). **Confirmado.**
- logservice: msg não sanitizado (logdispatch.h:75). **Confirmado.**

### Inferências (não verificadas por execução, baseadas em leitura de código)
- A corrida de read-modify-write no rolelist do uniquenamed (PreCreateRole) dependeria de dois RPCs concorrentes para o mesmo userid; **assume-se** que o BDB serializable isola isso, mas não foi testado. A possibilidade de lost-update existe se o BDB não re-ler após upgrade de lock. **Inferência.**
- A janela de TOCTOU no privilege check do gfaction (froleid stale após rebaixamento) **depende da latência** entre gamedbd→gfaction na propagação do UpdatePlayer. Não foi medido o tempo real da janela. **Inferência.**
- O impacto do log forging no logservice assume que os logs são usados para auditoria financeira; **não foi verificado** se o `world2.cash` é efetivamente usado como trilha de auditoria em produção. **Inferência.**
- O crash do gacd por XML malformado (NULL deref) **assume** que `xmlDocGetRootElement` retorna NULL para XML vazio/malformado — comportamento padrão do libxml2, mas não testado com a versão exata do `libxml2.so.2` shipped. **Inferência de comportamento de API.**
- A exposição dos daemons (localhost-only vs 0.0.0.0) **depende do conf runtime** gerado pelo boot script; o conf template tem 0.0.0.0 em alguns casos (gacd io.conf) mas o boot script normaliza para 127.0.0.1. Não foi verificado o conf final em runtime. **Inferência.**

### Não verificado (fora do escopo ou sem dados)
- Não foi executado nenhum daemon nem enviado pacotes de teste (auditoria read-only).
- Não foi verificada a versão exata do `libxml2.so.2` shipped no gacd quanto a CVEs de entity expansion.
- Não foi verificado o comportamento do `Protocol::Manager` quanto a limites de sessão/conexão concorrentes.
- Não foi auditado o glinkd (fora da área designada) além da checagem de `ValidRole` para FactionOPRequest.
- Não foi verificada a configuração de firewall/iptables do host.
