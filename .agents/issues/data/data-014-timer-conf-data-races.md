id: data-014
title: share/ — Timer::Update itera observers sem lock (UAF) e Conf::find usa operator[] sob read-lock (data race + crescimento do mapa)
category: data
type: bug
status: OPEN
severity: HIGH
owner: server153-framework (share/)
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/share/common/timer.h:46-49 — Update() itera observers() sem lock; Attach (:28) push_back e Detach (:29) erase sem lock — realocação durante iteração = UAF"
  - "src/server153/share/common/conf.h:82-84 — find() toma RDScoped (read lock) e usa confhash[section][key] (operator[] não-const INSERE default) → mutação de std::map sob read lock por múltiplos threads + crescimento indefinido; getkeys (:96) mesmo padrão"
impact: "Timer: crash em qualquer daemon multithread que registre timers após o startup (hoje Attach é majoritariamente em construtores estáticos — race latente). Conf: corrida de inserção concorrente em std::map (UB/corrupção de árvore RB) e leak de entradas fantasma a cada lookup de chave inexistente — conf.cpp:43 e thread.cpp:91 chamam find para chaves que podem não existir."
proposed_action: "Timer: mutex em observers() cobrindo Update/Attach/Detach (ou copy-and-swap atômico). Conf: usar confhash.find(section)+find(key) e retornar string vazia se ausente (nunca operator[]); opcionalmente cachear lookups no boot."
validation_notes: "TSan: dois threads fazendo Conf::find de chave inexistente sem warning; mapa de conf não cresce após N lookups; Attach após startup não crasha Update."
