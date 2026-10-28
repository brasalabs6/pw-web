---
id: PW23-020
title: "Streaming/caching/Cloudflare de cliente completo continuam não implementados"
priority: P2
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e15-assets-streaming-perf-v1
scope: pr-23
---

# PW23-020 — Streaming/caching/Cloudflare de cliente completo continuam não implementados

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`docs/CLOUDFLARE-DELIVERY.md` descreve pipeline ainda planejado; viewer baixa cena pequena e `scene.json`; sem mundo chunked, cache OPFS/IndexedDB versionado ou deploy validado.

## Impacto / por que importa
Assets da versão original ocupam GB; download antecipado pode inviabilizar cold start, e cache do browser pode sofrer eviction.

## Solução proposta
Concluir pipeline e15/e17: chunks SHA-256 por região, CDN R2, streaming/prefetch, manifest transacional, cache com quota/eviction, performance budgets e rollback.

## Testes e critérios de aceite
Cold/warm start, memória, FPS e latência medidas em browsers reais; world load sem download total e deploy staging/rollback com assets autorizados.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e15-assets-streaming-perf-v1`](../epics/e15-assets-streaming-perf-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
