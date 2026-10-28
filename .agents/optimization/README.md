# Optimization — PW Web 1.5.3 / Angelica / Mobile

**Status: somente candidatos estáticos, adiados até o port funcional.** O objetivo imediato é compilar, carregar e rodar o cliente WASM com render original, controles e gameplay básico no desktop e em celular real.

- [Problemas do port e bloqueios](../issues/README.md): compilação, runtime, gráficos do jogo, controles, segurança impeditiva, áudio, rede.
- Esta pasta: FPS, RAM, VRAM, shaders, streaming, LOD, energia, segurança preventiva e arquitetura para o futuro.
- Os IDs legados PW23-021…037 foram migrados sem perda de conteúdo. **As prioridades históricas do frontmatter não classificam blockers atuais.** Em especial PW23-034 é arquitetura opcional; PW23-014 segue o blocker funcional da integração Angelica/WebGL2.
- Se for comprovada falha de segurança explorável ou crash impeditivo, reclassificar com reprodutor e evidências em issues; não ignorar só por estar aqui.
- Implementação de FPS somente quando houver baseline mensurável em PC e celular, mesmo SHA/cena/qualidade, medições CPU+GPU frame time, memória, draw calls e estabilidade térmica.
- Relatório: [auditoria estática Angelica](AUDIT-CLIENT153-PERFORMANCE-MOBILE.md).

## Registros históricos migrados (17)

| ID | Arquivo original | Fase |
|---|---|---|
| [PW23-021](PW23-021-webgl-preserve-drawing-buffer-mobile.md) | `issues/PW23-021-webgl-preserve-drawing-buffer-mobile.md` | pós-port |
| [PW23-022](PW23-022-skinning-allocations-every-frame.md) | `issues/PW23-022-skinning-allocations-every-frame.md` | pós-port |
| [PW23-023](PW23-023-dynamic-resolution-dpr-mobile.md) | `issues/PW23-023-dynamic-resolution-dpr-mobile.md` | pós-port |
| [PW23-024](PW23-024-asset-copy-amplification-wasm.md) | `issues/PW23-024-asset-copy-amplification-wasm.md` | pós-port |
| [PW23-025](PW23-025-ski-untrusted-input-memory-budget.md) | `issues/PW23-025-ski-untrusted-input-memory-budget.md` | pós-port |
| [PW23-026](PW23-026-per-frame-uniform-state-and-draw-calls.md) | `issues/PW23-026-per-frame-uniform-state-and-draw-calls.md` | pós-port |
| [PW23-027](PW23-027-terrain-cache-async-streaming-visibility.md) | `issues/PW23-027-terrain-cache-async-streaming-visibility.md` | pós-port |
| [PW23-028](PW23-028-mobile-texture-transcode-etc2-astc.md) | `issues/PW23-028-mobile-texture-transcode-etc2-astc.md` | pós-port |
| [PW23-029](PW23-029-transparent-pipeline-overdraw.md) | `issues/PW23-029-transparent-pipeline-overdraw.md` | pós-port |
| [PW23-030](PW23-030-gpu-skinning-and-animation-lod.md) | `issues/PW23-030-gpu-skinning-and-animation-lod.md` | pós-port |
| [PW23-031](PW23-031-performance-regression-lab-mobile.md) | `issues/PW23-031-performance-regression-lab-mobile.md` | pós-port |
| [PW23-032](PW23-032-manifest-url-resource-trust-and-budgets.md) | `issues/PW23-032-manifest-url-resource-trust-and-budgets.md` | pós-port |
| [PW23-033](PW23-033-wasm-heap-budget-and-memory-growth.md) | `issues/PW23-033-wasm-heap-budget-and-memory-growth.md` | pós-port |
| [PW23-034](PW23-034-angelica-render-device-command-adapter.md) | `issues/PW23-034-angelica-render-device-command-adapter.md` | pós-port |
| [PW23-035](PW23-035-fixed-timestep-frame-pacing-and-visibility.md) | `issues/PW23-035-fixed-timestep-frame-pacing-and-visibility.md` | pós-port |
| [PW23-036](PW23-036-scene-entity-lod-culling-ps2-style.md) | `issues/PW23-036-scene-entity-lod-culling-ps2-style.md` | pós-port |
| [PW23-037](PW23-037-skinning-normal-weight-validation.md) | `issues/PW23-037-skinning-normal-weight-validation.md` | pós-port |

## Novos candidatos

Os novos candidatos usam `OPT-NNN`, sequenciais; preservar IDs, SHA e links file:line, separar hipótese de benchmark e deduplicar com os 17 registros históricos.

## Candidatos estáticos (15)

| ID | Prioridade pós-port | Hipótese |
|---|---|---|
| [OPT-001](OPT-001-texture-manager-linear-lookup.md) | P2 | Indexar o cache de texturas da Angelica e reduzir lock contention |
| [OPT-002](OPT-002-dedup-texture-fetch-upload.md) | P1 | Compartilhar download, hash e textura GPU entre meshes |
| [OPT-003](OPT-003-pck-decode-buffer-and-inflate.md) | P2 | Reduzir overhead de buffers e zlib em leituras PCK |
| [OPT-004](OPT-004-animation-track-cache-by-device.md) | P2 | Cache de tracksets com budget por dispositivo em vez de valor fixo |
| [OPT-005](OPT-005-mox-animation-frames-resident.md) | P2 | Estudar residência parcial de frames MOX no heap |
| [OPT-006](OPT-006-tick-only-animated-textures.md) | P2 | Evitar varredura de texturas estáticas em TickAnimation |

## Rodada de investigação 2 — 2026-10-28 (6)

| ID | Prioridade após port | Área | Hipótese |
|---|---|---|---|
| [OPT-007](OPT-007-water-nearest-grid-full-scan.md) | P1 | Água / consultas espaciais | Evitar varredura completa de grids na busca de água mais próxima |
| [OPT-008](OPT-008-water-point-query-spatial-index.md) | P2 | Água / teste de imersão | Indexar consultas IsUnderWater/GetWaterHeight por célula espacial |
| [OPT-009](OPT-009-water-reflection-refraction-pass-budget.md) | P1 | Água / custo GPU | Avaliar budgets de passes de reflexão/refração de água |
| [OPT-010](OPT-010-child-skinmodel-hanger-lookup.md) | P2 | Animação / vínculos de modelo | Evitar pesquisa repetida do hanger em modelos-filho |
| [OPT-011](OPT-011-texture-precache-bounded-scheduling.md) | P2 | Texturas / cold start e GPU | Substituir precache de todas as texturas por aquecimento incremental medido |
| [OPT-012](OPT-012-pck-path-routing-bounds-and-prefix.md) | P1 | PCK / lookup e hardening preventivo | Auditar limites de caminho e roteamento por prefixo no AFilePackMan |

## Rodada incremental — 2026-10-28 (3 novos)

| ID | Prioridade pós-port | Área | Hipótese |
|---|---|---|---|
| [OPT-013](OPT-013-water-area-load-dimension-wave-budgets.md) | P1 | Water area binary deserialization | Validar dimensões, contagem de ondas e orçamento de alocação de A3DWaterArea |
| [OPT-014](OPT-014-dds-mipmap-repacking-allocation-bounds.md) | P1 | Legacy Angelica DDS loading | Auditar repacking de mipmaps DDS e ownership de buffer temporário |
| [OPT-015](OPT-015-terrain-render-slot-lookup-reuse.md) | P2 | Terrain render-slot CPU work | Medir buscas lineares e sincronização quadrática dos render slots do terreno |

## Protocolo

1. Verificar HEAD do PR #23, não usar SHA presumido; confirmar status do port funcional.
2. Procurar evidência estática em Angelica, PCK/DDS/MOX, render, animação e mobile; registrar novos candidatos aqui.
3. Não implementar otimizações antes do game rodar. Se algum risco virar blocker real, classificar em `issues` com teste.
4. Depois do primeiro benchmark real, priorizar pelos ganhos por ms/MB e risco, sempre com before/after em hardware real.
5. Não criar terceiro PR nem tocar servidor PR #17; apenas documentação nesta auditoria.
