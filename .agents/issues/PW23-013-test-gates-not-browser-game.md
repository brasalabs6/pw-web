---
id: PW23-013
title: "CI verde só valida parsers e compilações, não game no Chrome"
priority: P1
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e16-qa-observability-v1
scope: pr-23
---

# PW23-013 — CI verde só valida parsers e compilações, não game no Chrome

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`.github/workflows/client153-port.yml` declara que não executa assets originais, login, realm ou gameplay; `test_browser_bootstrap.py` compila só um fragmento de wWinMain com `-D__EMSCRIPTEN__` nativo.

## Impacto / por que importa
Pode gerar falsa percepção de prontidão. Tests opt-in com assets e Playwright não integram os gates do workflow principal.

## Solução proposta
Criar níveis L1 TU, L2 link integral, L3 browser boot, L4 login, L5 mundo+2 sessões, L6 gameplay; screenshots, logs, traces por SHA; fixtures autorizadas no runner protegido.

## Testes e critérios de aceite
Status de cada gate factual; skip não contabilizado como PASS; game boot comprovado em navegador, não apenas trecho sintático.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e16-qa-observability-v1`](../epics/e16-qa-observability-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
