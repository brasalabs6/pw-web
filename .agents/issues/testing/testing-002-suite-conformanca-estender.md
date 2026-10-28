id: testing-002
title: Suite de conformança cobre só CreateRole — estender para challenge/login chain, pacote frob (regressão do wire DoS) e GetState de skills
category: testing
type: gap
status: OPEN
severity: MEDIUM
owner: server153-testing
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "tests/server153_createrole_wire.cpp — único teste de wire (CreateRole 1704 bytes, 23 campos); scripts/verify.sh — wire + shell + python + core engine, nada de protocolo de login ou de robustez de pacote"
impact: "O wire DoS de security-001 (pacote size=0xFFFFFFFF) e o handshake challenge/response não têm teste de regressão — um refactor no framework pode reintroduzir travamentos que só aparecem com cliente real. O crash de GetState(1) (data-013) também não tem guarda."
proposed_action: "1) Teste de wire hostil: pacote com size=0xFFFFFFFF e size=0x7FFFFFFF deve ser rejeitado com Marshal::Exception (não hang/crash) — vira PASS quando security-001 for fixado; até lá documenta o estado. 2) Teste de marshal do Challenge (version/edition/nonce). 3) Teste de SkillStub::GetState com stub de 1 estado retornando NULL tratado. 4) Integrar ao verify.sh com o mesmo esquema BLOCKED do wire atual."
validation_notes: "verify.sh roda os novos testes; o teste frob falha (documentando o bug) até o fix de security-001 e passa depois — nunca é skipado silenciosamente."
