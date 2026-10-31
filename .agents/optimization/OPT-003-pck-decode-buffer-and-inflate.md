---
id: OPT-003
title: "Reduzir overhead de buffers e zlib em leituras PCK"
priority: P2
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 335b8006e42a15d3c264e05f4dc60d8ed0273669
first_observed: 2026-10-28
validation: static-only
epic: e05-asset-pipeline-v1
---

# OPT-003 — Reduzir overhead de buffers e zlib em leituras PCK

**Registro de otimização adiada:** não implementar antes do cliente WASM executar o jogo em browser desktop e mobile e possuir baseline de FPS/memória. Se um problema se comprovar blocker, promover a `.agents/issues` com reprodutor.

## Evidência no código
[`src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp:686-718`](https://github.com/brasalabs6/pw-web/blob/335b8006e42a15d3c264e05f4dc60d8ed0273669/src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp#L686-L718) aloca staging comprimido por leitura; [`src/client153/Angelica2/AngelicaFile/Source/afilepackage.cpp:122-152`](https://github.com/brasalabs6/pw-web/blob/335b8006e42a15d3c264e05f4dc60d8ed0273669/src/client153/Angelica2/AngelicaFile/Source/afilepackage.cpp#L122-L152) cria e destrói stream zlib por `UnCompress`.

## Hipótese e risco
Possível overhead por arquivo pequeno no streaming e crescimento de alocações; ainda sem medição; nem toda descompressão se beneficia de reuso de stream.

## Estratégia a experimentar posteriormente
Primeiro medir; avaliar buffer pool limitado e filas de decode em Web Workers/threads disponíveis, sem bloquear render e sem mudar criptografia, validação de tamanho ou formato. Reuso zlib só com testes de equivalência.

## Testes antes de executar / critério de aceite
Fixtures de PCK pequenas/médias/grandes; medir p50/p95, allocs, memória pico e SHA do conteúdo; testes de dados comprimidos inválidos.

Requer antes/depois com mesma cena/dispositivo/SHA, p50/p95/p99 de frame time, custo de CPU/GPU e RAM/VRAM conforme disponíveis. Não alegar resultados sem execução.

## Rastreabilidade
- Baseline de inspeção: `335b8006e42a15d3c264e05f4dc60d8ed0273669` do [PR #23](https://github.com/brasalabs6/pw-web/pull/23).
- Épico [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md); deduplicar frente a PW23-021…037 nesta pasta.
- Nenhuma implementação, teste de FPS, mobile físico ou subagente foi executado nesta rodada de documentação.
