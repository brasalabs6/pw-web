---
id: PW23-045
title: "Execução síncrona de CreateThread/_beginthreadex quebra contrato nativo e CI"
priority: P1
status: open
kind: reproduced-ci-regression
first_observed: 2026-10-29
verified_against: fdd9d9cc0ced2920cd811b05cd6e725948407d77
epic: e03-platform-portability-v1
scope: pr-23
---

# PW23-045 — Execução síncrona de threads quebra contrato nativo e CI

## Evidência reproduzida

- **PR / branch / SHA:** [PR #23](https://github.com/brasalabs6/pw-web/pull/23), `feat/client153-common-containers`, [`fdd9d9cc0ced2920cd811b05cd6e725948407d77`](https://github.com/brasalabs6/pw-web/commit/fdd9d9cc0ced2920cd811b05cd6e725948407d77).
- **Execução CI real:** [run 40942456804, job native-and-wasm](https://github.com/brasalabs6/pw-web/actions/runs/40942456804), 2026-10-29 19:43 UTC, terminou em **failure**: **33/34 CTest passaram**; `client153_winsync_semantics` abortou com `std::runtime_error: code == STILL_ACTIVE`. Etapa WASM foi pulada.
- **Causa demonstrada no diff:** [`winsync.h:140–147`](https://github.com/brasalabs6/pw-web/blob/fdd9d9cc0ced2920cd811b05cd6e725948407d77/src/client153/platform/common/winsync.h#L140-L147) introduz `RunThreadBodyNow`; [`CreateThread:527–543`](https://github.com/brasalabs6/pw-web/blob/fdd9d9cc0ced2920cd811b05cd6e725948407d77/src/client153/platform/common/winsync.h#L527-L543) e [`_beginthreadex:251–272`](https://github.com/brasalabs6/pw-web/blob/fdd9d9cc0ced2920cd811b05cd6e725948407d77/src/client153/platform/common/winsync.h#L251-L272) chamam-no **sem guarda `__EMSCRIPTEN__`**, substituindo `std::thread` também no build nativo.
- **Contrato violado:** [`tests/client153/winsync_semantics.cpp:92–108`](https://github.com/brasalabs6/pw-web/blob/fdd9d9cc0ced2920cd811b05cd6e725948407d77/tests/client153/winsync_semantics.cpp#L92-L108) espera que `GetExitCodeThread` imediatamente após `CreateThread` retorne `STILL_ACTIVE` enquanto a thread dorme por 50ms; execução inline completa antes de retornar e devolve 42. O log confirma precisamente essa assertiva.
- **Risco WASM separado, ainda não reproduzido:** funções de thread de longa duração executadas inline podem bloquear a thread principal do navegador. Não afirmar freeze de Chrome sem trace.

## Impacto

Regressão real da semântica nativa Win32 e do gate de CI; a etapa WASM fica sem validação neste run. Não é uma medição de FPS, problema de gameplay comprovado nem falha de segurança explorável.

## Correção proposta

1. Preservar `std::thread` e estado de término no build nativo; limitar adaptação single-thread ao `__EMSCRIPTEN__`.
2. No browser, mapear rotinas de trabalho específicas (ex.: transição de céu) para tarefas cooperativas finitas/async, sem executar qualquer body potencialmente bloqueante de forma genérica no chamador.
3. Manter `GetExitCodeThread`, `WaitForSingleObject`, `ResumeThread`, `CloseHandle` consistentes com o contrato de cada backend; não mascarar falhas como sucesso.

## Critérios de aceite

- `ctest --test-dir build/client153-native --output-on-failure`: todos os testes, inclusive `client153_winsync_semantics`, passam no mesmo ambiente CI.
- `python3 -m unittest discover -s tests/client153 -p 'test_*.py'` e etapa WASM do workflow executadas até conclusão (PW23-042 é bloqueio distinto a acompanhar).
- Browser probe com SHA e evidência de progresso de frame/tick/input durante trabalho de céu, sem travamento da UI; sem afirmar sucesso apenas por `wWinMain` retornar 0.
- Atualizar `status: resolved`, commit, teste e links somente após CI verde no SHA corrigido.

## Deduplicação

PW23-001 documenta **lifetime/UAF de CloseHandle** já resolvido, não esta regressão de execução inline. PW23-002 trata ownership/recursão de mutex; PW23-019 trata modelo geral do loop browser; PW23-042 documenta falha anterior de `clang++` no teste FTDriver. Este registro trata o **novo CTest reproduzido** no HEAD `fdd9d9cc0ced2920cd811b05cd6e725948407d77`. Review correspondente: [PR review #5704509033](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5704509033).
