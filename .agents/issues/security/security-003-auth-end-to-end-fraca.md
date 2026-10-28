id: security-003
title: Autenticação end-to-end fraca — dev-auth sem segredo, sem rate limit, matrixtoken aceita tudo, GM concedido sem validação
category: security
type: gap
status: OPEN
severity: HIGH
owner: server153-gauthd
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/gauthd/matrixpasswd.hrp:54-58 (e matrixpasswd2.hrp:55-59) — hash = MD5(conta‖conta), sem salt, sem consulta a conta existente; única validação: userid%16==0 (a conta É a credencial)"
  - "src/server153/cnet/gauthd/gquerypasswd.hrp:48-50 — HMAC sobre o challenge está COMENTADO; devolve o hash cru"
  - "src/server153/cnet/gauthd/matrixtoken.hrp:39-53 — arg.token nunca é lido; aceita qualquer token (bypass se token_login_support=true)"
  - "src/server153/cnet/gauthd/queryuserprivilege.hpp:18-49 — concede privilégios GM máximos a qualquer userid sem validar o solicitante"
  - "src/server153/cnet/gauthd/accountingrequest.hpp:20-21 — MAC MD5 com shared_key default fraca + comparação memcmp não constante-tempo (PWSL/include/octets.h:50)"
  - "src/server153/cnet/glinkd/glinkserver.cpp:42-43 — nonce do challenge: 8 de 16 bytes sobrescritos com constantes do servidor (entropia efetiva 8 bytes)"
  - "Ausência total de rate-limiting/lockout em matrixpasswd/matrixpasswd2/matrixtoken/gquerypasswd/userlogin(2) (grep de attempt/lockout/throttle = 0 hits)"
impact: "Qualquer peer que alcance o gauthd autentica como qualquer conta múltiplo de 16 (o 'segredo' é enumerável); brute-force online irrestrito; escalação a GM sem autorização; bypass via token se a flag for ligada. Aceitável apenas como dev-auth isolado em localhost — precisa virar issue de produto antes de qualquer jogador externo."
proposed_action: "1) Backend de contas real (MariaDB já importado): senha hash+salt (bcrypt/argon2 ou HMAC-SHA256), lookup de existência antes de responder. 2) Rate limit por (userid, origem) com lockout. 3) Rejeitar RPC_MATRIXTOKEN enquanto não houver emissor de token. 4) ValidUser no QueryUserPrivilege + privilégios do DB. 5) Restaurar HMAC do gquerypasswd; HMAC-SHA256 + comparação constante-tempo no accounting. 6) Não sobrescrever o nonce aleatório. Decisão de produto (auth real) → user-only; o resto é engenharia."
validation_notes: "Depois do fix: conta inexistente deve ser rejeitada; 10 falhas seguidas devem bloquear temporariamente; token arbitrário deve dar retcode de erro; QueryUserPrivilege de sessão não autorizada deve recusar."
