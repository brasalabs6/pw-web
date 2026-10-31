---
id: OPT-001
title: "Indexar o cache de texturas da Angelica e reduzir lock contention"
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

# OPT-001 — Indexar o cache de texturas da Angelica e reduzir lock contention

**Registro de otimização adiada:** não implementar antes do cliente WASM executar o jogo em browser desktop e mobile e possuir baseline de FPS/memória. Se um problema se comprovar blocker, promover a `.agents/issues` com reprodutor.

## Evidência no código
[`src/client153/Angelica2/Angelica3D/Source/A3DTextureMan.cpp:80-95`](https://github.com/brasalabs6/pw-web/blob/335b8006e42a15d3c264e05f4dc60d8ed0273669/src/client153/Angelica2/Angelica3D/Source/A3DTextureMan.cpp#L80-L95) mostra `FindTexture` percorrendo uma lista encadeada por arquivo com `stricmp` sob mutex; [`src/client153/Angelica2/Angelica3D/Source/A3DTextureMan.cpp:104-120`](https://github.com/brasalabs6/pw-web/blob/335b8006e42a15d3c264e05f4dc60d8ed0273669/src/client153/Angelica2/Angelica3D/Source/A3DTextureMan.cpp#L104-L120) faz essa procura ao carregar texturas.

## Hipótese e risco
A complexidade O(N) por consulta e lock podem pesar em zonas com milhares de materiais; gargalo ainda não comprovado.

## Estratégia a experimentar posteriormente
Experimentar hash map de chave normalizada (caminho/formato/flags relevantes) preservando semântica de lista/refcount, IDs e case folding GBK; medir custo de memória e chaves colididas.

## Testes antes de executar / critério de aceite
Comparação 100/1.000/10.000 texturas, p95 lookup, contenção de lock, refcount e sanitizers; aprovação só se ganho real sem asset trocado.

Requer antes/depois com mesma cena/dispositivo/SHA, p50/p95/p99 de frame time, custo de CPU/GPU e RAM/VRAM conforme disponíveis. Não alegar resultados sem execução.

## Rastreabilidade
- Baseline de inspeção: `335b8006e42a15d3c264e05f4dc60d8ed0273669` do [PR #23](https://github.com/brasalabs6/pw-web/pull/23).
- Épico [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md); deduplicar frente a PW23-021…037 nesta pasta.
- Nenhuma implementação, teste de FPS, mobile físico ou subagente foi executado nesta rodada de documentação.
