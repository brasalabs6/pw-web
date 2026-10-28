---
id: PW23-007
title: "Não existe target WASM que linke e inicialize ElementClient"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e04-wasm-runtime-v1
scope: pr-23
---

# PW23-007 — Não existe target WASM que linke e inicialize ElementClient

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`src/client153/CMakeLists.txt`: navegador executa `platform/web/runtime.cpp`, `--no-entry`, exports `_client_draw/_client_load_file` e bibliotecas formats/file/math; `web/client153/app.mjs` carrega MOX/DDS com câmera orbital. `client153_engine_original` e `client153_aui_original` são targets `OBJECT EXCLUDE_FROM_ALL` separados.

## Impacto / por que importa
Provas reais de assets são um visualizador, não cliente de Perfect World em execução. Não há evidência de CECGame/CECGameRun inicializados no Chrome.

## Solução proposta
Criar `client153_game_full` (nome sugerido) com libs/objetos do jogo, entry browser, scheduler e integração renderer/UI; preservar viewer como regression target.

## Testes e critérios de aceite
Target do jogo linkado, instanciado no Chrome e percorrendo wWinMain/_InitGameApp/CECGame::Init/StartLogin até UI de login original com logs e screenshot. Sem falsos positivos.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e04-wasm-runtime-v1`](../epics/e04-wasm-runtime-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
