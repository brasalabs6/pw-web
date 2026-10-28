---
id: PW23-009
title: "GNET/WSS/gateway/servidor não validados ponta a ponta"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e10-network-transport-v1
scope: pr-23
---

# PW23-009 — GNET/WSS/gateway/servidor não validados ponta a ponta

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`platform/common/winsock2.h` mapeia APIs para POSIX; o handoff mantém GNET select/poll e wakeup UDP não testados no browser; CI não testa realm, rede ou gameplay.

## Impacto / por que importa
O navegador não oferece TCP arbitrário, e o modelo de prontidão e threads GNET não pode ser assumido compatível com Emscripten/WebSocket. Transporte pode bloquear ou corromper sessão.

## Solução proposta
Criar adapter assíncrono para StreamIO/ActiveIO/PollIO e gateway WSS binário ↔ TCP com backpressure, auth e destinos controlados; preservar marshal GNET original.

## Testes e critérios de aceite
Chrome↔WSS↔glinkd real: handshake, login, lista/seleção, EnterWorld, timeout, reconexão e dois clientes convergentes; golden packets.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e10-network-transport-v1`](../epics/e10-network-transport-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
