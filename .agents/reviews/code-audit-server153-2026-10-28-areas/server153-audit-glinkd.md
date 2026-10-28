# AUDITORIA READ-ONLY — glinkd (src/server153/cnet/glinkd/)

**Repo:** brasalabs6/pw-web, branch `feat/pw-source-1.5.3-ci-build`
**Data:** 2026-10-28
**Escopo:** `src/server153/cnet/glinkd/` — glinkd.cpp, glinkserver.hpp/cpp, state.hxx/cxx, stubs.cxx, ~400 protocolos .hpp/.hrp, .inl em `src/server153/cnet/inl/`
**Método:** grep/sed dirigido, leitura de código fonte. Nenhuma edição no repositório.

---

## Findings

### [HIGH|CONFIRMED] DeleteRole e UndoDeleteRole não validam userid da sessão antes de encaminhar ao delivery

- Type: security
- Evidence: `deleterole.hpp:22-28` — `Process` apenas faz `this->localsid=sid;` e encaminha ao `GDeliveryClient`. Não chama `ValidUser(sid, userid)` nem `ValidRole(sid, roleid)`. `undodeleterole.hpp:22-28` — idêntico. Comparar com `createrole.hpp:23` que faz `if (!GLinkServer::GetInstance()->ValidUser(sid,userid)) return;` e `rolelist.hpp:21-25` que também valida. O campo `roleid` (e potencialmente `userid` se existisse no payload) é controlado pelo cliente.
- Failure Scenario: Um cliente autenticado como user A envia DeleteRole com `roleid` pertencente ao user B. O glinkd encaminha ao delivery sem validar. Se o delivery não validar a posse do role (ou houver uma race condition), o role é deletado.
- Impact: Perda de dados de personagem. Violação de isolamento entre contas.
- Validation: Confirmar lendo `deleterole.hpp` e `undodeleterole.hpp` — ausência de qualquer `ValidUser`/`ValidRole`. Verificar se `gdeliveryd` valida posse do roleid antes de deletar (fora do escopo do glinkd).
- Fix Direction: Adicionar `if (!GLinkServer::GetInstance()->ValidUser(sid, userid)) return;` no início de `DeleteRole::Process` e `UndoDeleteRole::Process`, seguindo o padrão de `CreateRole`. Se o payload não contém `userid`, usar `sinfo->userid` da sessão.

---

### [HIGH|CONFIRMED] Ausência de limite por IP e rate limiting no login — halfloginset DoS

- Type: operational-risk
- Evidence: `glinkserver.hpp:150-157` — `user_count_limit` e `halflogin_limit` são globais, não por-IP. `response.hpp:59-63` — cada `Response` insere `sid` em `halfloginset`. `keyreestablish.hpp:24-28` — `KeyReestablish` também insere em `halfloginset`. Ambos aceitos em `state_GLoginServer` (pré-autenticação). Quando `halfloginset.size() >= halflogin_limit + 15`, `StopListen()` fecha o socket de escuta (`response.hpp:61-63`, `keyreestablish.hpp:26-28`). O `halfloginset` só é limpo em `OnDelSession` (`glinkserver.cpp:99`). O timeout de `state_GResponseReceive` é 60s (`state.cxx:33`).
- Failure Scenario: Atacante abre 45+ conexões simultâneas (halflogin_limit default=30 + threshold=15), envia `Response` em cada uma. Cada sid é inserido em `halfloginset`. `StopListen()` é chamado. Novos jogadores legítimos não conseguem conectar por até 60s (até as sessões do atacante expirarem por timeout). O atacante pode renovar continuamente.
- Impact: DoS no login — nenhum jogador novo consegue entrar no servidor.
- Validation: Confirmar que não há `OnCheckAddress` com filtragem por IP em `gdeliveryclient.cpp:65` (vazio) nem em glinkserver. Confirmar que `halfloginset` é global.
- Fix Direction: Implementar limite de conexões simultâneas por IP (ex.: mapa IP→count com cap configurável). Adicionar rate limit por IP no estado `state_GLoginServer` antes de processar `Response`/`KeyReestablish`. Reduzir timeout de `state_GResponseReceive` ou adicionar limpeza proativa de half-login expirados.

---

### [MEDIUM|CONFIRMED] SelectRole não valida que roleid pertence à sessão

- Type: security
- Evidence: `selectrole.hpp:23-35` — `Process` constrói `PlayerLogin` com `roleid` do payload (controlado pelo cliente) e envia ao delivery sem validar `ValidRole(sid, roleid)` ou `ValidUser(sid, userid)`. Comparar com `enterworld.hpp:25-34` que valida `roleinfomap.find(roleid)` e checa `sid`/`roleid`.
- Failure Scenario: Cliente autenticado envia `SelectRole` com `roleid` de outro jogador. Se o delivery não validar a posse, o cliente pode tentar entrar no mundo como outro personagem.
- Impact: Potencial bypass de isolamento de personagem. Depende da validação no delivery/gameserver.
- Validation: Ler `selectrole.hpp` — nenhuma chamada a `ValidRole`/`ValidUser`. Verificar se `gdeliveryd` valida posse do role antes de `PlayerLogin`.
- Fix Direction: Adicionar `ValidRole(sid, roleid)` antes de enviar `PlayerLogin`, ou validar que `roleid` está na lista de roles retornada por `RoleList_Re`.

---

### [MEDIUM|CONFIRMED] CollectClientMachineInfo encaminha ao delivery sem nenhuma validação

- Type: security
- Evidence: `collectclientmachineinfo.hpp:20-22` — `Process` apenas faz `GDeliveryClient::GetInstance()->SendProtocol(this);`. Sem `ValidUser`, `ValidRole`, ou qualquer checagem. O `userid` é um campo do payload (controlado pelo cliente). SizePolicy = 2048 bytes (`inl/collectclientmachineinfo`).
- Failure Scenario: Cliente envia `CollectClientMachineInfo` com `userid` arbitrário e até 2KB de `machineinfo` arbitrária. O delivery processa sem questionar a origem.
- Impact: Injeção de dados de máquina falsos para qualquer userid. Pode poluir logs anti-cheat ou causar ações incorretas no delivery.
- Validation: Confirmar ausência de validação em `collectclientmachineinfo.hpp:20-22`.
- Fix Direction: Adicionar `ValidUser(sid, userid)` e usar `sinfo->userid` da sessão em vez do `userid` do payload.

---

### [MEDIUM|CONFIRMED] SetCustomData encaminha ao delivery sem validar roleid

- Type: security
- Evidence: `setcustomdata.hpp:20-23` — `Process` apenas faz `this->localsid=sid; GDeliveryClient::GetInstance()->SendProtocol(this);`. Comentário diz "check localsid by delivery" mas o `roleid` é controlado pelo cliente. Comparar com `getcustomdata.hpp:22-27` que faz `ValidRole(sid, roleid)`. SizePolicy = 1536 bytes.
- Failure Scenario: Cliente autenticado envia `SetCustomData` com `roleid` de outro jogador. Se o delivery não validar, os custom data do role alvo são sobrescritos.
- Impact: Corrupção de dados de personagem alheio.
- Validation: Comparar `setcustomdata.hpp` vs `getcustomdata.hpp` — o segundo valida, o primeiro não.
- Fix Direction: Adicionar `if (!GLinkServer::ValidRole(sid, roleid)) { SessionError(...); return; }` antes de encaminhar.

---

### [MEDIUM|CONFIRMED] Trace fprintf por pacote em Protocol::Decode — performance e info leak

- Type: performance-risk
- Evidence: `protocol.h:538-547` — Bloco marcado `// pw-web debug` que executa `fprintf(stderr, "[decode] type=%d size=%d bytes:", ...)` seguido de loop até 64 bytes hex para CADA protocolo decodificado. Este código roda no thread de I/O (PollIO) de forma síncrona.
- Failure Scenario: Sob carga normal (centenas de pacotes/segundo), o `fprintf` para stderr bloqueia o thread de I/O. Sob carga alta, torna-se gargalo significativo. Adicionalmente, o conteúdo raw dos pacotes (incluindo dados de login, chat, etc.) é despejado em stderr.
- Impact: Degradação de performance sob carga. Vazamento de dados de pacotes em logs/stderr.
- Validation: Ler `protocol.h:538-547` — o bloco não tem `#ifdef DEBUG` nem guard de log level.
- Fix Direction: Remover o bloco de debug ou envolvê-lo em `#ifdef PW_WEB_DEBUG`. Usar `Log::log(LOG_DEBUG, ...)` em vez de `fprintf(stderr, ...)`.

---

### [MEDIUM|LIKELY] Challenge nonce parcialmente previsível — apenas 8 de 16 bytes aleatórios

- Type: security
- Evidence: `glinkserver.cpp:40-44` — `challenge.Setup(16)` preenche 16 bytes com dados aleatórios (via `Security::Create(RANDOM)`), mas em seguida sobrescreve os primeiros 8 bytes: `*(unsigned int*)challenge.nonce.begin()=GetServerAttr();` e `*((int*)challenge.nonce.begin()+1)=GetFreeCreatime();`. Ambos são constantes do servidor (configurados em runtime, mas iguais para todas as sessões).
- Failure Scenario: O nonce do challenge tem apenas 8 bytes de entropia (bytes 8-15). Se `server_attr` e `freecreatime` são conhecidos ou constantes (0 por default no construtor, `glinkserver.hpp:138-139`), um atacante pode prever o nonce e pré-computar a resposta do challenge-response.
- Impact: Redução da entropia do nonce de autenticação. Facilita ataques de replay ou pre-computation se o algoritmo de hash for fraco.
- Validation: Confirmar que `server_attr` e `freecreatime` default para 0 (`glinkserver.hpp:138-139`). Verificar se são configurados a partir do conf ou do delivery.
- Fix Direction: Não sobrescrever os bytes aleatórios do nonce. Se `server_attr` e `freecreatime` precisam ser comunicados ao cliente, usar campos separados do protocolo Challenge (que já tem `version`, `algo`, `edition`, `exp_rate`).

---

### [MEDIUM|CONFIRMED] state_GSelectRoleServer com timeout de 86400s (24h) permite sessões-zumbi

- Type: operational-risk
- Evidence: `state.cxx:133` — `state_GSelectRoleServer` tem timeout 86400 (24 horas). O alivetimemap/keepalive só é configurado após `EnterWorld` (`enterworld.hpp:47`: `lsm->SetAliveTime(sid, _CLIENT_TTL)`). Antes disso, a sessão depende apenas do timeout do estado.
- Failure Scenario: Um cliente atinge o estado de seleção de role e fica parado. A sessão persiste por 24 horas consumindo um slot em `sessions` e potencialmente em `halfloginset`. Combinado com a ausência de limite por IP, um atacante pode abrir muitas conexões, completar o login até a seleção de role, e manter todas ativas por 24h.
- Impact: Exaustão de recursos (sessões, memória, slots). DoS prolongado.
- Validation: Confirmar `state.cxx:133` — timeout 86400. Confirmar que `SetAliveTime` só é chamado em `enterworld.hpp:47` e `gamedatasend.hpp:22` (pós-jogo).
- Fix Direction: Reduzir o timeout de `state_GSelectRoleServer` para um valor razoável (ex.: 300s). Adicionar keepalive no pré-jogo ou usar o alivetimemap desde o login.

---

### [MEDIUM|LIKELY] OnDelSession early-return deixa roleinfomap sem limpar e não envia StatusAnnounce offline

- Type: data-consistency
- Evidence: `glinkserver.cpp:107-114` — Se `ui.sid != sid` (o sid registrado no roleinfomap difere do sid da sessão que está fechando), a função faz `return` sem: (1) apagar a entrada de `roleinfomap`, (2) enviar `StatusAnnounce(userid, sid, _STATUS_OFFLINE)` ao delivery (linha 120, após o bloco).
- Failure Scenario: Cenário de colisão de roleid (duas sessões com o mesmo roleid, possivelmente via race condition no switchuser). A sessão que fecha não notifica o delivery do offline, e a entrada em roleinfomap vaza.
- Impact: Delivery mantém o role marcado como online. Vazamento de entrada em roleinfomap. O role pode ficar "trancado" — impossível relogar.
- Validation: Ler `glinkserver.cpp:107-120` — o `return` na linha 113 pula a linha 120 (`StatusAnnounce`).
- Fix Direction: Mover o `StatusAnnounce` para antes do bloco `if (roleid>0)` ou reestruturar para sempre enviar o announce antes de qualquer early-return.

---

### [LOW|CONFIRMED] GMShutupRole: bloco de atualização do shutuprolemap executado incondicionalmente (falta `else`)

- Type: data-consistency
- Evidence: `gmshutuprole.hpp:34-41` — Após o bloco `if (manager==lsm) { ... }`, há um bloco `{ lsm->shutuprolemap[dstroleid]=...; }` sem `else`. Comparar com `gmshutup.hpp:37-42` que tem `else { lsm->shutupusermap[dstuserid]=...; }`. No GMShutupRole, a atualização local ocorre tanto no caminho do cliente quanto no do delivery.
- Failure Scenario: Um GM cliente envia GMShutupRole. O mute local é aplicado imediatamente. Se o delivery rejeita a operação (ex.: o role não existe), o mute local persiste incorretamente até expirar por timeout.
- Impact: Mute local inconsistente com o estado do delivery. Duração do mute pode divergir.
- Validation: Comparar `gmshutuprole.hpp:34-41` vs `gmshutup.hpp:37-42`.
- Fix Direction: Adicionar `else` antes do bloco de atualização do shutuprolemap, alinhando com o padrão de GMShutup.

---

### [LOW|CONFIRMED] GMForbidSellPoint é um no-op — Process vazio

- Type: maintainability-risk
- Evidence: `gmforbidsellpoint.hpp:20-22` — `Process` é vazio. Sem privilege check, sem encaminhamento ao delivery. O protocolo é aceito em `state_GDataExchgServer` (`state.cxx:268`).
- Failure Scenario: Um GM envia GMForbidSellPoint. Nada acontece. Nenhuma resposta é enviada. O GM acredita que o comando foi ignorado ou que há um bug.
- Impact: Feature silenciosamente quebrada. GM não consegue proibir sellpoint via glinkd.
- Validation: Ler `gmforbidsellpoint.hpp:20-22`.
- Fix Direction: Implementar o handler (privilege check PRV_NOSELLPOINT + encaminhar ao delivery) ou remover o protocolo do state array se a feature não é suportada.

---

### [LOW|CONFIRMED] GMRestartServer: caminho do cliente desabilitado com `return;` e código morto

- Type: maintainability-risk
- Evidence: `gmrestartserver.hpp:39` — `return;` no topo do `if (manager==lsm)` torna todo o código subsequente (privilege check, encaminhamento) inalcançável.
- Failure Scenario: Um GM tenta reiniciar o servidor via cliente. O pacote é aceito pelo state machine mas o Process retorna imediatamente. O GM não recebe feedback.
- Impact: Feature desabilitada de forma não óbvia. Código morto confunde manutenção.
- Validation: Ler `gmrestartserver.hpp:38-55`.
- Fix Direction: Se a intenção é desabilitar o restart via cliente, remover o código morto e documentar. Se não, remover o `return;`.

---

### [LOW|CONFIRMED] static int retry_time em AnnounceProviderID — variável estática não thread-safe e não usada

- Type: maintainability-risk
- Evidence: `announceproviderid.hpp:20` — `static int retry_time;` é declarada, setada para 0 na linha 31, mas nunca lida. É uma variável estática de função compartilhada entre todas as invocações e threads.
- Failure Scenario: Múltiplos game servers reconectando simultaneamente escrevem na mesma variável estática sem proteção. Embora o valor não seja usado, é um code smell.
- Impact: Nenhum funcional, mas indica código não revisado.
- Validation: Ler `announceproviderid.hpp:20,31`.
- Fix Direction: Remover a variável `retry_time`.

---

## Otimizações e melhorias

1. **WorldChat broadcast O(n) por mensagem** (`worldchat.hpp:30-33`): `std::for_each` itera todos os roles online e envia individualmente. Para servidores com muitos jogadores, considerar batching ou usar o mecanismo de multicast do protocolo (`S2CMulticast`).

2. **AccumProto4Switch clona cada pacote** (`glinkserver.hpp:316-318`): Durante a janela de switch (15s), `AccumProto4Switch` chama `p.Clone()` para cada pacote recebido. Em servidores com alto tráfego, isso pode acumular muitos protocolos na `ProtocolQueue`. Considerar um limite no tamanho da fila.

3. **glinkd.cpp: atoi sem validação de config** (`glinkd.cpp:58-66`): Valores de config como `accumulate`, `max_users`, `halflogin_users`, `sender_interval` são lidos com `atoi()` sem validação. Se o conf estiver ausente ou inválido, `atoi` retorna 0. `accumulate_limit=0` desabilita o limite de buffer de saída. `sender_interval=0` pode causar divisão por zero em `RunTimerSender(0/100000)`. Validar valores de config com fallback seguro.

4. **Represent::Map cresce sem limite** (`protocol.h:76-89`): O `Represent::Map` (cache de encoding de protocolos) é global e nunca é limpo durante a execução. Cada protocolo enviado cria uma entrada. Para protocolos criados dinamicamente (ex.: chat, gamedata), isso pode crescer indefinidamente. Considerar limpeza periódica ou usar weak references.

5. **Octets::frob_size loop infinito potencial** (`octets.h:96-99`): `frob_size` dobra `tmp` até `size <= tmp`. Se `size >= 2^31`, o shift left causa overflow para 0, resultando em loop infinito. Atualmente mitigado pelo SizePolicy (max 1MB), mas se um protocolo for adicionado com SizePolicy grande, pode travar o servidor.

---

## Fatos verificados vs inferências

### Verificados (lidos diretamente no código):
- DeleteRole/UndoDeleteRole não chamam ValidUser/ValidRole (`deleterole.hpp:22-28`, `undodeleterole.hpp:22-28`)
- SelectRole não valida roleid da sessão (`selectrole.hpp:23-35`)
- CollectClientMachineInfo não tem validação (`collectclientmachineinfo.hpp:20-22`)
- SetCustomData não valida roleid (`setcustomdata.hpp:20-23`), GetCustomData valida (`getcustomdata.hpp:22-27`)
- GMForbidSellPoint é vazio (`gmforbidsellpoint.hpp:20-22`)
- GMShutupRole falta `else` (`gmshutuprole.hpp:34-41` vs `gmshutup.hpp:37-42`)
- GMRestartServer tem `return;` com código morto (`gmrestartserver.hpp:39`)
- Challenge nonce sobrescreve 8 bytes aleatórios (`glinkserver.cpp:42-43`)
- state_GSelectRoleServer timeout = 86400s (`state.cxx:133`)
- fprintf por pacote em Decode (`protocol.h:538-547`)
- halfloginset é global, sem limite por IP (`glinkserver.hpp:150-157,190`)
- OnDelSession early-return pula StatusAnnounce (`glinkserver.cpp:107-120`)
- static retry_time não usado (`announceproviderid.hpp:20,31`)
- GM commands (GMKickoutUser, GMShutup, GMKickoutRole, GMShutupRole, GMToggleChat, GMForbidRole, GMOnlineNum, GMListOnlineUser, GMGetPlayerConsumeInfo) fazem PrivilegeCheck no glinkd
- RequestLimiter tem rate limit para chat (5/10s), whisper (8/10s), findname (3/10s) (`requestlimiter.h:80-82`)
- SizePolicy max = 1048576 (1MB) para os maiores protocolos

### Inferências (não confirmadas no escopo do glinkd):
- Se gdeliveryd valida posse do roleid em DeleteRole/UndoDeleteRole/SelectRole — não verificado (fora do escopo do glinkd)
- Se a redução de entropia do nonce é explorável na prática — depende do algoritmo de hash e da implementação do cliente
- Se o DoS de halfloginset é prático — depende da capacidade do atacante de abrir conexões simultâneas (sem SYN flood protection verificado)
- Se `sender_interval=0` causa divisão por zero — não verificado em `RunTimerSender` (fora do escopo do glinkd, na framework share/)
