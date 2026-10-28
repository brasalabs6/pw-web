id: operations-009
title: Performance do framework GNET — PollIO single-thread, 2+ alocações por pacote, Represent::Map com mutex global, GetOsbytes com write-lock, frob_size 2x, MMX no compress
category: operations
type: enhancement
status: OPEN
severity: MEDIUM
owner: server153-framework (share/)
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "share/io/pollio.h:490-500 — PollIO single-thread; handler bloqueado para o I/O do daemon inteiro (agravado por security-001/010 que rodam na mesma thread)"
  - "share/io/protocol.h:455-466 — new Task + Protocol::Create→Clone() + cópias de Octets por pacote (2+ heap allocs); protocol.h:49-88 — Represent::Map com mutex GLOBAL em todo Encode()"
  - "share/io/protocol.h:412 — GetOsbytes usa WRScoped (exclusivo) para LEITURA de estatística; octets.h:94-98 — frob_size arredonda para potência de 2 (até 50% de desperdício)"
  - "share/io/streamcompress.h:20-50, mppc.h:110-140 — compressão usa registradores MMX (obsoleto; estado FPU compartilhado entre threads até o emms)"
  - "share/common/octets.h:38-43 — addref sem \"memory\" clobber (release tem — consistência de refcount frágil fora de x86)"
impact: "Sob carga de milhares de jogadores: thread de poll como gargalo, churn de alocação por pacote, contenção no mutex global de encode, MMX limitando portabilidade. Não são bugs hoje — são os limites de escala do framework."
proposed_action: "Prioridade: 1) GetOsbytes → RDScoped (1 linha). 2) Pool de Task/Protocol ou dispatch inline para protocolos leves. 3) Cache per-thread de Represent. 4) Avaliar sharding do PollIO por fd. 5) MMX→SSE2 quando o port WASM/portabilidade exigir. Medir antes de otimizar (perf counters no PollIO)."
validation_notes: "Benchmark de throughput (packets/s) antes/depois de cada mudança; nenhuma regressão no wire conformance test."
