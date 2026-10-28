id: security-006
title: logservice aceita logs de qualquer origem sem auth — forja de trilha de auditoria e poluição do log de cash
category: security
type: bug
status: OPEN
severity: MEDIUM
owner: server153-logservice
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/logservice/logservice.conf:1-20 — [LogserviceServer] udp 11100 e [LogserviceTcpServer] tcp 11101 sem isec/oseckey (sem criptografia nem MAC)"
  - "src/server153/cnet/logservice/remotelog.hpp:22-24 — Process passa priority/hostname/servicename/msg do pacote direto ao LogDispatch sem validar origem nem campos (idem remotelogvital/statinfo)"
  - "src/server153/cnet/logservice/logdispatch.h:149-163 — priority controlado pelo remetente escolhe o arquivo (LOG_CASH → world2.cash); h:75 — msg escrito sem sanitizar newlines"
impact: "Qualquer processo que alcance 11100/11101 (localhost hoje; fatal se exposto) forja entradas que parecem vir de qualquer servidor — inclusive no world2.cash (trilha financeira) — e oculta ataques reais. Newlines no msg criam entradas multi-linha indistinguíveis. Sem rate-limit, enche disco."
proposed_action: "1) HMAC compartilhado entre logservice e logclient (biblioteca já linkada por todos os daemons); rejeitar pacote sem MAC. 2) Sanitizar msg (escapar \\n/\\r e controle). 3) Whitelist de priority por origem. 4) Rate-limit por origem."
validation_notes: "Pacote RemoteLog sem MAC deve ser descartado com log de erro; msg com \\n deve aparecer em uma linha só; priority inválida rejeitada."
