id: security-001
title: Wire DoS no GNET — frob_size loop infinito e reserve sem cap derrubam qualquer daemon com um pacote de 5 bytes
category: security
type: bug
status: OPEN
severity: HIGH
owner: server153-framework (share/)
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/share/common/octets.h:93-98 — frob_size: `while(size>tmp) tmp<<=1;` com size=0xFFFFFFFF faz tmp (0x80000000) wrapar para 0 → loop infinito (size_t 32-bit)"
  - "src/server153/share/io/protocol.h:523-530 — Decode lê CompactUINT(size) do pacote e chama data.reserve(size) sem cap; SizePolicy default (protocol.h:573) retorna true para qualquer tamanho"
  - "src/server153/share/common/octets.h:106-109 — Rep::create usa operator new = malloc(size+extra) sem check NULL; reserve(0x7FFFFFFF) em processo 32-bit → malloc falha → placement new em NULL = crash"
impact: "Um cliente conectado a qualquer daemon (glinkd:29000 é o exposto) envia um pacote de 5 bytes (header e0 ff ff ff ff): a thread de poll entra em loop infinito e o daemon inteiro para de responder (não responde nem a SIGTERM). Variante com size=0x7FFFFFFF: malloc de ~2GB → crash/OOM. Afeta TODOS os daemons que usam o framework (todos)."
proposed_action: "1) Em Protocol::Decode, rejeitar size > bytes restantes do stream antes do reserve (cap natural). 2) Guard em frob_size (throw se size > MAX_CAP) ou cálculo sem loop via __builtin_clz. 3) Check de NULL/bad_alloc em Rep::create. Epic: estabilidade do servidor — fazer antes de qualquer exposição pública."
validation_notes: "Unit test read-only: chamar Octets::reserve(0xFFFFFFFF) com timeout e esperar exceção, não hang (não rodar contra o stack vivo). Teste de wire: pacote com header size=0xFFFFFFFF via gclient de teste deve ser rejeitado com erro de protocolo, não travar o glinkd."

## Links
- Review: .agents/reviews/code-audit-server153-2026-10-28.md (área: server153-audit-share.md)
- Related issues: security-010, testing-002
