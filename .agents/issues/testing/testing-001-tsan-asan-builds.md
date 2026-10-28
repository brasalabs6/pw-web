id: testing-001
title: Builds TSan/ASan ausentes — a classe inteira de UAF/races (UserContainer, subsistemas, gfaction, timer) seria capturada sob carga sintética
category: testing
type: gap
status: OPEN
severity: HIGH
owner: server153-build/CI
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "A auditoria de 2026-10-28 confirmou estaticamente ~10 famílias de UAF/data race (data-001/002/003/004/010/014) que um build ASan+TSan do gdeliveryd/gfaction capturaria em carga sintética de re-login/kickout/join-leave"
  - ".github/workflows/server153-build.yml — nenhum job de sanitizer; scripts/verify.sh — sem alvo sanitizado"
impact: "Sem sanitizers, os fixes de concorrência (data-001..004) não têm como ser validados — e regressões voltam a derrubar o servidor em produção de forma intermitente (o padrão 'crash aleatório' que o watchdog mascara)."
proposed_action: "1) Job CI (ou script local) que builda gdeliveryd+gfaction com -fsanitize=address,undefined e roda um driver de carga sintética (2+ gclients re-logando/kickando o mesmo userid; join/leave de faction em loop). 2) Variantes TSan para as races de timer/subsistemas. 3) Sanitizers opcionais no build normal atrás de flag (SAN=1)."
validation_notes: "O driver de carga sob ASan deve rodar sem reports após os fixes de data-001..004; com TSan, sem warnings de data race nos caminhos corrigidos."
