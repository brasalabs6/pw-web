---
id: OPT-041
title: "Validação de tamanho de lote individual no A3DVertexCollector"
priority: P1
status: candidate
kind: static-hardening
phase: post-browser-port
blocking: false
source_sha: d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8
first_observed: 2026-10-28
validation: static-only
epic: e06-render-webgl2-v1
scope: PR-23
---

# OPT-041 — Limites por chamada no A3DVertexCollector

**Somente pós-browser-port.** Análise estática do cliente Angelica 1.5.3 no SHA `d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8`; não houve compilação local, benchmark, fuzzing, reprodução de defeito ou teste em aparelho móvel. Não fazer tuning antes do jogo funcionar em navegador PC e mobile.

## Evidências

- [`A3DVertexCollector.h:54-60`](https://github.com/brasalabs6/pw-web/blob/d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8/src/client153/Angelica2/Output/a3dsdk/include/A3DVertexCollector.h#L54-L60) fixa os limites do stream em 256 vértices e 1.024 índices.
- [`A3DVertexCollector.cpp:275-285`](https://github.com/brasalabs6/pw-web/blob/d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8/src/client153/Angelica2/Angelica3D/Source/A3DVertexCollector.cpp#L275-L285) aloca o stream com essas capacidades.
- [`A3DVertexCollector.cpp:342-363`](https://github.com/brasalabs6/pw-web/blob/d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8/src/client153/Angelica2/Angelica3D/Source/A3DVertexCollector.cpp#L342-L363) dispara um flush se o acumulado exceder o limite, mas não verifica que **uma única chamada** tenha no máximo 256 vértices/1.024 índices nem rejeita contagens negativas.
- [`A3DVertexCollector.cpp:400-425`](https://github.com/brasalabs6/pw-web/blob/d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8/src/client153/Angelica2/Angelica3D/Source/A3DVertexCollector.cpp#L400-L425) copia `iSize * iNumVert` bytes e escreve `iNumIdx` índices após esse flush.

## Hipótese e impacto

Se algum caller fornecer lote individual maior que o stream, o trecho visível não garante os limites antes de `memcpy` e das escritas de índices. Isso sugere risco de corrupção de buffer e estabilidade; a alcançabilidade real depende dos callers e da implementação de `A3DStream::Lock*`. O flush/lock/cópia também merece medição de custo CPU/WASM↔WebGL2. **Não é vulnerabilidade crítica reproduzida, blocker comprovado nem medição de FPS, RAM/VRAM ou energia.**

## Plano de benchmark e validação pós-port

1. Primeiro comprovar boot, render Angelica, controles, login e gameplay no browser desktop e mobile, e obter baseline por SHA.
2. Inspecionar todos os callers e contratos de lock/capacidade. Criar fixtures controlados com 255/256/257/512 vértices, 1023/1024/1025 índices e contagens negativas, executando ASan/UBSan nativo e testes WASM de limites.
3. Se confirmado, comparar rejeição de lote vs segmentação, preservando topologia, ordenação, transparência e screenshots/golden frames.
4. Medir frame time CPU/GPU p50/p95/p99, draw calls, upload, alocações, RAM/VRAM observável e energia/temperatura em dispositivos reais com cena, DPR e qualidade iguais. Nenhum ganho é presumido.

**Aceite:** sem acesso fora dos buffers, sem perda de geometria e sem regressão visual; qualquer ganho exige benchmark antes/depois. Se surgir crash impeditivo ou vulnerabilidade crítica reproduzida, reclassificar em `.agents/issues` com evidência e deduplicação.

## Deduplicação

Distinto de [OPT-040](OPT-040-sprite-buffer-batch-capacity-and-flush.md) (lotes de quads A2D), [OPT-015](OPT-015-terrain-render-slot-lookup-reuse.md) (busca de slots do terreno), [PW23-026](PW23-026-per-frame-uniform-state-and-draw-calls.md) (viewer WebGL2) e [PW23-037](PW23-037-skinning-normal-weight-validation.md) (pesos). Este registro trata especificamente de uma chamada de `A3DVertexCollector::PushVertices`.

Não alterar fonte, servidor #17, PRs ou merge.
