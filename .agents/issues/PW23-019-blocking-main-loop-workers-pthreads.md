---
id: PW23-019
title: "Game loop, render/defence threads e waits ainda sem modelo browser validado"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e04-wasm-runtime-v1
scope: pr-23
---

# PW23-019 — Game loop, render/defence threads e waits ainda sem modelo browser validado

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`ElementClient.cpp` usa wWinMain, loop de mensagens e threads; `docs/evidence/client153-full-build.md` lista main/render/defence; `app.mjs` roda apenas viewer RAF. Não há prova de Worker/pthreads/COOP/COEP.

## Impacto / por que importa
Loops síncronos e espera Win32 podem congelar UI. Thread affinity e lifetime dos shims podem quebrar renderer/GNET.

## Solução proposta
Definir ciclo init/tick/render/dispose não bloqueante com RAF/Worker e filas; ativar pthreads somente com isolamento e testes quando necessário.

## Testes e critérios de aceite
Chrome mantém frames e inputs responsivos sob carga, rede e animação; context-loss, shutdown e 2 sessões sem deadlock.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e04-wasm-runtime-v1`](../epics/e04-wasm-runtime-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
