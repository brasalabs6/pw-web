id: data-008
title: cgame hashtab — guards só em find/erase; get/nGet/put_noresize/find_or_insert/begin/elems_in_bucket dividem por zero em tabela vazia (causa raiz: ordem de destruição de DSOs)
category: data
type: bug
status: OPEN
severity: HIGH
owner: server153-cgame
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cgame/include/hashtab.h — guards pw-web apenas em find (:313) e erase (:486); SEM guard: get (:455), nGet (:470), put_noresize (:396), find_or_insert (:378), begin/iterator (:170), elems_in_bucket (:341) — todos chamam get_hash = _hash(key) % _buckets.size()"
  - "src/server153/cgame/gs/substance.cpp:78 — ~ClassInfo apaga do registro incondicionalmente (sem checar _guid != -1, que o construtor :29 checa); o crash SIGFPE real (pai do gs, commit a4fc35aa) vem dessa ordem de destruição entre DSOs (libskill.so/libtask.so vs gs)"
impact: "Qualquer destrutor estático que use get/nGet/put_noresize/find_or_insert no registro destruído → SIGFPE (divisão por zero) no shutdown/descarga de DSOs — mesma família do crash já visto; o guard parcial cobre 2 de ~8 caminhos. Pode derrubar processos de instância em runtime, não só no exit."
proposed_action: "Fix na causa raiz: tornar o registro de ClassInfo 'leaky' (alocado no heap, nunca destruído — elimina a ordem de destruição entre DSOs). Curto prazo: centralizar o guard em get_hash (retornar 0/bucket sentinel quando _buckets.empty()) em vez de replicar por método; checar _guid != -1 no destrutor do ClassInfo."
validation_notes: "TU de teste: hashtab com buckets vazios chamando get/nGet/put_noresize/find_or_insert deve retornar end()/false sem SIGFPE; boot+shutdown do gs limpo (o crash cosmético do pai some com o registro leaky)."
