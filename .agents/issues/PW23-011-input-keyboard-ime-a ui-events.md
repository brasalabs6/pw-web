---
id: PW23-011
title: "Bridge de eventos do navegador para AUI/gameplay ainda é parcial"
priority: P1
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e09-interface-input-audio-v1
scope: pr-23
---

# PW23-011 — Bridge de eventos do navegador para AUI/gameplay ainda é parcial

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`web/client153/app.mjs` passa só posição de pointer/origem, drag orbital do viewer; `keystate.cpp` armazena cursor. Testes `test_ime_contract.py` e `test_cursor_position.py` são probes nativos.

## Impacto / por que importa
Não há prova de teclado, input de texto/composição, key repeat, mouse wheel, foco, DPI, resize e Win32 message pump efetivo consumido por CECGame/AUI.

## Solução proposta
Normalizar eventos do DOM em fila temporal enviada ao WASM; mapear Win32 calls necessárias, atualizar state/resize/IME e integração com game loop sem bloqueio.

## Testes e critérios de aceite
Teste real Playwright em UI AUI para click/typing/acento/IME/focus loss/mouse/keyboard/resize; sem caracteres corrompidos.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e09-interface-input-audio-v1`](../epics/e09-interface-input-audio-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
