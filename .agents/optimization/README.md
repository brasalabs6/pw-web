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


## Rodada dedicada — combate, skills, PvP e multidões (13 candidatos, 2026-10-28)

[Relatório detalhado da arquitetura e matriz de benchmarks](AUDIT-COMBAT-SKILLS-CROWD-2026-10-28.md). Todos os itens abaixo são `post-browser-port`, não bloqueiam o funcionamento do game.

| ID | Prioridade pós-port | Subsistema | Hipótese |
|---|---|---|---|
| [OPT-016](OPT-016-combat-gfx-stop-cache-retention-60000s.md) | P1 | Efeitos — lifetime e memória | Reduzir retenção potencial de GFX de impacto parados durante guerras |
| [OPT-017](OPT-017-aoe-skill-target-projectile-fanout.md) | P1 | Skills AoE — fanout de eventos | Evitar explosão multiplicativa de eventos visuais de skills AoE |
| [OPT-018](OPT-018-skill-gfx-event-pool-active-capacity.md) | P1 | Skills — event pool / alocação | Orçamento de capacidade e reciclagem do pool de eventos SkillGfx |
| [OPT-019](OPT-019-offscreen-gfx-simulation-vs-render.md) | P1 | VFX — CPU fora de tela | Desacoplar simulação de efeitos invisíveis do registro para render |
| [OPT-020](OPT-020-skill-gfx-frustum-screen-space-culling.md) | P1 | VFX — GPU fillrate e visibilidade | Complementar culling de VFX por raio com frustum e tamanho projetado |
| [OPT-021](OPT-021-crowd-player-tick-simulation-vs-presentation.md) | P1 | Crowd — scheduler CPU | Separar atualização de combate de apresentação de centenas de jogadores |
| [OPT-022](OPT-022-crowd-per-player-shadow-submission.md) | P1 | Crowd — sombras GPU | Reduzir submissão de sombras por personagem sem afetar combate |
| [OPT-023](OPT-023-crowd-combat-nameplate-ui-overdraw.md) | P2 | Crowd — UI e text | Orçamento de nomes, barras e textos de combate em multidões |
| [OPT-024](OPT-024-skill-composer-eager-load-startup.md) | P2 | Skills — cold-start e memória | Avaliar eager load de compositores de skills e cache lazy por cena |
| [OPT-025](OPT-025-skill-gfx-priority-policy-vs-binary-hide.md) | P1 | Combate — política de relevância | Evoluir flags binárias de GFX para orçamento adaptativo sem ocultar telegraphs |
| [OPT-026](OPT-026-per-projectile-target-bone-hook-lookup.md) | P1 | Skills — follow target | Cache de posição/hook por entidade e frame para projéteis de skill |
| [OPT-027](OPT-027-multisection-skill-composer-linear-find.md) | P2 | Skills — lookup de compositor | Indexar compositores de skills multietapa por (skill,section) |
| [OPT-028](OPT-028-crowd-target-picking-raytrace-linear-scan.md) | P2 | Interação — alvo/ray tracing | Acelerar seleção de alvo em guerras com índice espacial de jogadores |


## Rodada incremental — loading, PCK/PKX, compressão e tamanho de assets (2026-10-28; 10 candidatos)

[Relatório dedicado: AUDIT-ASSET-LOADING-COMPRESSION-2026-10-28.md](AUDIT-ASSET-LOADING-COMPRESSION-2026-10-28.md). Os itens abaixo são candidatos documentais **pós-port**, não bloqueios comprovados.

| ID | Prioridade pós-port | Subsistema | Hipótese |
|---|---|---|---|
| [OPT-029](OPT-029-pck-entry-size-compression-inventory.md) | P1 | Inventory / bytes / codecs | Medir peso e compressibilidade de cada asset dentro dos PCK/PKX |
| [OPT-030](OPT-030-viewer-scene-stage-select-only-used-dds.md) | P1 | Staging / tamanho de bundle de demonstração | Evitar extração de DDS irmãs não referenciadas pela cena MOX |
| [OPT-031](OPT-031-pck-catalog-open-many-small-reads.md) | P1 | PCK catalog / load latency | Reduzir custo de abertura do índice PCK com milhares de entradas |
| [OPT-032](OPT-032-game-pck-shared-open-redecode-lifetime.md) | P2 | PCK / repetição de IO e decompression | Avaliar reabertura e descompressão repetida de arquivos compartilhados no AFilePackGame |
| [OPT-033](OPT-033-browser-dependency-fetch-priority-pipelining.md) | P1 | Browser / network waterfall / first frame | Pipelinar dependências da cena Web com concorrência e prioridades limitadas |
| [OPT-034](OPT-034-pck-codec-policy-by-content-type.md) | P2 | Compression / CDN cooker | Comparar compressão seletiva por tipo de asset sem alterar assets originais |
| [OPT-035](OPT-035-scene-block-loading-byte-time-budget.md) | P1 | World/scene / hitch / loading progress | Substituir política de loading de cena baseada só em número de blocos |
| [OPT-036](OPT-036-pck-read-short-read-integrity-contract.md) | P1 | PCK / hardening preventivo | Verificar leitura completa e limites de payload no leitor PCK legado |
| [OPT-037](OPT-037-pck-replacement-append-fragmentation.md) | P2 | PCK / tamanho de release | Evitar crescimento histórico de PCK por substituições append-only no pipeline de build |
| [OPT-038](OPT-038-asset-inventory-non-pck-coverage.md) | P2 | Inventory / scope/coverage | Incluir assets fora de element/*.pck na análise de peso e prioridades de download |

## Auditoria incremental — GFX original (2026-10-28; 1 candidato)

| ID | Prioridade pós-port | Subsistema | Hipótese |
|---|---|---|---|
| [OPT-039](OPT-039-gfx-shared-texture-cache-lru-and-locked-load.md) | P2 | GFX / cache de textura original | Medir cache LRU de 16 entradas, duas aquisições de referência e mutex durante segunda carga; preservar refcounts |

Baseline de inspeção: `4bd108b30650647d1c510db252442de764b27dd1`. Candidato estático, sem benchmark e sem bloqueio do port; distinto de OPT-001/002/011/016. O port WebAssembly jogável em desktop e mobile continua sendo o gate anterior a qualquer tuning.

## Auditoria incremental — A2D sprite batching (2026-10-28; 1 candidato)

| ID | Prioridade pós-port | Subsistema | Hipótese |
|---|---|---|---|
| [OPT-040](OPT-040-sprite-buffer-batch-capacity-and-flush.md) | P2 | A2D SpriteBuffer / UI WebGL2 | Medir limite de lote de sprites, custo de lock/cópia/draw e validar possível excedente de quads em um slot; inspeção estática, não bug reproduzido |

Inspeção no SHA `c86bb95a0cb37d45d31b81a6c7a27d3afe1f6a24`. Distinto de PW23-026 (viewer WebGL2), OPT-023 (placas em multidões) e OPT-001 (cache de texturas). Sem benchmark, teste mobile ou alteração de código. Port funcional continua prioritário.

## Auditoria incremental — A3DVertexCollector (2026-10-28; 1 candidato)

| ID | Prioridade pós-port | Subsistema | Hipótese |
|---|---|---|---|
| [OPT-041](OPT-041-vertex-collector-single-batch-capacity-bounds.md) | P1 | Angelica / buffers de geometria | Verificar capacidade de um push individual antes de copiar vértices e índices; hardening estático sem reprodução |

Inspeção no SHA `d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8`. Distinto de OPT-040 e OPT-015; somente pós-port, sem benchmark, testes mobile ou alteração de código.

## Auditoria incremental — loader binário de cenas (2026-10-28; 1 candidato)

| ID | Prioridade pós-port | Subsistema | Hipótese |
|---|---|---|---|
| [OPT-042](OPT-042-scene-binary-counts-reads-and-string-bounds.md) | P1 | Angelica / A3DScene binary loader | Validar contagens, multiplicações, leituras completas e limites de strings antes de alocar/parsear cenas |

Inspeção estática no SHA `97dfe0d423494ee35178c082c518a66659cfd0d2`. Diferente de OPT-013 (água), OPT-014 (DDS) e OPT-036 (PCK). Sem crash reproduzido, benchmark, teste mobile ou mudança de código; somente fase post-browser-port. IDs históricos PW23-021…037 preservados.

## Auditoria incremental — horizon culling Angelica (2026-10-28; 1 candidato)

| ID | Prioridade pós-port | Subsistema | Hipótese |
|---|---|---|---|
| [OPT-043](OPT-043-horizon-culling-screen-column-scan.md) | P2 | Angelica / A3DHorizonCull / CPU-WASM | Medir varredura por coluna de tela e custo/benefício do culling; verificar segmentos degenerados após port funcional |

Inspeção estática no SHA `c4eb8fe8cb8262a217bdae69789174cae334db64`. Distinto de PW23-036 (LOD/culling geral) e OPT-015 (render slots). Sem benchmark, teste mobile ou correção de código. Preservados os IDs PW23-021…037; execução do jogo no navegador segue prioritária.
