---
id: PW23-033
title: "Definir budget de WASM heap, pools, descarte e recuperação OOM"
priority: P1
status: open
kind: memory-architecture
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e15-assets-streaming-perf-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-033 — Definir budget de WASM heap, pools, descarte e recuperação OOM

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/CMakeLists.txt:215-227`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/CMakeLists.txt#L215-L227) habilita `-sALLOW_MEMORY_GROWTH=1` sem um contrato explícito de budget por tier no target; [`web/client153/app.mjs:13-15`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/web/client153/app.mjs#L13-L15) usa malloc transitório e não exibe contadores heap; não se identifica um mecanismo integral de budget global entre JS, WASM e GPU na rota demonstrada.

## Impacto e hipótese técnica
WASM cresce sob pressão de assets, e abas móveis podem sofrer kill sem recuperação controlada. Crescimento de memória não equivale a vazamento; precisa profiling por cenário.

## Implementação proposta
Definir memory budgets por tier (JS/WASM/VFS/GPU), pools/arenas fixas por frame e regiões, métricas high watermark, checkpoints de liberação, degradação de qualidade antes de OOM e tratamento de erro; testar memory growth e fragmentação.

## Testes / plano de medição
Soak de 30/60 minutos com teleports, load/unload, background/foreground, context loss, repeated scene mounts; curva de bytes residentes e p99 de alocação.

## Critério de aceite
Memória se estabiliza depois de ciclos equivalentes, unload libera recursos e OOM é tratado sem loop ou travamento.

## Dependências e governança
- Épico: [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
