id: data-009
title: cgame — validação de walkability do movimento do player está comentada (CheckMove); movemap carregado e não usado
category: data
type: bug
status: OPEN
severity: HIGH
owner: server153-cgame
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cgame/gs/player.cpp:12419 — `// if(!CheckMove(use_time,mode)) return -1;` COMENTADO; CheckPlayerMove (:12415) valida só velocidade e eixo Y"
  - "src/server153/cgame/gs/world.cpp:1602 — w_movemap é carregado; usado apenas em spawns de NPC (npcgenerator.cpp:4306), não na validação de movimento do jogador"
impact: "Cliente modificado anda através de paredes/água/áreas restritas dentro dos limites de velocidade (wall-walk/teleport suave) — bypass de barreiras, posicionamento injusto em PvP, acesso a áreas protegidas. A validação de velocidade ativa pega speed hack, mas não pathing."
proposed_action: "Reativar CheckMove ou implementar validação de walkability com w_movemap/path_finding::GetValidPos (mesma infra usada para spawns de NPC) no caminho de movimento do player. Medir custo por pacote de movimento antes de ligar em produção (é o hot path)."
validation_notes: "Cliente de teste com posição dentro de parede: o gs deve rejeitar/corrigir; benchmark do custo do CheckMove no heartbeat antes/depois."
