id: data-012
title: cskill — SkillWrapper::Swap troca cd_adjust_count com sw.cd_adjust (typo); cooldowns corrompidos em troca de forma
category: data
type: bug
status: OPEN
severity: HIGH
owner: server153-cskill
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cskill/skill/skillwrapper.cpp:1162-1163 — `std::swap(cd_adjust, sw.cd_adjust);` (correto) seguido de `std::swap(cd_adjust_count, sw.cd_adjust);` (BUG: deveria ser sw.cd_adjust_count)"
  - "Consumo: skillwrapper.h:354 GetCDAdjust() usa cd_adjust_count/cd_adjust; skillfilter.h:16957 consulta GetCDAdjustCount()"
impact: "Após troca de forma (GodEvilConvert/EventChange recriam o wrapper), os campos de ajuste de cooldown ficam cruzados entre os wrappers: buff/debuff de CD aplicado o número errado de vezes ou com magnitude errada — cooldowns duplicados ou perdidos. Bug de balanceamento de combate."
proposed_action: "Trocar `sw.cd_adjust` por `sw.cd_adjust_count` na linha 1163 (1 caractere). Teste de identidade: após Swap, cada campo deve estar no campo homônimo do outro wrapper."
validation_notes: "Unit test: dois wrappers com cd_adjust/count conhecidos, Swap, verificar preservação de identidade dos pares."
