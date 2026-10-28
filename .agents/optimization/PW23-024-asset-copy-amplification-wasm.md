---
id: PW23-024
title: "Reduzir picos de RAM durante fetch, VFS, parsers e texturas"
priority: P1
status: open
kind: observed-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e15-assets-streaming-perf-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-024 — Reduzir picos de RAM durante fetch, VFS, parsers e texturas

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`web/client153/app.mjs:7-36`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/web/client153/app.mjs#L7-L36) usa `arrayBuffer()` → `Uint8Array` → `crypto.subtle.digest` → `HEAPU8.set`, para MOX cria arquivo Emscripten VFS; [`src/client153/platform/web/runtime.cpp:228-244`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/platform/web/runtime.cpp#L228-L244) lê o mesmo MOX para outro `vector`; [`src/client153/Angelica2/Angelica3D/port/dds.cpp:8-14`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/port/dds.cpp#L8-L14) duplica DDS BGRA antes de trocar canais.

## Impacto e hipótese técnica
Há cópias simultâneas dos mesmos bytes e buffers de staging; podem elevar pico de RAM/WASM e causar tab kill especialmente em mobile. Não há medida do pico atual.

## Implementação proposta
Pipeline de recursos progressivo e deduplicado: SHA em chunk/stream ou pipeline confiável de armazenamento, staging de byte spans, upload sem VFS intermediária quando aplicável, buffer pool e conversão BGRA otimizada; manter verificação SHA antes de consumir o asset.

## Testes / plano de medição
Instrumentar JS heap, WASM heap, pico RSS, VFS residente e GPU estimada com pacote de assets representativo. Comparar mesmo asset hash, browser/Android e arquivos truncados.

## Critério de aceite
Documentar live peak antes/depois; arquivo inválido não é aplicado e buffers temporários são liberados.

## Dependências e governança
- Épico: [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
