---
id: OPT-042
title: "Validar contagens, leituras e strings do loader binário A3DScene"
priority: P1
status: candidate
kind: static-hardening
phase: post-browser-port
blocking: false
source_sha: 97dfe0d423494ee35178c082c518a66659cfd0d2
first_observed: 2026-10-28
validation: static-only
epic: e07-world-terrain-v1
scope: PR-23
---

# OPT-042 — Limites de memória e integridade no loader binário A3DScene

**Fase: post-browser-port.** Inspeção estática da Angelica original no SHA `97dfe0d423494ee35178c082c518a66659cfd0d2`, sem execução, fuzzing, benchmark, teste WebGL2 ou celular. Prioridade do projeto permanece compilar e executar o cliente completo com render, input e gameplay em navegador PC/mobile. A inclusão da TU no target de objetos não comprova que esta rota de carregamento seja alcançável no jogo.

## Evidência exata

- [`A3DScene.cpp:156-180`](https://github.com/brasalabs6/pw-web/blob/97dfe0d423494ee35178c082c518a66659cfd0d2/src/client153/Angelica2/Angelica3D/Source/A3DScene.cpp#L156-L180): `Load` lê `m_nNumTextures` do arquivo, multiplica por `sizeof(TEXTURE_RECORD)` para alocar e ler sem checagem explícita de contagem negativa, limite máximo, overflow da multiplicação ou número exato de bytes retornados.
- [`A3DScene.cpp:181-203`](https://github.com/brasalabs6/pw-web/blob/97dfe0d423494ee35178c082c518a66659cfd0d2/src/client153/Angelica2/Angelica3D/Source/A3DScene.cpp#L181-L203): usa `strlen` em nomes provenientes do registro binário e aloca `nFaceNum * 3 * sizeof(A3DVERTEX)` por textura sem validar explicitamente o limite e a terminação das strings no trecho.
- [`A3DScene.cpp:225-251`](https://github.com/brasalabs6/pw-web/blob/97dfe0d423494ee35178c082c518a66659cfd0d2/src/client153/Angelica2/Angelica3D/Source/A3DScene.cpp#L225-L251): lê `m_nNumFaces`, aloca três buffers derivados da contagem e realiza leituras subsequentes sem conferir o comprimento efetivamente lido neste método.

## Hipótese, impacto e incertezas

Arquivos de cena truncados ou com contagens fora do formato esperado podem solicitar memória excessiva, provocar overflow de cálculo, leituras de strings além do campo ou estado parcial inconsistente. O impacto potencial é pico de heap WASM, travamento durante loading e robustez da cadeia de assets; **não há OOM, crash, exploração ou regressão do port reproduzidos**. É necessário conferir os contratos internos de `AFile::Read`, os tamanhos reais dos registros e a alcançabilidade do loader antes de concluir severidade.

## Plano de validação e benchmark — somente após port funcional

1. Estabelecer baseline com SHA, cena, assets autorizados, navegador e dispositivos PC/Android/iOS reais; medir tempo de loading p50/p95/p99, bytes processados, alocações, pico de heap WASM, RAM/VRAM observável, frame time CPU/GPU, energia/temperatura e integridade visual.
2. Usar corpus legítimo de cenas para determinar limites de contagens, faces e nomes. Em fixtures isolados, variar contagens em -1/0/limites/INT_MAX, truncar leituras em cada campo, remover terminador NUL dos nomes e injetar falhas de alocação; rodar ASan/UBSan nativo e checks WASM.
3. Se necessário, propor validação transacional de contagens e bytes restantes, multiplicação checked em `size_t`, checagem de string com comprimento limitado ao campo, limites de memória por cena e rollback de alocações parciais, preservando compatibilidade dos assets originais.
4. Comparar before/after na mesma cena, SHA de build, qualidade e dispositivo; exigir equivalência de conteúdo e ausência de regressão de carregamento. Não pressupor ganhos de FPS, RAM/VRAM ou bateria.

## Deduplicação e classificação

Distinto de [OPT-013](OPT-013-water-area-load-dimension-wave-budgets.md) (formato de áreas de água), [OPT-014](OPT-014-dds-mipmap-repacking-allocation-bounds.md) (DDS), [OPT-036](OPT-036-pck-read-short-read-integrity-contract.md) (leitor de contêiner PCK), [PW23-025](PW23-025-ski-untrusted-input-memory-budget.md) (SKI) e [PW23-033](PW23-033-wasm-heap-budget-and-memory-growth.md) (orçamento global). Este registro cobre exclusivamente `A3DScene::Load` e seu formato binário de cena.

Se for demonstrado crash impeditivo ou vulnerabilidade crítica, abrir/reclassificar em `.agents/issues` com reprodutor e evidência, sem tratar como tuning. Sem mudança de código-fonte, terceiro PR, alteração do servidor #17 ou auto-merge.
