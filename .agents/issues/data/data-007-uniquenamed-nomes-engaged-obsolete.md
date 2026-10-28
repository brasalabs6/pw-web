id: data-007
title: uniquenamed — nomes ENGAGED nunca são reclamados (código morto) e OBSOLETE nunca é liberado; exaustão gradual de nomes e name-squat
category: data
type: bug
status: OPEN
severity: HIGH
owner: server153-uniquenamed
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/uniquenamed/precreaterole.hrp:46-59 — `if(!(ENGAGED==status && time>300)) return DUPLICATE; else return DUPLICATE;` — o ramo de reclaim (ENGAGED>300s) é código morto; mesmo padrão em preplayerrename.hrp:44-58, precreatefaction.hrp:55-66, precreatefamily.hrp:55-70"
  - "src/server153/cnet/uniquenamed/postplayerrename.hpp:39-46 (e postfactionrename.hpp:48-54) — renomear marca o nome antigo como UNIQUENAME_OBSOLETE sem liberá-lo; nenhum Pre* trata OBSOLETE como reutilizável; só postdeleterole.hrp:44 faz del"
  - "Agravante: uniquenamed.cpp:232-235 — case_insensitive default 0 (conf ausente); TransformName (uniquenameserver.cpp:33-41) só normaliza ASCII A-Z"
impact: "Toda criação/renomeação que falha entre Pre e Post (crash do gdeliveryd, timeout) trava o nome como ENGAGED para sempre; nomes renomeados ficam OBSOLETE bloqueados. Com o tempo o pool de nomes se esgota; atacante dispara PreCreateRole em massa e nunca confirma (name-squat DoS de namespace). Case-sensitive + TransformName ASCII-only permite look-alike ('Admin' vs 'admin')."
proposed_action: "1) No ramo else (ENGAGED && time>300): apagar a entrada stale e prosseguir com a nova reserva. 2) Coletor periódico de ENGAGED>300s. 3) Tratar OBSOLETE como reutilizável no Pre* (ou purge com idade). 4) Definir case_insensitive no conf e estender TransformName ao charset do jogo (decisão de produto → user-only)."
validation_notes: "Teste: PreCreateRole seguido de crash (sem Post) → após 300s o nome deve estar reutilizável; renomear 'Velho'→'Novo' → 'Velho' deve poder ser registrado por outro player."
