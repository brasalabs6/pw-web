id: data-011
title: gamedbd InitGUID com while(...); vazio — hang no boot (reboot no mesmo segundo) ou GUID colidindo sem re-checagem
category: data
type: bug
status: OPEN
severity: MEDIUM
owner: server153-gamedbd
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/gamedbd/gamedbmanager.cpp:33-37 — `while(plog->find(key,value,txn));` com ponto-e-vírgula (corpo vazio); o bloco { key.clear(); guid += 0x100000000LL; key << guid; } executa UMA vez fora do loop"
impact: "(a) Se a chave guid (timestamp<<32) já existir no syslog — reboot dentro do mesmo segundo ou entrada residual — find retorna true para sempre: gamedbd nunca sobe (hang de boot). (b) Caso contrário, o GUID é incrementado uma vez sem verificar se o novo valor está livre — colisão não detectada (PutSyslog usa GetGUID() como chave, utilfunction.cpp:47-68)."
proposed_action: "Remover o ponto-e-vírgula: `while(plog->find(key,value,txn)) { key.clear(); guid += 0x100000000LL; key << guid; }` — mudança de 1 caractere."
validation_notes: "Boot duas vezes no mesmo segundo (ou com entrada guid residual no syslog) deve subir sem hang; GUIDs sequenciais não colidem."
