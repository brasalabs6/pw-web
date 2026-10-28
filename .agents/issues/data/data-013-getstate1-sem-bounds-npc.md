id: data-013
title: cskill — GetState(1) sem bounds-check em NpcStart/NpcRun; 389 skills de estado único = crash determinístico se atribuídos a NPC
category: data
type: bug
status: OPEN
severity: HIGH
owner: server153-cskill
evidence_quality: likely
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cskill/skill/skillwrapper.cpp:990-992 — `skill->GetStub()->GetState(1)->GetTime(NULL)` sem checar tamanho; skill.h:778 NpcRun idem GetState(1)"
  - "src/server153/cskill/skill/skill.h:345-348 — GetState protege só com ASSERT (aborta no build atual, sem NDEBUG — gcc.defs.mk define _DEBUG); release = leitura OOB silenciosa"
  - "Contagem: 389 dos ~2879 skills gerados têm só State1 (grep -L 'class State2' skills/skill*.h)"
impact: "NPC configurado com skill de estado único → NpcStart → GetState(1) → ASSERT aborta o gs (build atual) ou segfault (release). Crash determinístico da instância por configuração de gamedata."
proposed_action: "1) GetState retorna NULL em OOB (em vez de ASSERT+UB) e callers tratam. 2) NpcStart/NpcRun checam GetStateSize()>=2 antes de GetState(1). 3) Validação na carga: skills de estado único não podem ser atribuídos a NPC."
validation_notes: "Atribuir skill de 1 estado a um NPC de teste deve logar erro e não crashar; GetState(1) em stub de 1 estado retorna NULL tratado."
