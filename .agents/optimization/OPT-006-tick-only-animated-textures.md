---
id: OPT-006
title: "Evitar varredura de texturas estáticas em TickAnimation"
priority: P2
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 335b8006e42a15d3c264e05f4dc60d8ed0273669
first_observed: 2026-10-28
validation: static-only
epic: e06-render-webgl2-v1
---

# OPT-006 — Evitar varredura de texturas estáticas em TickAnimation

**Registro de otimização adiada:** não implementar antes do cliente WASM executar o jogo em browser desktop e mobile e possuir baseline de FPS/memória. Se um problema se comprovar blocker, promover a `.agents/issues` com reprodutor.

## Evidência no código
[`src/client153/Angelica2/Angelica3D/Source/A3DTextureMan.cpp:262-272`](https://github.com/brasalabs6/pw-web/blob/335b8006e42a15d3c264e05f4dc60d8ed0273669/src/client153/Angelica2/Angelica3D/Source/A3DTextureMan.cpp#L262-L272) percorre `m_ListTexture` inteira sob mutex e chama `TickAnimation()` em cada item. Não foi ainda comprovado quantas vezes é chamada no loop de jogo.

## Hipótese e risco
Potencial O(N) por tick mesmo com materiais estáticos, mas pode ser irrelevante se a chamada for infrequente.

## Estratégia a experimentar posteriormente
Perfil do call graph; caso relevante, manter sublista de texturas animadas, com registro/desregistro e refcount seguro, ou atualização de frequência por tipo de shader.

## Testes antes de executar / critério de aceite
1k/10k texturas estáticas e 10 animadas; calls/frame, CPU, lock hold, testes de equivalência e aging.

Requer antes/depois com mesma cena/dispositivo/SHA, p50/p95/p99 de frame time, custo de CPU/GPU e RAM/VRAM conforme disponíveis. Não alegar resultados sem execução.

## Rastreabilidade
- Baseline de inspeção: `335b8006e42a15d3c264e05f4dc60d8ed0273669` do [PR #23](https://github.com/brasalabs6/pw-web/pull/23).
- Épico [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md); deduplicar frente a PW23-021…037 nesta pasta.
- Nenhuma implementação, teste de FPS, mobile físico ou subagente foi executado nesta rodada de documentação.
