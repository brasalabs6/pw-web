---
id: PW23-012
title: "Codificações GBK/UTF-16/UTF-8 incompatíveis entre caminhos"
priority: P1
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e05-asset-pipeline-v1
scope: pr-23
---

# PW23-012 — Codificações GBK/UTF-16/UTF-8 incompatíveis entre caminhos

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`app.mjs` usa TextDecoder('gbk') em recursos; `atlbase.h` faz conversões narrow usando UTF-8 strict; compilação usa `UNICODE` e `-fshort-wchar` com CRT UTF-16 próprio.

## Impacto / por que importa
Texto/nome de assets/strings GNET/diálogos podem estar em CP936, e macros de conversão ATL podem receber tipos errados.

## Solução proposta
Formalizar encoding por origem e boundary (WinAPI A/W, ACHAR, PCK, Lua, RPC), implementar conversões explícitas CP936/UTF8/UTF16 e falhas claras para sequências inválidas.

## Testes e critérios de aceite
Fixtures nomes reais não ASCII, português/chinês, surrogate pairs, path PCK e string wire; round-trip sem data loss.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e05-asset-pipeline-v1`](../epics/e05-asset-pipeline-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
