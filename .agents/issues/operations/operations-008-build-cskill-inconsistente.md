id: operations-008
title: Build cskill inconsistente — skill/makefile produz libskill.a 64-bit (sem -m32); o path documentado não gera o libskill.so que o gs linka; stubs sem dependência de headers
category: operations
type: bug
status: OPEN
severity: MEDIUM
owner: server153-cskill (build)
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cskill/skill/makefile — CPP = g++ (SEM -m32; compilação-teste produz ELF 64-bit); target all = libskill.a; Makefilelib (via mk/gcc.defs.mk:6 com -m32) produz o libskill.so 32-bit que o gs linka (cgame/gs/Makefile:23)"
  - "server153/Makefile:3 — execdirs inclui cskill/skill/ (roda skill/makefile no subs); o .a 64-bit resultante é inútil para o gs 32-bit"
  - "skill/makefile — STUBOBJS sem regra de dependência de ../skills/*.h (Makefilelib:40 tem; o build ativo via skill/makefile não) → stubs*.o stale quando skillNNNN.h muda; stubs1.o = 8.8MB com -O0 -w"
impact: "make all limpo pela documentação não produz o artefato que o gs consome (link falha ou .so stale com lógica de skill antiga — o mesmo padrão do make -B já documentado para headers). Risco de divergência silenciosa de skills."
proposed_action: "Unificar: um makefile só, com -m32, produzindo libskill.a E libskill.so que o gs linka; skill/makefile vira wrapper ou é removido; -MMD -MP nos stubs para tracking de dependência."
validation_notes: "make limpo do zero deve produzir libskill.so 32-bit e o gs deve linkar; tocar um skillNNNN.h deve reconstruir os stubs*.o dependentes."
