# Auditoria READ-ONLY — src/server153/cgame/

Branch: `feat/pw-source-1.5.3-ci-build`  
Data: 2026-10-28  
Método: grep/sed dirigido, leitura de código-fonte. Nenhuma edição no repositório.

Fixes pw-web JÁ aplicados (não reportados): guards find/erase do hashtab.h (tabela vazia); crash do pai do gs é cosmético conhecido; serialização logout vs re-login (commit f6e0abb7); RWLock scoped wrappers (commit bc2e616b); EnterWorld rwlock (commit 74df6c67).

---

### [Critical|Confirmed] hashtab: get/nGet/put_noresize/find_or_insert/begin/elems_in_bucket chamam get_hash sem guard de tabela vazia
- Type: bug
- Evidence: `include/hashtab.h:455` (`get`), `:470` (`nGet`), `:396` (`put_noresize`), `:378` (`find_or_insert`), `:170` (construtor de iterator com `_Node*`), `:341` (`elems_in_bucket`). Todas chamam `get_hash(key)` que faz `_hash(key) % _buckets.size()`. Os guards de tabela vazia foram aplicados apenas em `find` (`:313`) e `erase` (`:486`); os demais caminhos não foram protegidos.
- Failure Scenario: O crash SIGFPE documentado (commit b0dfe5ef) ocorre porque destrutores de ClassInfo em libskill.so chamam `erase`/`find` no registro de classes depois que o mapa (hashtab) já foi destruído — `_buckets` tem size 0 → div por zero. Se qualquer outro destrutor estático (em libskill.so, libtask.so ou no próprio gs) chamar `get`, `nGet`, `put_noresize` ou `find_or_insert` no registro destruído, o mesmo SIGFPE ocorre, pois esses caminhos não têm o guard. O cenário é real: `ClassInfo::GetRunTimeClass(guid)` (`substance.cpp:95`) chama `find` (protegido), mas qualquer código que use `get`/`nGet` no registro durante shutdown seria vulnerável.
- Impact: Crash do processo gs (SIGFPE/trap divide error) em shutdown ou descarga de DSO, potencialmente levando down todos os mundos filhos do gs.
- Validation: Compilar um TU de teste que instancie um `hashtab<int,int,_hash_function>`, chame `clear()`+destrua o vector manualmente (simulando `_buckets.size()==0`), então chame `get(1)` ou `nGet(1)` — reproduz o SIGFPE. Alternativamente, grep por `GetClsInfoMap().get\|GetClsInfoMap().nGet\|GetClsInfoMap().put` em todos os .cpp das DSOs.
- Fix Direction: O guard deve ser centralizado em `get_hash` (retornar 0 ou um sentinel quando `_buckets.empty()`), não replicado em cada chamador. Alternativamente, resolver a causa raiz: tornar o registro de ClassInfo "leaky" (nunca destruído) usando um `std::aligned_storage` ou alocando no heap sem destrutor, eliminando o problema de ordem de destruição entre DSOs.

### [High|Confirmed] ClassInfo::~ClassInfo apaga do mapa incondicionalmente, inclusive para classes com guid=-1
- Type: bug
- Evidence: `substance.cpp:78` — `GetClsInfoMap().erase(_guid);` no destrutor, sem checar `if(_guid != -1)`. O construtor (`substance.cpp:29`) checa `if(guid != -1)` antes de inserir no mapa, mas o destrutor não tem a checagem simétrica. `substance::m_classsubstance` é registrada com guid=-1 (`substance.cpp:6`).
- Failure Scenario: No shutdown, `substance::m_classsubstance` (global estática no gs binary) é destruída. Seu destrutor chama `GetClsInfoMap().erase(-1)`. Se o mapa já foi destruído (ordem de DSO), o guard de `erase` captura o caso de buckets vazios. Mas se o mapa ainda existe, `erase(-1)` procura a chave -1 (que nunca foi inserida) — inofensivo, porém se o mapa está em estado parcialmente destruído (buckets válidos mas nodes corrompidos), pode crashar.
- Impact: Baixo na prática (erase de chave inexistente é no-op), mas é uma fragilidade que depende do guard de hashtab para não crashar.
- Validation: Verificar que `erase` em uma hashtab com buckets não-vazios para uma chave inexistente retorna false sem efeito colateral (já confirmado pelo código — `erase` percorre a bucket chain e retorna false se não encontra).
- Fix Direction: Adicionar `if(_guid != -1) GetClsInfoMap().erase(_guid);` no destrutor, espelhando o construtor. Ou melhor: resolver a causa raiz do item anterior (registro leaky).

### [High|Confirmed] Validação de movimento: checagem de movemap (walkability) está comentada
- Type: security
- Evidence: `player.cpp:12419` — `//	if(!CheckMove(use_time,mode)) return -1;` — a chamada para `CheckMove` está comentada. A função `CheckPlayerMove` (`player.cpp:12415`) valida apenas velocidade (speed_square vs max_speed/cur_speed) e direção Y (jump/fall). Não há validação de walkability/terrain/movemap.
- Failure Scenario: Um cliente modificado envia pacotes de movimento com velocidade válida mas para posições dentro de paredes, através de água, fora do mapa, ou para áreas inacessíveis. O servidor aceita porque apenas a velocidade é verificada. O jogador atravessa paredes, alcança áreas restritas, ou escapa do mapa.
- Impact: Exploit de movimento (wallhack/teleport dentro dos limites de velocidade). Permite acesso a áreas protegidas, bypass de barreiras de colisão, e posicionamento injusto em PvP/PvE.
- Validation: Confirmar que `CheckMove` existe e o que faz (grep por `CheckMove` em player.cpp e arquivos relacionados). Verificar se `w_movemap` é carregado (`world.cpp:1602`) mas não usado na validação de movimento do jogador.
- Fix Direction: Reativar a chamada `CheckMove` ou implementar validação de walkability usando `w_movemap`/`path_finding::GetValidPos` (já usado em npcgenerator.cpp:4306 para spawns de NPC). Validar que a posição de destino é alcançável do ponto atual via o movemap.

### [Medium|Confirmed] NPC spawn Y abaixo do terreno: GetHeightAt retorna vHeightMin para coordenadas fora dos bounds
- Type: bug
- Evidence: `terrain.cpp:189` — `GetHeightAt` retorna `m_config.vHeightMin` quando `(nX < 0 || nZ < 0 || nX >= m_nNumVertX || nZ >= m_nNumVertZ)`. `terrain_gen_pos::Generate` (`npcgenerator.cpp:4306-4310`) faz `pos.y = offset; float h = plane->GetHeightAt(pos.x,pos.z); pos.y += h;`. Se a região de spawn se estende além dos bounds do terrain, `h = vHeightMin` (constante mínima), e o NPC spawna em `vHeightMin + offset`, que pode estar bem abaixo do terreno real na borda.
- Failure Scenario: Uma região de spawn definida no config se estende até a borda ou além do terrain. NPCs nessa borda recebem `y = vHeightMin + offset_terrain`. Se `vHeightMin` é um valor negativo ou muito baixo (configurado como limite inferior), e/ou `offset_terrain` é negativo, o NPC aparece abaixo do solo. O NPC fica preso no subterrâneo, inacessível, ou cai infinitamente se não houver colisão.
- Impact: NPCs invisíveis/inacessíveis em bordas de mapa. Problema de gameplay (mob inalvejável, ou mob que aparece flutuando/caindo).
- Validation: Verificar o valor de `vHeightMin` no config do terrain (`gs.conf` ou arquivo de terrain). Verificar se regiões de spawn no config de NPC se estendem além dos bounds do terrain. Reproduzir com um spawn na borda do mapa e observar a posição Y.
- Fix Direction: Em `terrain_gen_pos::Generate`, se `GetHeightAt` retornar `vHeightMin` (indicando OOB), rejeitar a posição e tentar novamente (já há um loop de 5 tentativas, mas o fallback `:4317` também usa `GetHeightAt` sem checar OOB). Alternativamente, clampar a posição para dentro dos bounds antes de consultar a altura.

### [Medium|Likely] parallel_world_manager::Heartbeat: RunTick sem heartbeat_lock; w_player_count lido sob lock mas modificado sem lock
- Type: data-consistency
- Evidence: `parallel_world_manager.cpp:18-27` — o loop `pPlane->RunTick()` executa sem `_heartbeat_lock`. `:28` adquire `_heartbeat_lock` apenas para a seção de gestão de capacidade. `PlayerSwitchWorld` (`:100-170`) modifica `w_player_count` (via `RemovePlayer`/`InsertPlayer` que chamam `interlocked_increment`/`interlocked_decrement` em `world.h:239-265`) sem `_heartbeat_lock`. A seção de capacidade (`:36-66`) lê `w_player_count` sob `_heartbeat_lock` para decidir `w_obsolete` e `FreeWorld`.
- Failure Scenario: `Heartbeat` e `PlayerSwitchWorld` rodam em threads diferentes do pool (Heartbeat via `timer_tick`→`World_Tick_Task`; PlayerSwitchWorld via `MsgQueue::Run` processando comando de jogador). Durante um switch, o `w_player_count` do plano origem cai momentaneamente para 0 (jogador removido, ainda não adicionado ao destino). Se a Heartbeat lê 0 nesse instante e `WorldCapacityHigh()` é verdadeiro, marca o plano como `w_obsolete=1`. O timer de obsolete (10s+) mitiga o free prematuro, mas a decisão de obsolete pode ser tomada em um instante inconsistente. Mais grave: `RunTick()` (NPC AI, grid iteration) corre simultaneamente com `PlayerSwitchWorld` modificando o grid/slice do plano (RemovePlayer/InsertPlayer operam na slice).
- Impact: Potencial race na gestão de capacidade (marcar plano como obsoleto prematuramente, ou delay no free). Potencial acesso concorrente ao grid/slice durante switch de plano. O timer de 10s reduz a severidade do free prematuro, mas a race no grid entre RunTick e switch não é mitigada.
- Validation: Verificar se `world::RemovePlayer`/`InsertPlayer` (que operam na `slice* pPiece`) usam algum lock além do spinlock do jogador. Verificar se `RunTick`→`w_npc_gen.Run()` acessa a grid de jogadores. Confirmar que ambos rodam em threads do pool diferentes (Heartbeat é World_Tick_Task, switch é MsgQueue::Run).
- Fix Direction: Adquirir `_heartbeat_lock` (ou um lock por plano) antes de `RunTick`, ou serializar `PlayerSwitchWorld` com a Heartbeat. Alternativamente, usar um lock por plano (`w_plane_lock`) para serializar modificações estruturais do plano (RemovePlayer/InsertPlayer) com o RunTick.

### [Low|Confirmed] _enemy_list: iteração reversa com erase pula elementos quando dois consecutivos precisam ser apagados
- Type: bug
- Evidence: `player.cpp:9455-9470` — loop `for(;it > _enemy_list.begin(); ) { --it; if(it->second <=0) it = _enemy_list.erase(it); else it->second = 0; }`. `erase` retorna o iterador para o elemento SEGUINTE (forward), mas o loop decrementa `it` no topo, pulando o elemento retornado pelo erase.
- Failure Scenario: Se dois elementos consecutivos (do fim) têm `second <= 0`, o primeiro é apagado (erase retorna o próximo), o loop decrementa (pulando o próximo), e o elemento adjacente não é processado neste heartbeat. Ele será processado no próximo ciclo (4 ticks depois).
- Impact: Mínimo — entradas da enemy list (cooldown de aggro) não são resetadas por um ciclo extra. Sem crash, sem corrupção.
- Validation: Traçar o loop com [A,B,C,D] onde C e D têm second<=0: D é apagado (it→end), --it→B (C é pulado). Confirmar que C permanece com second>0 até o próximo heartbeat.
- Fix Direction: Após `erase`, não decrementar. Padrão correto: `if(erase) { it = erase(it); if(it == end) it = end; else continue sem --it; }`. Ou iterar com um padrão que não depende do retorno de erase.

---

## Otimizações e melhorias

1. **DoHeartbeat por tick para todos os objetos ativos** (`actobject.cpp:1300`): `DoHeartbeat` chama `OnHeartbeat`, `UpdateDataToParent`, `_commander->OnHeartbeat`, `UpdateVisibleState`, `RefreshSubscibeList` para cada objeto ativo a cada tick. `RefreshSubscibeList` envia dados aos subscribers a cada 16 ticks. Com milhares de NPCs e jogadores, isso é O(N * subscribe_list_size) por tick. O batching do `obj_manager` (MM_HEARTBEAT_TICK) já reduz o número de objetos por tick, mas o custo por objeto permanece. Considerar reduzir a frequência de `UpdateVisibleState` e `RefreshSubscibeList` para objetos que não mudaram de estado (`ClearRefreshState` no fim sugere que há um flag de mudança, mas `RefreshSubscibeList` é chamado incondicionalmente).

2. **Broadcast de posição é event-driven (bom)**: `object_move`/`object_stop_move` são enviados apenas em eventos de movimento (`player.cpp:3626`, `:3642`), não a cada tick. `notify_pos` (`player.cpp:3514`) é enviado apenas em transições de plano. Isso é eficiente — não há broadcast de posição por tick.

3. **operator new/delete global com header de 4 bytes** (`start.cpp:57-99`): Cada alocação faz `raw_alloc(n+4)`, armazena `size_t` em `p-4`, retorna `p+4`. O overhead de 4 bytes por alocação é aceitável, mas a contabilidade (`AllocSize`/`DeleteSize` com `interlocked_add` em arrays de 65536 counters) tem custo de sincronização por alocação. Em builds de produção, a contabilidade poderia ser desativada com um flag de compilação para reduzir overhead.

4. **fast_allocator: fix_allocator nunca libera chunks completamente vazios se _count <= 2*MAX_COUNT** (`amemory.h:286-289`): `remove_chunk` só é chamado quando `_count > 2*MAX_COUNT` (510). Se um fix_allocator tem entre 1 e 510 objetos alocados e todos são liberados, os chunks permanecem alocados (memória retida). Para um servidor de longa duração com padrões de alocação variáveis, isso pode acumular memória retida por tamanho de bucket. Considerar liberar chunks vazios mais agressivamente (quando `is_full() && _count > MAX_COUNT`).

---

## Fatos verificados vs inferências

### Verificados (lido diretamente no código)
- hashtab.h: guards apenas em `find`/`erase`; `get`/`nGet`/`put_noresize`/`find_or_insert`/`begin`/`elems_in_bucket` chamam `get_hash` sem guard.
- `substance.cpp:78`: destrutor de ClassInfo chama `erase(_guid)` sem checar `guid != -1`.
- `player.cpp:12419`: `CheckMove` está comentado; apenas validação de velocidade ativa.
- `terrain.cpp:189`: `GetHeightAt` retorna `vHeightMin` para OOB.
- `npcgenerator.cpp:4306-4310`: spawn Y = offset + GetHeightAt.
- `parallel_world_manager.cpp:18-27`: `RunTick` sem `_heartbeat_lock`; `:28+` adquire lock apenas para gestão de capacidade.
- `start.cpp:57-69`: `operator new` usa `raw_alloc(n+4)` e retorna `p+4` (header de 4 bytes).
- `global_manager.cpp:62-76`: `Heartbeat` é despachada para o thread pool via `World_Tick_Task`.
- `netmsg.cpp:346`: `user_offline` segura `spin_autolock(pPlayer->spinlock)` antes de chamar `LostConnection`.
- `actsession.h:99`: `~act_timer_session` chama `RemoveTimer` se `_timer_index >= 0`.
- `objmanager.h:170`: `DoChange` adquire `_lock_change`; `CollectHeartbeatObject` adquire `_lock_heartbeat`; `_obj_set` só é modificado sob `_lock_heartbeat`.
- `world.h:322-363`: `MapPlayer`/`UnmapPlayer`/`FindPlayer` usam `w_pmap_lock`.

### Inferências (não executadas, baseadas em leitura de código)
- A race entre `Heartbeat::RunTick` e `PlayerSwitchWorld` é inferida pela concorrência no thread pool; não foi reproduzida. O timer de obsolete de 10s+ mitiga o free prematuro, mas a race no grid/slice não é mitigada.
- O impacto do `operator new` desalinhado (4-byte em vez de 8-byte) é inferido: em x86-32 funciona (hardware tolera), mas é UB para tipos com requisito de 8-byte alignment. Não foi medido o impacto em performance.
- O bug do `_enemy_list` erase reverso foi traçado manualmente; não foi reproduzido em runtime. O impacto é mínimo (1 ciclo de delay).
- A ausência de `CheckMove` como exploit de movimento é inferida; não foi testado com um cliente modificado. A validação de velocidade ativa pode ser suficiente para detectar speed hacks, mas não walk-through-walls.
