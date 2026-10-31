---
id: PW23-018
title: "Classificar WinINet/PSAPI/LoadLibrary: interfaces não são funcionalidades"
priority: P1
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e03-platform-portability-v1
scope: pr-23
---

# PW23-018 — Classificar WinINet/PSAPI/LoadLibrary: interfaces não são funcionalidades

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`platform/common/Wininet.h` falha com ERROR_INTERNET_NOT_SUPPORTED; `Psapi.h` retorna ERROR_CALL_NOT_IMPLEMENTED; `Windows.h` retorna nullptr em LoadLibrary/GetProcAddress.

## Impacto / por que importa
Essas chamadas podem ser opcionais ou bloquear boot/UI/defence se o cliente depender delas. O shim não é implementação HTTP/enumeração de processos/DLL.

## Solução proposta
Auditar call sites e definir feature gates; criar fetch adapter somente quando necessário; degradar módulos Windows-only conscientemente sem enfraquecer validação do servidor.

## Testes e critérios de aceite
Bootstrap com cada feature faltante + testes caminhos de erro; sem falsa declaração de implementação.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e03-platform-portability-v1`](../epics/e03-platform-portability-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
