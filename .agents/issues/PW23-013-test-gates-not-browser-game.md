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

## Revalidação 2026-10-29 — probe e coletor divergiram após migração para main thread

**SHA do código analisado:** `d7be65777b43cefee59fe5ad14465db15d5e36ee` ([commit](https://github.com/brasalabs6/pw-web/commit/d7be65777b43cefee59fe5ad14465db15d5e36ee)). **Achado estático determinístico, não teste executado:** o novo [`scripts/client153-entry-browser-pre.js:11-16,90-95`](https://github.com/brasalabs6/pw-web/blob/d7be65777b43cefee59fe5ad14465db15d5e36ee/scripts/client153-entry-browser-pre.js#L11-L16) registra mensagens de texto no console e em `#status` (por exemplo `return: pw153_browser_probe returned N` ou `EXCEPTION: ...`). Entretanto, o [`tests/client153/browser-entry-probe.cjs:55-80`](https://github.com/brasalabs6/pw-web/blob/d7be65777b43cefee59fe5ad14465db15d5e36ee/tests/client153/browser-entry-probe.cjs#L55-L80) ainda espera linhas contendo `"type":"return"`/`"type":"exception"` ou `WORKER ERROR`, e [`browser-entry-outcome.cjs:4-15`](https://github.com/brasalabs6/pw-web/blob/d7be65777b43cefee59fe5ad14465db15d5e36ee/tests/client153/browser-entry-outcome.cjs#L4-L15) só reconhece mensagens JSON com `type` e `result`. O novo formato nunca será classificado como `returned` por esse parser, mesmo quando o probe concluir com código zero. Além disso, o `page.goto` e `page.evaluate` do teste dependem da responsividade da thread principal, atualmente ocupada pelo `wWinMain` síncrono (ver PW23-019).

**Impacto:** o diagnóstico Chromium fica desatualizado, pode terminar em `incomplete`/timeout ou não capturar a falha real; o gate não prova boot, render ou gameplay. Não inferir falha de CI ou de jogo sem execução do teste. O CI nativo/WASM não executa automaticamente este probe completo com os assets originais.

**Correção de port/teste (não tuning):** alinhar protocolo do emissor e coletor (mensagens estruturadas de `return`, `exception`, progress e heartbeat); registrar falhas de `page.goto`, timeout e render em artefatos, distinguir `boot iniciado` de `frame apresentado`; adicionar teste isolado que simule os dois resultados (zero e não zero) antes de rodar Chromium com assets autorizados. Revalidar o teste depois da correção de event loop em PW23-019. **Nenhum navegador, fixture, benchmark ou dispositivo mobile foi executado nesta inspeção.**

**Deduplicação:** permanece em **PW23-013** (cobertura e validade do gate). Não criar nova issue para o mesmo teste.
