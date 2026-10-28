id: data-004
title: gfaction muta vetor de membros sob read-lock — OnLeave/OnJoin/UpdateUser/UpdateFactionCache usam RDScoped
category: data
type: bug
status: OPEN
severity: HIGH
owner: server153-gfaction
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/gfaction/factiondb.cpp:638 (OnLeave: RDScoped + member.erase), :660 (OnJoin: RDScoped + member.add), :676 (UpdateUser: RDScoped + froleid/nickname write), :699 (UpdateFactionCache: RDScoped + level/announce write) — todos modificam sob READ lock"
  - "Contraste: OnLogin (factiondb.cpp:713) e OnAllianceApply (:1213) usam WRScoped corretamente; factiondb.h:100 — locker é Thread::RWLock"
impact: "Duas respostas de AddMember/DelMember simultâneas seguram o read-lock compartilhado e mutam o mesmo std::vector — UB (iteradores invalidados, vetor corrompido, crash do gfaction). Lista de membros fantasma/perdida, broadcasts incorretos."
proposed_action: "Trocar RDScoped por WRScoped nos 4 métodos (e auditar as ~38 ocorrências de RDScoped no factiondb.cpp: toda função que escreve usa WRScoped)."
validation_notes: "Grep de regressão: nenhuma mutação sob RDScoped no factiondb; teste de carga com join/leave simultâneos da mesma faction sem crash."
