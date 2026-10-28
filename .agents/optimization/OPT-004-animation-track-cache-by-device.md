---
id: OPT-004
title: "Cache de tracksets com budget por dispositivo em vez de valor fixo"
priority: P2
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 335b8006e42a15d3c264e05f4dc60d8ed0273669
first_observed: 2026-10-28
validation: static-only
epic: e08-avatar-animation-v1
---

# OPT-004 — Cache de tracksets com budget por dispositivo em vez de valor fixo

**Registro de otimização adiada:** não implementar antes do cliente WASM executar o jogo em browser desktop e mobile e possuir baseline de FPS/memória. Se um problema se comprovar blocker, promover a `.agents/issues` com reprodutor.

## Evidência no código
[`src/client153/Angelica2/Angelica3D/Source/A3DTrackMan.cpp:365-409`](https://github.com/brasalabs6/pw-web/blob/335b8006e42a15d3c264e05f4dc60d8ed0273669/src/client153/Angelica2/Angelica3D/Source/A3DTrackMan.cpp#L365-L409) inicializa `m_ActCache` com limites históricos 16 MiB/14 MiB, sem perfil de device.

## Hipótese e risco
Um limite histórico único pode ser alto para mobile low-RAM ou baixo em cenários densos; medir taxa de cache hit é obrigatório antes de mudar.

## Estratégia a experimentar posteriormente
Instrumentar bytes/hits/misses e experimentos com cache tiers, LRU respeitando refcount, prefetch de ações visíveis e comportamento estável sob pressão.

## Testes antes de executar / critério de aceite
10/50/100 avatares, cold/warm zone transitions, miss rate, frame hitches e pose correta em dispositivos diferentes.

Requer antes/depois com mesma cena/dispositivo/SHA, p50/p95/p99 de frame time, custo de CPU/GPU e RAM/VRAM conforme disponíveis. Não alegar resultados sem execução.

## Rastreabilidade
- Baseline de inspeção: `335b8006e42a15d3c264e05f4dc60d8ed0273669` do [PR #23](https://github.com/brasalabs6/pw-web/pull/23).
- Épico [e08-avatar-animation-v1](../epics/e08-avatar-animation-v1/README.md); deduplicar frente a PW23-021…037 nesta pasta.
- Nenhuma implementação, teste de FPS, mobile físico ou subagente foi executado nesta rodada de documentação.
