id: data-001
title: UserContainer do gdeliveryd sem lock sistêmico — FindUser/FindRole crus em ~60 handlers, login/logout/kickout mutam maps sem lock (UAF)
category: data
type: bug
status: OPEN
severity: HIGH
owner: server153-gdeliveryd
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/gdeliveryd/mapuser.h:591,599 — FindUser(int)/FindRole(int) retornam ponteiro cru sem lock; callers sem lock em ~60 sítios (playerlogin.hpp:31, kickoutuser.hpp:31, gmkickoutuser.hpp:27, ackickoutuser.hpp:24/139, remoteloginquery_re.hpp:31, waitqueue.cpp:233/318, map.cpp:423/474/482, autoteamman, countrybattleman, addcash_re.hpp:27, rolelist.hpp:52, qpaddcash.hpp:25, ...)"
  - "src/server153/cnet/gdeliveryd/mapuser.h:659-672 — RoleLogin faz rolemap.erase + delete user->role + insert SEM lock; callers playerlogin.cpp:112/155, waitqueue.cpp:266 sem lock"
  - "src/server153/cnet/gdeliveryd/map.cpp:346-360 — OnPlayerLogout chama FindRole cru + RoleLogout (delete role) sem lock"
  - "src/server153/cnet/gdeliveryd/kickoutuser.hpp:30-44, gmkickoutuser.hpp:27-31, ackickoutuser.hpp:23-28, userlogin2.hrp:36-60/139-170 — TOCTOU: FindUser sem lock → janela até o WR lock interno do UserLogout"
  - "src/server153/cnet/gdeliveryd/statusannounce.hpp:36-73 — residual do fix pw-web: check sob WR lock, logout fora; segundo FindUser cru"
  - "src/server153/cnet/gdeliveryd/map.cpp:199 — UserLogin sobrescreve UserInfo existente via operator= sem cleanup (leak de role/acstate)"
  - "src/server153/cnet/gdeliveryd/map.cpp:66-87,415-467 — CheckTimeoutUser itera/erase sets sem locker; ClearRemoteUsers/DisconnectZoneUsers iteram usermap sem lock"
impact: "Heap-use-after-free e data races em std::map sob re-login/logout/kickout concorrentes — a mesma família do crash REAL já corrigido (RoleLogout↔re-login, commit f6e0abb7), agora mapeada por completo: qualquer handler segurando um pinfo cru colide com o erase/delete de outro thread. Crash intermitente do gdeliveryd derruba todos os links (outage total)."
proposed_action: "Mudança de API com melhor custo/benefício: tornar FindUser/FindRole lock-safe (a versão com lock FindUser(int, UserInfo&) já existe em mapuser.h:520) ou exigir lock no caller via wrapper que retorna cópia; unificar os 8+ caminhos de logout num helper LogoutUser(userid, kicktype) que toma o WR lock e revalida estado; RoleLogin sob WR lock; UserLogin com kick explícito da sessão antiga. Fazer junto com testing-001 (ASan) para validar."
validation_notes: "Build ASan do gdeliveryd + carga sintética (2 conexões re-logando/kickando o mesmo userid em loop) deve rodar limpo; contagem estática de callers de FindUser(int) fora de lock deve zerar."
