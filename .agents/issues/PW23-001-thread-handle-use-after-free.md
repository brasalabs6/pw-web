---
id: PW23-001
title: "CloseHandle de thread ativa pode causar use-after-free"
priority: P0
status: resolved
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e03-platform-portability-v1
scope: pr-23
---

# PW23-001 — CloseHandle de thread ativa pode causar use-after-free

**Status:** `resolved` no commit `a2a9c251` (branch `feat/client153-common-containers`, PR #23).

## Resolução (2026-10-27, commit a2a9c251)
O estado de execução da thread (mutex/cv/done/exitCode) foi separado para um `ThreadState` com ownership conjunto entre o handle (`WinThread`) e a lambda de entrada via `shared_ptr` (`winsync.h`). `CloseHandle` de thread em execução agora libera apenas o handle (detach do `std::thread`); o estado da entrada sobrevive até a thread concluir. A corrida de join múltiplo também foi corrigida: exatamente um waiter executa o `join` (flag `joined` sob o mutex do estado).

**Comando/ambiente:** `cmake --build build/client153-{native,sanitize} && ctest --test-dir build/client153-{native,sanitize}` — GCC 16, ASan+UBSan no build sanitize.
**Evidência:** teste novo `tests/client153/winsync_semantics.cpp` (close-while-running escreve num atomic após o close; 4 waiters concorrentes; exit code; suspended create). Native 31/31, wasm 26/26, sanitize 31/31 (ASan/UBSan) — sem use-after-free, leak ou acesso a objeto destruído.

## Evidência verificável
`src/client153/platform/common/winsync.h`: a lambda de `_beginthreadex` usa `state->wrapped` depois de iniciar; em `CloseHandle` para thread executando há `detach()` seguido por `delete thread`.

## Impacto / por que importa
Fechar o handle antes da conclusão não deve destruir o estado usado pela thread. Do jeito atual, a lambda pode acessar mutex/cv/done já liberados (race e UB).

## Solução proposta
Separar estado de execução e handle com ownership/lifetime independente (`shared_ptr`/refcount ou equivalente). Auditar waits/fechamentos concorrentes e cleanup após a conclusão.

## Testes e critérios de aceite
Stress test fechando handle antes do término sob ASan/UBSan; repetir com múltiplos waiters e fechamento após término. Sem use-after-free, leak ou callback em objeto destruído.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e03-platform-portability-v1`](../epics/e03-platform-portability-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
