id: operations-007
title: Build de produção subótimo — -O0 -g sem strip, -w suprime TODOS os warnings, imagem 480MB single-stage, CI sem cache/jobs=2/artifact de 120MB sempre
category: operations
type: enhancement
status: OPEN
severity: MEDIUM
owner: server153-build
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/share/mk/gcc.defs.mk:2 — CPP = g++ -w -m32 ... (-w silencia todos os warnings); builds atuais com -g -O0 (binários: gdeliveryd 37MB, gs 33MB, libskill.so 34MB); strip reduz gdeliveryd 67→7MB, gs 58→9MB"
  - "cloudflare/Dockerfile — single FROM debian:12, strip dentro do RUN (contexto de build 147MB com binários unstripped); libxml2.so.2 stale (1MB) sobrevive ao find -name '*.so' (não casa .so.2)"
  - ".github/workflows/server153-build.yml — apt do zero a cada run, --jobs 2 (runner tem 4 vCPU), artifact pw-server153-package (120MB) com if: always() e retention default 90d"
impact: "Binários 4-8x maiores e mais lentos em produção (debug info + O0); warnings reais mascarados por -w; cold start da imagem CF maior (480MB de pull); CI ~15min quando daria ~5min; storage de artifacts desperdiçado."
proposed_action: "1) Build de release: -O2 (ou -O1) + -g separado (debug symbols em artifact), remover -w e corrigir/permitir warnings seletivamente. 2) Stage(): strip ANTES do cp (contexto 147→60MB); Dockerfile multistage (debian:12-slim runtime → imagem ~250MB); remover libxml2.so.2 (find -name '*.so.*'). 3) CI: imagem base pré-construída com toolchain, --jobs 4, artifact if: success() + stripped + retention 7d."
validation_notes: "Comparar tamanho/velocidade dos binários release vs atual (benchmark de boot); imagem < 300MB; CI < 8min; warnings visíveis no build."
