---
id: OPT-002
title: "Compartilhar download, hash e textura GPU entre meshes"
priority: P1
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 335b8006e42a15d3c264e05f4dc60d8ed0273669
first_observed: 2026-10-28
validation: static-only
epic: e05-asset-pipeline-v1
---

# OPT-002 — Compartilhar download, hash e textura GPU entre meshes

**Registro de otimização adiada:** não implementar antes do cliente WASM executar o jogo em browser desktop e mobile e possuir baseline de FPS/memória. Se um problema se comprovar blocker, promover a `.agents/issues` com reprodutor.

## Evidência no código
[`src/client153/platform/web/runtime.cpp:187-205`](https://github.com/brasalabs6/pw-web/blob/335b8006e42a15d3c264e05f4dc60d8ed0273669/src/client153/platform/web/runtime.cpp#L187-L205) cria `glGenTextures` uma vez por mesh. [`web/client153/app.mjs:30-36`](https://github.com/brasalabs6/pw-web/blob/335b8006e42a15d3c264e05f4dc60d8ed0273669/web/client153/app.mjs#L30-L36) consulta e baixa `scene.textures[name]` a cada mesh, calcula SHA e chama upload sem deduplicação inter-mesh.

## Hipótese e risco
Em modelos com materiais repetidos pode haver tráfego/upload/VRAM duplicados. Isso difere de PW23-024, que trata cópias do mesmo fluxo dentro de JS/WASM/VFS.

## Estratégia a experimentar posteriormente
Cache de recursos por SHA+sampler/format/mips, coalescing das promessas fetch e textura GL compartilhada por refcount; preservar materiais e liberar após último usuário.

## Testes antes de executar / critério de aceite
Cena 20 meshes com a mesma DDS comparada com 20 DDS diferentes: downloads, texturas GL, alocações GPU e screenshot diff.

Requer antes/depois com mesma cena/dispositivo/SHA, p50/p95/p99 de frame time, custo de CPU/GPU e RAM/VRAM conforme disponíveis. Não alegar resultados sem execução.

## Rastreabilidade
- Baseline de inspeção: `335b8006e42a15d3c264e05f4dc60d8ed0273669` do [PR #23](https://github.com/brasalabs6/pw-web/pull/23).
- Épico [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md); deduplicar frente a PW23-021…037 nesta pasta.
- Nenhuma implementação, teste de FPS, mobile físico ou subagente foi executado nesta rodada de documentação.
