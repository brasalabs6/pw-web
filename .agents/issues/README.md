# Findings técnicos do port PW 1.5.3 — PR #23

**Escopo:** problemas e lacunas identificados em revisões do cliente C++/WASM, preservados em arquivos rastreáveis. Esta pasta complementa (não substitui) os épicos/workorders de `.agents/epics`. **Não são GitHub Issues.**

**Otimização separada:** [`.agents/optimization`](../optimization/README.md). Os registros PW23-021…037 foram movidos para que esta pasta priorize o port WebAssembly e funcionamento inicial no navegador.

**Snapshot da inspeção:** e0cf055be10f9bab95aa9429c90f977c5cd00c8f (2026-10-27). O agente local deve **revalidar cada issue contra o HEAD antes de implementá-la**. O status `open` não implica que a branch permaneceu sem alterações desde o snapshot.

## Fluxo de manutenção

1. Consultar índice e procurar duplicatas por ID, símbolo e arquivo. Criar novos `PW23-NNN` sequenciais; nunca reutilizar ID.
2. Classificar `P0` para blocker/corretude grave, `P1` para problemas importantes de port e `P2` para lacunas não impeditivas. **Otimizações opcionais são documentadas em `../optimization`.** Status: `open`, `in_progress`, `resolved`, `wont_fix`, `duplicate`.
3. Registrar evidência exata, impacto, hipótese de solução, testes e critério de aceite. Não chamar de bug reproduzido quando for inferência estática.
4. Ao corrigir, atualizar o **mesmo arquivo** com `resolved_at`, commit SHA, teste nativo/WASM/Chrome/servidor, resultado e nova evidência; manter histórico.
5. Vincular ao épico/workorder existente. Não substituir roadmap nem modificar PR #17 do servidor sem coordenação; seguir `AGENTS.md` (PR único do cliente, sem self-merge).
6. Revisões futuras podem registrar novas issues **quando efetivamente executadas**; esta pasta não representa monitoramento automático em background.

## Índice de 29 achados de portabilidade e funcionamento (24 abertos, 1 em andamento, 4 resolvidos)

| ID | Prioridade | Estado | Título | Épico |
|---|---|---|---|---|
| [PW23-001](PW23-001-thread-handle-use-after-free.md) | P0 | resolved | CloseHandle de thread ativa pode causar use-after-free | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-002](PW23-002-winmutex-owner-recursion-infinite.md) | P0 | resolved | WinMutex não preserva ownership, recursão nem INFINITE | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-003](PW23-003-roleinfo-wire-v145-mismatch.md) | P0 | open | RoleInfo difere na serialização entre cliente e servidor v145 | [e10-network-transport-v1](../epics/e10-network-transport-v1/README.md) |
| [PW23-004](PW23-004-atl-com-qi-variant-bstr.md) | P0 | open | Corrigir ownership, QueryInterface, Variant e layout BSTR | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-005](PW23-005-activex-pwbrowser-stub.md) | P1 | open | PWBrowser/ActiveX continua sem host funcional | [e09-interface-input-audio-v1](../epics/e09-interface-input-audio-v1/README.md) |
| [PW23-006](PW23-006-winapi-false-success-and-pointer-state.md) | P0 | open | Shims WinAPI mascaram falhas e perdem dados de janela | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-007](PW23-007-missing-full-game-wasm-target.md) | P0 | open | Não existe target WASM que linke e inicialize ElementClient | [e04-wasm-runtime-v1](../epics/e04-wasm-runtime-v1/README.md) |
| [PW23-008](PW23-008-incomplete-tu-and-link-closure.md) | P0 | open | Fechar TUs AUI/Element/Angelica e dependências reais do link | [e04-wasm-runtime-v1](../epics/e04-wasm-runtime-v1/README.md) |
| [PW23-009](PW23-009-gnet-browser-transport-unproven.md) | P0 | open | GNET/WSS/gateway/servidor não validados ponta a ponta | [e10-network-transport-v1](../epics/e10-network-transport-v1/README.md) |
| [PW23-010](PW23-010-s3tc-required-for-bgra.md) | P1 | open | Não exigir S3TC para renderizar DDS BGRA sem compressão | [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md) |
| [PW23-011](PW23-011-input-keyboard-ime-a ui-events.md) | P1 | open | Bridge de eventos do navegador para AUI/gameplay ainda é parcial | [e09-interface-input-audio-v1](../epics/e09-interface-input-audio-v1/README.md) |
| [PW23-012](PW23-012-encoding-gbk-utf16-utf8-mismatch.md) | P1 | open | GBK/UTF-16/UTF-8: fallback GB18030 de AWScriptFile falha na consulta de tamanho (reprodutor isolado) | [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md) |
| [PW23-013](PW23-013-test-gates-not-browser-game.md) | P1 | open | CI verde só valida parsers e compilações, não game no Chrome | [e16-qa-observability-v1](../epics/e16-qa-observability-v1/README.md) |
| [PW23-014](PW23-014-angelica-d3d8-webgl2-not-integrated.md) | P0 | open | Backend da Angelica original ainda não desenha os frames do jogo | [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md) |
| [PW23-015](PW23-015-web-audio-video-not-implemented.md) | P1 | open | AngelicaMedia compila mas não há saída WebAudio comprovada | [e09-interface-input-audio-v1](../epics/e09-interface-input-audio-v1/README.md) |
| [PW23-016](PW23-016-version-and-server-v145-baseline.md) | P0 | open | Revisão do código-fonte não comprovadamente idêntica ao PW 1.5.3 v145 | [e02-native-client-baseline-v1](../epics/e02-native-client-baseline-v1/README.md) |
| [PW23-017](PW23-017-stale-roadmap-and-handoff.md) | P1 | open | Roadmap, body PR e handoff registram estados antigos | [e16-qa-observability-v1](../epics/e16-qa-observability-v1/README.md) |
| [PW23-018](PW23-018-wininet-psapi-dll-seams.md) | P1 | open | Classificar WinINet/PSAPI/LoadLibrary: interfaces não são funcionalidades | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-019](PW23-019-blocking-main-loop-workers-pthreads.md) | P0 | open | Game loop, render/defence threads e waits ainda sem modelo browser validado | [e04-wasm-runtime-v1](../epics/e04-wasm-runtime-v1/README.md) |


| [PW23-020](PW23-020-assets-streaming-cache-cloudflare-pending.md) | P2 | open | Streaming/caching/Cloudflare de cliente completo continuam não implementados | [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md) |

| [PW23-038](PW23-038-ddraw-struct-layout-ci-regression.md) | P1 | resolved | Regressão de layout DDS 64-bit corrigida no commit 368c9da; CI #158 verde | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-039](PW23-039-strmif-sal-in-macro-ci-regression.md) | P1 | resolved | Regressão SAL __in/__out corrigida no commit 7eb3f4d; CI 40773786307 nativo/WASM verde | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-040](PW23-040-elements-data-browser-parser-init-failure.md) | P0 | open | Parser de elements.data falha no boot WASM; bypass apenas diagnóstico, dados do jogo ainda não validados | [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md) |
| [PW23-041](PW23-041-login-ecwld-scene-load-bypass.md) | P0 | open | Login.ecwld falha ao abrir no Chromium; bypass de LoadSceneObjects não comprova renderização da cena | [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md) |
| [PW23-042](PW23-042-ftdriver-test-hardcoded-clang-ci-failure.md) | P1 | open | Novo teste FTDriver invoca clang++ ausente no runner; CI do cliente falha e WASM é pulado | [e16-qa-observability-v1](../epics/e16-qa-observability-v1/README.md) |

| [PW23-043](PW23-043-a3dengine-init-browser-boot-blocker.md) | P0 | in_progress | Dispositivo D3D8 inicializa; pendências de caps/surfaces/configuração continuam | [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md) |

| [PW23-044](PW23-044-serverlist-direct-fopen-backslash-path.md) | P1 | open | Serverlist não abre por barras invertidas em access/fopen diretos no MEMFS | [e10-network-transport-v1](../epics/e10-network-transport-v1/README.md) |

| [PW23-045](PW23-045-winsync-synchronous-thread-ci-regression.md) | P1 | open | Execução inline de CreateThread/_beginthreadex quebra semântica nativa; CTest 33/34 e WASM pulado no CI | [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md) |
| [PW23-046](PW23-046-game-sources-absolute-path-wasm-ci-regression.md) | P0 | open | Manifest gerado usa caminhos absolutos de workstation; CI WASM falha no configure | [e04-wasm-runtime-v1](../epics/e04-wasm-runtime-v1/README.md) |

**Triagem sugerida:** começar por memória/lock e compatibilidade de wire (001–004), depois shims obrigatórios + link do jogo (006–009, 014, 019). Qualidade de testes e evidência (013/017) devem acompanhar cada milestone.

**Referências:** [review inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) · [review atualizado](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
