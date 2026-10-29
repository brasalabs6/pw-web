---
id: OPT-047
title: "Medir churn de VBO/IBO temporários por Draw*PrimitiveUP no adaptador D3D8"
priority: P2
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 982ad80682ba866a175ce1e75051a14a0b178ede
first_observed: 2026-10-29
validation: static-only
epic: e06-render-webgl2-v1
scope: pr-23
---

# OPT-047 — Buffers GPU temporários por chamada Draw*PrimitiveUP

**Escopo:** hipótese estática de CPU/driver/GPU e energia, **somente pós-browser-port**. O cliente Angelica original ainda não comprovou frames de gameplay no Chrome nem no celular. Não executar tuning enquanto os blockers de render, assets e entrada persistirem.

## Evidência imutável

- [`d3d8_device.cpp:242–255`](https://github.com/brasalabs6/pw-web/blob/982ad80682ba866a175ce1e75051a14a0b178ede/src/client153/platform/web/d3d8_device.cpp#L242-L255): cada `DrawPrimitiveUP` cria VBO com `glGenBuffers`, transfere `count * stride` via `glBufferData`, desenha e exclui com `glDeleteBuffers`.
- [`d3d8_device.cpp:256–274`](https://github.com/brasalabs6/pw-web/blob/982ad80682ba866a175ce1e75051a14a0b178ede/src/client153/platform/web/d3d8_device.cpp#L256-L274): cada `DrawIndexedPrimitiveUP` cria **dois** buffers, envia vértices e índices, desenha e exclui ambos no mesmo caminho.

## Impacto hipotético e limites

Caso o cliente use essas APIs muitas vezes por frame (UI, sprites, efeitos ou malhas pequenas), criar/excluir recursos GL e reenviar buffers repetidamente **pode** aumentar chamadas WebGL, custo de CPU/WASM→GL, tráfego e p95/p99 de frametime; buffers temporários também podem pressionar memória/VRAM e consumo energético em mobile. **Não há contagem real de chamadas, benchmark, uso de RAM/VRAM, ganho de FPS ou teste em dispositivo.** A semântica de índices/FVF e a renderização correta são gates de port separados em PW23-014; não atribuir eventuais pixels ausentes a esta hipótese de performance.

## Plano de benchmark — depois do port funcional

1. Instrumentar contagens de `DrawPrimitiveUP` e `DrawIndexedPrimitiveUP` por frame, vértices/índices e bytes enviados, número de `glGenBuffers/glDeleteBuffers/glBufferData`, draw calls e erro/context-loss. Classificar chamadas por subsistema da Angelica.
2. No **mesmo SHA/cena/qualidade/resolução** do jogo funcional, coletar CPU frame time p50/p95/p99, GPU time quando suportado, 1% low, pico de RAM WASM/VRAM estimada, alocações e hitches. Repetir no Chrome desktop e em Android físico, com medição térmica/energia apenas quando instrumentada.
3. Se o churn for relevante, comparar implementação de buffers transitórios reutilizáveis/ring buffer e política de orphaning, preservando `Draw*UP`, stride, tipos de índice, lifetime dos dados e resultados visuais. Capturar screenshots/golden frames, estado GL, perda de contexto e validação de primitivos antes/depois.
4. Aceitar mudança somente se houver ganho reproduzível sem regressão visual ou funcional; não modificar código agora.

## Deduplicação

Distinto de [PW23-026](PW23-026-per-frame-uniform-state-and-draw-calls.md) (viewer WebGL2 e estado por frame), [OPT-040](OPT-040-sprite-buffer-batch-capacity-and-flush.md) (buffering do sprite original), [OPT-045](OPT-045-d3d8-lockrect-full-texture-repack-upload.md) (texturas) e [PW23-014](../issues/PW23-014-angelica-d3d8-webgl2-not-integrated.md) (corretude do backend). Este candidato é exclusivamente sobre **lifetime de VBO/IBO nas novas rotas D3D8 UP**.

Preservar PW23-021…037. Nenhum benchmark ou teste mobile executado; nenhum código-fonte alterado. Não criar outro PR, não tocar servidor #17 e não fazer auto-merge.
