---
id: PW23-002
title: "WinMutex não preserva ownership, recursão nem INFINITE"
priority: P0
status: resolved
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e03-platform-portability-v1
scope: pr-23
---

# PW23-002 — WinMutex não preserva ownership, recursão nem INFINITE

**Status:** `resolved` no commit `a2a9c251` (branch `feat/client153-common-containers`, PR #23).

## Resolução (2026-10-27, commit a2a9c251)
`WinMutex` agora rastreia a thread owner (`DWORD owner`) e a profundidade recursiva (`unsigned depth`). O owner readquire livremente (cada aquisição incrementa a profundidade), `ReleaseMutex` de terceiro falha, release parcial mantém o ownership, e o último nível acorda os waiters. `WaitForSingleObject` trata `INFINITE` como espera verdadeiramente não limitada (`cv.wait`) em eventos, semáforos, mutexes e threads — não mais `wait_for(~49 dias)`. O `PollSignaled` do mutex ficou consistente com a aquisição recursiva.

**Comando/ambiente:** `cmake --build build/client153-{native,sanitize} && ctest --test-dir build/client153-{native,sanitize}` — GCC 16, ASan+UBSan no build sanitize.
**Evidência:** teste novo `tests/client153/winsync_semantics.cpp` cobrindo os critérios: reentrância dupla/tripla, release parcial, release por terceiro rejeitado, timeout zero/positivo de terceiro, create initialOwner (owner recursivo + timeout de outra thread), espera INFINITE entregue pelo release do owner, waits concorrentes. Native 31/31, wasm 26/26, sanitize 31/31.

## Evidência verificável
`winsync.h`: `WinMutex` guarda só `bool owned`; `WaitForSingleObject` espera `!owned`; `ReleaseMutex` não confere thread owner; `INFINITE` passa por `wait_for(milliseconds)`.

## Impacto / por que importa
Mutex Win32 aceita múltiplas aquisições pelo mesmo owner, impede release por outra thread e distingue espera infinita. Shim atual pode bloquear indevidamente ou liberar mutex alheio.

## Solução proposta
Armazenar owner e profundidade recursiva, corrigir semântica de timeout/wakeup e tratar casos não suportados explicitamente. Não confundir com CRITICAL_SECTION (já reentrante).

## Testes e critérios de aceite
Testes reentrância duas vezes, release parcial, release por terceiro, timeout zero/positivo/INFINITE, create initialOwner, fechamento em uso e waits concorrentes.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e03-platform-portability-v1`](../epics/e03-platform-portability-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
