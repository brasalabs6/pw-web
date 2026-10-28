id: security-002
title: glinkd encaminha operações de role sem validar posse (DeleteRole, UndoDeleteRole, SelectRole, SetCustomData, CollectClientMachineInfo)
category: security
type: bug
status: OPEN
severity: HIGH
owner: server153-glinkd
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/glinkd/deleterole.hpp:22-28 e undodeleterole.hpp:22-28 — Process encaminha ao delivery sem ValidUser/ValidRole (contraste: createrole.hpp:23 valida)"
  - "src/server153/cnet/glinkd/selectrole.hpp:23-35 — monta PlayerLogin com roleid do payload sem validar (contraste: enterworld.hpp:25-34 valida roleinfomap+sid)"
  - "src/server153/cnet/glinkd/setcustomdata.hpp:20-23 — sem ValidRole (contraste: getcustomdata.hpp:22-27 valida); collectclientmachineinfo.hpp:20-22 — userid do payload, sem nenhuma checagem"
impact: "Cliente autenticado como user A opera roles do user B: deletar personagem alheio (se o delivery não revalidar), entrar no mundo como outro personagem, sobrescrever custom data de terceiro, injetar machineinfo falsa para qualquer userid. Violação de isolamento entre contas + perda de dados."
proposed_action: "Adicionar a validação de posse existente (ValidUser/ValidRole, padrão de createrole/getcustomdata) nos 5 handlers. Verificar se o gdeliveryd revalida posse nesses fluxos (defesa em profundidade). Epic: hardening do glinkd."
validation_notes: "Teste com duas contas: user A tenta DeleteRole/SelectRole com roleid do user B — deve ser rejeitado no glinkd. Grep de regressão: nenhum handler de role/user no glinkd sem ValidUser/ValidRole."

## Links
- Review: .agents/reviews/code-audit-server153-2026-10-28.md (área: server153-audit-glinkd.md)
- Related issues: security-009
