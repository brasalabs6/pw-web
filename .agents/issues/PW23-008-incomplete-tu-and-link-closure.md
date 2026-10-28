---
id: PW23-008
title: "Fechar TUs AUI/Element/Angelica e dependências reais do link"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e04-wasm-runtime-v1
scope: pr-23
---

# PW23-008 — Fechar TUs AUI/Element/Angelica e dependências reais do link

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`docs/CLIENT153-HANDOFF.md` registra último probe 578/787 objetos ElementClient e 25/46 AUI. `client153_engine_original` compila apenas parte das fontes. Trial-link anterior de ~526 objetos devolveu 12 símbolos indefinidos, não link global.

## Impacto / por que importa
Compilar uma fração não garante linking completo; o conjunto restante pode introduzir muitos outros símbolos e dependências (renderer, mídia, Lua etc.).

## Solução proposta
Reexecutar probe no HEAD para cada projeto VCProj e library; fechar erros de ATL/varargs/case/asm e integrar AUI/Angelica/Media; relatório estruturado de missing symbols e ABI `-fshort-wchar` em todos targets.

## Testes e critérios de aceite
100% do código necessário ao binário game compila E LINKA, com relatórios reproduzíveis por projeto/objeto/símbolo/CI; não traduzir % de TU em % do game.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e04-wasm-runtime-v1`](../epics/e04-wasm-runtime-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
