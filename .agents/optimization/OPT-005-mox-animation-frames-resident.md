---
id: OPT-005
title: "Estudar residência parcial de frames MOX no heap"
priority: P2
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 335b8006e42a15d3c264e05f4dc60d8ed0273669
first_observed: 2026-10-28
validation: static-only
epic: e15-assets-streaming-perf-v1
---

# OPT-005 — Estudar residência parcial de frames MOX no heap

**Registro de otimização adiada:** não implementar antes do cliente WASM executar o jogo em browser desktop e mobile e possuir baseline de FPS/memória. Se um problema se comprovar blocker, promover a `.agents/issues` com reprodutor.

## Evidência no código
[`src/client153/Angelica2/Angelica3D/port/mox.cpp:92-155`](https://github.com/brasalabs6/pw-web/blob/335b8006e42a15d3c264e05f4dc60d8ed0273669/src/client153/Angelica2/Angelica3D/port/mox.cpp#L92-L155) decodifica `nf` frames completos para `m.vertices`; [`src/client153/platform/web/runtime.cpp:170-177`](https://github.com/brasalabs6/pw-web/blob/335b8006e42a15d3c264e05f4dc60d8ed0273669/src/client153/platform/web/runtime.cpp#L170-L177) usa somente `mesh.vertices[0]` na prova WebGL2.

## Hipótese e risco
Viewer retém frames não usados em cena estática; na versão jogável, animações podem precisar de todos e não devem ser simplesmente descartadas.

## Estratégia a experimentar posteriormente
Separar representação animada, metadata, janela de frames descompactados e buffers GPU; estudar compressão/streaming com keyframes e fallback de fidelidade integral.

## Testes antes de executar / critério de aceite
MOX originais com múltiplos frames; heap peak e p95 de animação/seek, hashes/frame diffs e teste de perda de contexto.

Requer antes/depois com mesma cena/dispositivo/SHA, p50/p95/p99 de frame time, custo de CPU/GPU e RAM/VRAM conforme disponíveis. Não alegar resultados sem execução.

## Rastreabilidade
- Baseline de inspeção: `335b8006e42a15d3c264e05f4dc60d8ed0273669` do [PR #23](https://github.com/brasalabs6/pw-web/pull/23).
- Épico [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md); deduplicar frente a PW23-021…037 nesta pasta.
- Nenhuma implementação, teste de FPS, mobile físico ou subagente foi executado nesta rodada de documentação.
