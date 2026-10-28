---
id: PW23-005
title: "PWBrowser/ActiveX continua sem host funcional"
priority: P1
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e09-interface-input-audio-v1
scope: pr-23
---

# PW23-005 — PWBrowser/ActiveX continua sem host funcional

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`src/client153/platform/common/atlcom_registry.cpp` possui registro vazio; `atlhost.h` faz `CAxWindow::Create` retornar null e `AtlAxGetControl/AtlAxCreateControl` retornar E_FAIL; mapas COM/SINK vazios no `atlbase.h`.

## Impacto / por que importa
O navegador Internet Explorer embutido no cliente não opera, apesar de os headers ATL compilarem. Não se deve misturar este widget web legado com a AUI principal do jogo.

## Solução proposta
Classificar todos os call sites: opcional/obrigatório. Isolar o PWBrowser como feature opcional ou implementar overlay/iframe específico com event bridge e políticas de origem; evitar construir um COM genérico desnecessário.

## Testes e critérios de aceite
Sem travamento de login AUI quando opcional estiver desativado; caso implementado, criar/navegar/receber eventos/destruir widget no Chrome.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e09-interface-input-audio-v1`](../epics/e09-interface-input-audio-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
