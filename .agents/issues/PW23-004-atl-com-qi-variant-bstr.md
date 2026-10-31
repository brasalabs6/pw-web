---
id: PW23-004
title: "Corrigir ownership, QueryInterface, Variant e layout BSTR"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e03-platform-portability-v1
scope: pr-23
---

# PW23-004 — Corrigir ownership, QueryInterface, Variant e layout BSTR

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`src/client153/platform/common/atlbase.h`: `CComQIPtr` chama `QueryInterface(nullptr, ...)`, outro construtor faz `static_cast` sem QI, falta destructor que libera referência; `CComVariant` permite cópia superficial com BSTR; `SysAllocString` prefixa tamanho em unidades e não bytes.

## Impacto / por que importa
Possíveis referências COM inválidas, leaks, double free e divergência ABI BSTR. Superfície de compilação não equivale a compatibilidade COM.

## Solução proposta
Implementar QI por IID real/fail closed, Rule of Five ou cópia proibida, AddRef/Release corretos, prefixo de bytes e `SysStringLen` em wchar; rever operator& e CoCreateInstance em reatribuição.

## Testes e critérios de aceite
Testes de mocks multi-interface, counters AddRef/Release, BSTR com UTF-16, cópia/move, ASan/UBSan e ABI. Não considerar ActiveX pronto até haver adapter funcional.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e03-platform-portability-v1`](../epics/e03-platform-portability-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
