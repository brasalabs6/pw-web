# Findings técnicos do port PW 1.5.3 — PR #23

**Escopo:** problemas e lacunas identificados em revisões do cliente C++/WASM, preservados em arquivos rastreáveis. Esta pasta complementa (não substitui) os épicos/workorders de `.agents/epics`. **Não são GitHub Issues.**

**Nova rodada estática de performance/mobile:** 2026-10-28, baseline `6184be84d4d8766ac02f489c4867df2e0044539a` (PW23-021…037). Ver [`AUDIT-CLIENT153-PERFORMANCE-MOBILE.md`](AUDIT-CLIENT153-PERFORMANCE-MOBILE.md). Nenhum benchmark Android executado.\n\n**Snapshot da inspeção:** e0cf055be10f9bab95aa9429c90f977c5cd00c8f (2026-10-27). O agente local deve **revalidar cada issue contra o HEAD antes de implementá-la**. O status `open` não implica que a branch permaneceu sem alterações desde o snapshot.

## Fluxo de manutenção

1. Consultar índice e procurar duplicatas por ID, símbolo e arquivo. Criar novos `PW23-NNN` sequenciais; nunca reutilizar ID.
2. Classificar `P0` para blocker/corretude grave; `P1` importante; `P2` otimização/entrega. Status: `open`, `in_progress`, `resolved`, `wont_fix`, `duplicate`.
3. Registrar evidência exata, impacto, hipótese de solução, testes e critério de aceite. Não chamar de bug reproduzido quando for inferência estática.
4. Ao corrigir, atualizar o **mesmo arquivo** com `resolved_at`, commit SHA, teste nativo/WASM/Chrome/servidor, resultado e nova evidência; manter histórico.
5. Vincular ao épico/workorder existente. Não substituir roadmap nem modificar PR #17 do servidor sem coordenação; seguir `AGENTS.md` (PR único do cliente, sem self-merge).
6. Revisões futuras podem registrar novas issues **quando efetivamente executadas**; esta pasta não representa monitoramento automático em background.

## Índice de 37 issues

| ID | Prioridade | Estado | Título | Épico |
|---|---|---|---|---|
| [PW23-001](PW23-001-thread-handle-use-after-free.md) | P0 | open | CloseHandle de thread ativa pode causar use-after-free | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-002](PW23-002-winmutex-owner-recursion-infinite.md) | P0 | open | WinMutex não preserva ownership, recursão nem INFINITE | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-003](PW23-003-roleinfo-wire-v145-mismatch.md) | P0 | open | RoleInfo difere na serialização entre cliente e servidor v145 | [e10-network-transport-v1](../epics/e10-network-transport-v1/README.md) |
| [PW23-004](PW23-004-atl-com-qi-variant-bstr.md) | P0 | open | Corrigir ownership, QueryInterface, Variant e layout BSTR | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-005](PW23-005-activex-pwbrowser-stub.md) | P1 | open | PWBrowser/ActiveX continua sem host funcional | [e09-interface-input-audio-v1](../epics/e09-interface-input-audio-v1/README.md) |
| [PW23-006](PW23-006-winapi-false-success-and-pointer-state.md) | P0 | open | Shims WinAPI mascaram falhas e perdem dados de janela | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-007](PW23-007-missing-full-game-wasm-target.md) | P0 | open | Não existe target WASM que linke e inicialize ElementClient | [e04-wasm-runtime-v1](../epics/e04-wasm-runtime-v1/README.md) |
| [PW23-008](PW23-008-incomplete-tu-and-link-closure.md) | P0 | open | Fechar TUs AUI/Element/Angelica e dependências reais do link | [e04-wasm-runtime-v1](../epics/e04-wasm-runtime-v1/README.md) |
| [PW23-009](PW23-009-gnet-browser-transport-unproven.md) | P0 | open | GNET/WSS/gateway/servidor não validados ponta a ponta | [e10-network-transport-v1](../epics/e10-network-transport-v1/README.md) |
| [PW23-010](PW23-010-s3tc-required-for-bgra.md) | P1 | open | Não exigir S3TC para renderizar DDS BGRA sem compressão | [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md) |
| [PW23-011](PW23-011-input-keyboard-ime-a ui-events.md) | P1 | open | Bridge de eventos do navegador para AUI/gameplay ainda é parcial | [e09-interface-input-audio-v1](../epics/e09-interface-input-audio-v1/README.md) |
| [PW23-012](PW23-012-encoding-gbk-utf16-utf8-mismatch.md) | P1 | open | Codificações GBK/UTF-16/UTF-8 incompatíveis entre caminhos | [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md) |
| [PW23-013](PW23-013-test-gates-not-browser-game.md) | P1 | open | CI verde só valida parsers e compilações, não game no Chrome | [e16-qa-observability-v1](../epics/e16-qa-observability-v1/README.md) |
| [PW23-014](PW23-014-angelica-d3d8-webgl2-not-integrated.md) | P0 | open | Backend da Angelica original ainda não desenha os frames do jogo | [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md) |
| [PW23-015](PW23-015-web-audio-video-not-implemented.md) | P1 | open | AngelicaMedia compila mas não há saída WebAudio comprovada | [e09-interface-input-audio-v1](../epics/e09-interface-input-audio-v1/README.md) |
| [PW23-016](PW23-016-version-and-server-v145-baseline.md) | P0 | open | Revisão do código-fonte não comprovadamente idêntica ao PW 1.5.3 v145 | [e02-native-client-baseline-v1](../epics/e02-native-client-baseline-v1/README.md) |
| [PW23-017](PW23-017-stale-roadmap-and-handoff.md) | P1 | open | Roadmap, body PR e handoff registram estados antigos | [e16-qa-observability-v1](../epics/e16-qa-observability-v1/README.md) |
| [PW23-018](PW23-018-wininet-psapi-dll-seams.md) | P1 | open | Classificar WinINet/PSAPI/LoadLibrary: interfaces não são funcionalidades | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-019](PW23-019-blocking-main-loop-workers-pthreads.md) | P0 | open | Game loop, render/defence threads e waits ainda sem modelo browser validado | [e04-wasm-runtime-v1](../epics/e04-wasm-runtime-v1/README.md) |
| [PW23-020](PW23-020-assets-streaming-cache-cloudflare-pending.md) | P2 | open | Streaming/caching/Cloudflare de cliente completo continuam não implementados | [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md) |

| [PW23-021](PW23-021-webgl-preserve-drawing-buffer-mobile.md) | P1 | open | Desabilitar preserveDrawingBuffer em render interativo | [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md) |
| [PW23-022](PW23-022-skinning-allocations-every-frame.md) | P1 | open | Eliminar alocações O(vértices) por frame em AnimatedSkin | [e08-avatar-animation-v1](../epics/e08-avatar-animation-v1/README.md) |
| [PW23-023](PW23-023-dynamic-resolution-dpr-mobile.md) | P1 | open | Substituir DPR fixo por resolução dinâmica com histerese | [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md) |
| [PW23-024](PW23-024-asset-copy-amplification-wasm.md) | P1 | open | Reduzir picos de RAM durante fetch, VFS, parsers e texturas | [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md) |
| [PW23-025](PW23-025-ski-untrusted-input-memory-budget.md) | P1 | open | Impor limites globais e fuzz para SKI não confiável | [e17-security-deployment-v1](../epics/e17-security-deployment-v1/README.md) |
| [PW23-026](PW23-026-per-frame-uniform-state-and-draw-calls.md) | P2 | open | Cachear uniforms e reduzir chamadas WebGL/mesh | [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md) |
| [PW23-027](PW23-027-terrain-cache-async-streaming-visibility.md) | P1 | open | Reprojetar streaming, cache e LOD do A3DTerrain2 por budget | [e07-world-terrain-v1](../epics/e07-world-terrain-v1/README.md) |
| [PW23-028](PW23-028-mobile-texture-transcode-etc2-astc.md) | P1 | open | Pipeline de formatos comprimidos para GPUs móveis | [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md) |
| [PW23-029](PW23-029-transparent-pipeline-overdraw.md) | P1 | open | Separar passes opacos e transparentes; minimizar overdraw | [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md) |
| [PW23-030](PW23-030-gpu-skinning-and-animation-lod.md) | P1 | open | Migrar skinning massivo para GPU e criar animation LOD | [e08-avatar-animation-v1](../epics/e08-avatar-animation-v1/README.md) |
| [PW23-031](PW23-031-performance-regression-lab-mobile.md) | P1 | open | Criar laboratório de FPS, memória, energia e CI de regressão | [e16-qa-observability-v1](../epics/e16-qa-observability-v1/README.md) |
| [PW23-032](PW23-032-manifest-url-resource-trust-and-budgets.md) | P1 | open | Restringir origem, tamanho e quantidade de assets do manifest | [e17-security-deployment-v1](../epics/e17-security-deployment-v1/README.md) |
| [PW23-033](PW23-033-wasm-heap-budget-and-memory-growth.md) | P1 | open | Definir budget de WASM heap, pools, descarte e recuperação OOM | [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md) |
| [PW23-034](PW23-034-angelica-render-device-command-adapter.md) | P0 | open | Construir backend da Angelica com render queue tipada e adapter | [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md) |
| [PW23-035](PW23-035-fixed-timestep-frame-pacing-and-visibility.md) | P1 | open | Frame scheduler fixed-timestep e pacing para mobile/browser | [e04-wasm-runtime-v1](../epics/e04-wasm-runtime-v1/README.md) |
| [PW23-036](PW23-036-scene-entity-lod-culling-ps2-style.md) | P1 | open | Culling hierárquico e orçamento por entidade/efeito estilo PS2 | [e12-entities-gameplay-v1](../epics/e12-entities-gameplay-v1/README.md) |
| [PW23-037](PW23-037-skinning-normal-weight-validation.md) | P1 | open | Validar pesos, índices, normals e finitude antes do blend | [e17-security-deployment-v1](../epics/e17-security-deployment-v1/README.md) |

**Triagem sugerida:** começar por memória/lock e compatibilidade de wire (001–004), depois shims obrigatórios + link do jogo (006–009, 014, 019). Qualidade de testes e evidência (013/017) devem acompanhar cada milestone.

**Referências:** [review inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) · [review atualizado](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
