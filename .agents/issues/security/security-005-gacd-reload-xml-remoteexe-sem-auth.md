id: security-005
title: gacd — ACReloadConfig sem auth desativa toda a punição anti-cheat; XML sem NULL-check derruba o daemon; ACRemoteExe sem auth injeta código no cliente
category: security
type: bug
status: OPEN
severity: HIGH
owner: server153-gacd
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/gacd/punishmanager.cpp:71 — OnUpdateConfig chama ClearRules() ANTES de ler o config novo: XML vazio = zero regras = nenhum cheat é punido"
  - "src/server153/cnet/gacd/acreloadconfig.hpp:20 + gaccontrolserver.cpp:55-62 — reload aceito da primeira conexão do control-port (29712) sem autenticação; isec=6/osec=5 é compressão MPPC, não criptografia"
  - "src/server153/cnet/gacd/xmlconfig.cpp:106-108 — xmlParseMemory + xmlDocGetRootElement sem NULL-check; Build(pDoc, NULL) deref NULL->name (xmlconfig.cpp:76-77) → SIGSEGV com XML malformado"
  - "src/server153/cnet/gacd/acremoteexe.hpp:55-79 — REMOTEEXE_MAKE/RUN enviam exe (Octets da rede) como code-pieces ao cliente, sem validar roleid nem autenticar a sessão"
impact: "1) Um peer local no control-port neutraliza TODO o anti-cheat com um reload vazio (ClearRules) — silencioso e persistente até restart. 2) XML malformado crasha o gacd (DoS da proteção). 3) ACRemoteExe é execução arbitrária de código no cliente de jogo via canal sem auth (RCE client-side)."
proposed_action: "1) Autenticar ACReloadConfig/ACRemoteExe (token/HMAC compartilhado; ou restringir o control-port a unix socket). 2) Atomic swap no reload: parse em temporário, só ClearRules+apply se o parse for válido e completo. 3) NULL-checks em pDoc/root antes de Build; limitar tamanho do buffer. 4) Remover xmlMemoryDump() de produção (xmlconfig.cpp:111,128)."
validation_notes: "Teste 1: reload com XML vazio deve ser rejeitado e as regras antigas devem permanecer. Teste 2: XML malformado deve logar erro, não crashar. Teste 3: ACRemoteExe sem token deve ser recusado."
