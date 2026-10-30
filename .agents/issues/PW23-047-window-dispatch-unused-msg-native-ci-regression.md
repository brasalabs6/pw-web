---
id: PW23-047
title: "Unused MSG in window dispatch breaks native CI and skips WASM"
priority: P1
status: open
kind: reproduced-ci-regression
first_observed: 2026-10-30
verified_against: a2a4952e22ccc995610db2362689813c97885680
epic: e03-platform-portability-v1
scope: pr-23
---

# PW23-047 — unused MSG in window dispatch breaks native CI

## Evidência reproduzida

- **PR / branch / SHA:** [PR #23](https://github.com/brasalabs6/pw-web/pull/23), `feat/client153-common-containers`, [`a2a4952e22ccc995610db2362689813c97885680`](https://github.com/brasalabs6/pw-web/commit/a2a4952e22ccc995610db2362689813c97885680).
- **CI oficial:** [run 41038635911 / native-and-wasm job 124005606985](https://github.com/brasalabs6/pw-web/actions/runs/41038635911/job/124005606985), 2026-10-30 10:10 UTC, conclusão **failure**.
- **Comando/gate:** workflow `Client 1.5.3 port build`, etapa `Native original-layout and bounded package tests`; a compilação CMake nativa termina em `gmake: *** [Makefile:101: all] Error 2` e exit 2.
- **Erro literal:** `src/client153/platform/common/window.cpp:89:7: error: variable 'message_struct' set but not used [-Werror=unused-but-set-variable]`, `cc1plus: all warnings being treated as errors`.
- **Causa no diff:** [`window.cpp:85–96`](https://github.com/brasalabs6/pw-web/blob/a2a4952e22ccc995610db2362689813c97885680/src/client153/platform/common/window.cpp#L85-L96) constrói e preenche `MSG message_struct` mas chama `window.procedure(&window, message, wParam, lParam)` diretamente, sem ler a variável.
- **Efeito no pipeline:** etapa `Build WebGL2 runtime and execute WASM parser regressions` marcada `skipped`; **não** há resultado WASM para esse HEAD. Não confundir com falha de jogo no Chrome.

## Impacto

Regressão de compilação do target de plataforma nativo e perda de validação WASM no CI obrigatório. Não comprova crash do browser, regressão de FPS nem problema de gameplay.

## Correção proposta

Eliminar o `MSG` temporário sem uso (ou efetivamente usá-lo em contrato necessário) mantendo `-Werror`; garantir que a adaptação do dispatch não introduza warnings ou quebre a semântica de `WNDPROC`. Não fazer tuning.

## Testes e critérios de aceite

1. Em checkout limpo do SHA corrigido, executar o mesmo workflow e obter build nativo sem warnings tratados como erros.
2. `ctest --test-dir build/client153-native --output-on-failure` com todos os testes realmente executados e aprovados.
3. Etapa WASM **não** pulada: configurar, compilar/linkar `client153_browser_entry` e executar `ctest --test-dir build/client153-wasm --output-on-failure` com resultados verdes.
4. Registrar SHA, links de job e contagens; nenhum resultado de compilação prova render, login ou gameplay.

## Deduplicação

- PW23-045 cobre regressão anterior de `CreateThread` e `STILL_ACTIVE`, não este warning de `window.cpp`.
- PW23-011 cobre semântica e cobertura do bridge de entrada; os problemas estáticos `PeekMessageW` e `WM_MOUSEWHEEL` devem ser acompanhados lá, não em nova issue de blocker.
- [Review incremental #5711080100](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5711080100) documenta ambos com SHA e linhas.
- Preservar PR único #23, sem tocar PR servidor #17, sem auto-merge.
