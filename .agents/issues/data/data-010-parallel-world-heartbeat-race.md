id: data-010
title: parallel_world_manager — Heartbeat (RunTick/NPC AI) sem lock concorre com PlayerSwitchWorld no grid do plano
category: data
type: risk
status: OPEN
severity: MEDIUM
owner: server153-cgame
evidence_quality: likely
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cgame/gs/instance/parallel_world_manager.cpp:18-27 — pPlane->RunTick() roda SEM _heartbeat_lock (adquirido só na seção de capacidade :28+); PlayerSwitchWorld (:100-170) modifica o grid/slice via RemovePlayer/InsertPlayer (world.h:239-265, interlocked counters) em outra thread do pool (Heartbeat=World_Tick_Task via global_manager.cpp:62-76; switch=MsgQueue::Run)"
  - "w_player_count lido sob lock na decisão de w_obsolete/FreeWorld, mas escrito sem lock — leitura em instante inconsistente (contador 0 momentâneo durante switch pode marcar plano obsoleto; timer de 10s mitiga)"
impact: "Acesso concorrente ao grid do plano durante switch de mundo paralelo: race em slice/grid entre NPC AI (RunTick) e a mudança estrutural do player — crash potencial ou estado de plano inconsistente; decisão de obsolete em leitura rasgada."
proposed_action: "Serializar RunTick com PlayerSwitchWorld (lock por plano ou _heartbeat_lock cobrindo o RunTick); ler w_player_count sob o mesmo lock da decisão de obsolete."
validation_notes: "TSan/carga: switches de plano frequentes + heartbeat ativo sem warnings; plano com 1 player não pode ser marcado obsoleto durante o switch dele."
