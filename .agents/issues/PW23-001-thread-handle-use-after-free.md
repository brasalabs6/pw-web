---
id: PW23-001
title: "CloseHandle de thread ativa pode causar use-after-free"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e03-platform-portability-v1
scope: pr-23
---

# PW23-001 — CloseHandle de thread ativa pode causar use-after-free

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

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
