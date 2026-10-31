---
id: PW23-006
title: "Shims WinAPI mascaram falhas e perdem dados de janela"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e03-platform-portability-v1
scope: pr-23
---

# PW23-006 — Shims WinAPI mascaram falhas e perdem dados de janela

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`platform/common/Windows.h`: `SetWindowTextA/W` retorna TRUE sem alterar estado; `SendMessageA/W` ignora mensagem; `GetWindowLong/SetWindowLong` não persistem dados; `SetWindowLongPtr` converte para `LONG`; `IsBadReadPtr` retorna sempre TRUE.

## Impacto / por que importa
UI/boot pode confiar em chamadas sem efeito; estado de janela/pointers se perde e conversão LONG_PTR pode truncar ponteiros.

## Solução proposta
Inventariar call sites por caminho de execução. Implementar window registry/message dispatch para APIs essenciais; retornar erro explícito para opcionais sem suporte; preservar widths/ABI e não usar sucesso de fachada.

## Testes e critérios de aceite
Testes round-trip set/get texto e userdata, mensagens com callbacks, error paths e integração real AUI/boot. Nenhum método obrigatório reporta sucesso sem ação.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e03-platform-portability-v1`](../epics/e03-platform-portability-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
