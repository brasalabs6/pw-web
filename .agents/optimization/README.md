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

Os próximos candidatos usam `OPT-NNN`, sequenciais; novas inspeções devem preservar IDs, SHA e link file:line, separar hipótese de benchmark e deduplicar com os 17 arquivos acima.

## Protocolo

1. Verificar HEAD do PR #23, não usar SHA presumido; confirmar status do port funcional.
2. Procurar evidência estática em Angelica, PCK/DDS/MOX, render, animação e mobile; registrar novos candidatos aqui.
3. Não implementar otimizações antes do game rodar. Se algum risco virar blocker real, classificar em `issues` com teste.
4. Depois do primeiro benchmark real, priorizar pelos ganhos por ms/MB e risco, sempre com before/after em hardware real.
5. Não criar terceiro PR nem tocar servidor PR #17; apenas documentação nesta auditoria.
