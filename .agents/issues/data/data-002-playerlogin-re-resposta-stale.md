id: data-002
title: PlayerLogin_Re aplica resposta stale à sessão corrente sem validar roleid/status/gameid
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
  - "src/server153/cnet/gdeliveryd/playerlogin_re.hpp:100-227 — Process valida só FindUser(userid)!=NULL; não checa pinfo->roleid==roleid, pinfo->status==_STATUS_SELECTROLE, nem pinfo->gameid==src_provider_id; result==ERR_SUCCESS → RealLogin(roleid, pinfo) + envia PlayerLogin_Re ao pinfo->linksid; result==2||5 → pinfo->gameid=_GAMESERVER_ID_INVALID"
  - "Caminho cross-server pior: src/server153/cnet/gdeliveryd/remoteloginquery_re.hpp:31-72 — FindUser cru + RealLogin + status write + UserLogout, tudo sem lock (o caminho local playerlogin_re.hpp:164 usa WR lock)"
impact: "Re-login/logout entre o request e a resposta do gs: a resposta antiga é aplicada à sessão NOVA — RealLogin com role errado, PlayerLogin_Re enviado ao link errado, gameid da sessão nova zerado (quebra login em andamento ou personagem em jogo). Bug lógico distinto dos locks de data-001: é ausência de matching request↔response."
proposed_action: "Validar sob WR lock que pinfo->roleid==roleid && status==_STATUS_SELECTROLE && gameid==src_provider_id antes de aplicar; senão descartar (e PlayerOffline ao gs de origem). Idealmente casar resposta ao request com token/sequência. Envolver remoteloginquery_re no mesmo lock do caminho local."
validation_notes: "Teste de re-login rápido com role diferente entre request e resposta do gs: a resposta stale deve ser descartada com log; a sessão nova não pode ter gameid zerado."
