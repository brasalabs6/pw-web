---
id: PW23-031
title: "Criar laboratório de FPS, memória, energia e CI de regressão"
priority: P1
status: open
kind: missing-measurement
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e16-qa-observability-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-031 — Criar laboratório de FPS, memória, energia e CI de regressão

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`.github/workflows/client153-port.yml:20-49`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/.github/workflows/client153-port.yml#L20-L49) compila e testa parsers; registra explicitamente ausência de jogo/assets/realm. [`web/client153/app.mjs:1-50`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/web/client153/app.mjs#L1-L50) conta frames e draw calls, mas não p50/p95/p99 frame times, CPU/GPU timing, heap peak nem bateria. Complementa PW23-013 (completude do jogo).

## Impacto e hipótese técnica
Sem medições reproduzíveis não é possível escolher otimizações por ganho real, detectar regressões nem afirmar FPS mobile.

## Implementação proposta
Benchmark sintético reproduzível SEM redistribuir assets, dataset autorizado fora Git, mais cenários reais quando disponíveis; PerformanceObserver/JS frame delta, C++ zones, EXT_disjoint_timer_query_webgl2 quando presente, memória por API disponível, logs de throttling, snapshots Chrome/Android e histórico com thresholds.

## Testes / plano de medição
Scenarios: spawn, cidade lotada, campo, voo, combate, teleport, 100 NPCs; cache frio/quente, 5 minutos + 30 minutos soak, perf em desktop e 3 tiers mobile; métricas p50/p95/p99, 1% lows, CPU/GPU frame, malloc bytes, GPU calls e energia.

## Critério de aceite
Baseline versionada por SHA/dispositivo/release; novas mudanças trazem before/after e gates diferenciados (hardware real vs CI smoke).

## Dependências e governança
- Épico: [e16-qa-observability-v1](../epics/e16-qa-observability-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
