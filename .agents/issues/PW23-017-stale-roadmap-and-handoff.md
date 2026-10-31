---
id: PW23-017
title: "Roadmap, body PR e handoff registram estados antigos"
priority: P1
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e16-qa-observability-v1
scope: pr-23
---

# PW23-017 — Roadmap, body PR e handoff registram estados antigos

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`docs/ROADMAP.md` ainda marca M3 não iniciado, PR body refere 8/8 e 'skeleton próximo'; `docs/CLIENT153-RESUMPTION-GROK.md` diz 'não commitado' apesar de existir. Handoff é histórico acumulativo.

## Impacto / por que importa
Agentes podem repetir trabalho, esquecer gaps ou achar que 73,4% TU significa 73,4% produto.

## Solução proposta
Atualizar uma matriz de capabilities por SHA, ambiente, status, comando, evidência e owner; manter seções históricas datadas, sincronizar PR body e links para issues.

## Testes e critérios de aceite
Nenhuma afirmação de conclusão sem teste; documentação atual e sem duplicar fonte de verdade.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e16-qa-observability-v1`](../epics/e16-qa-observability-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
