---
id: PW23-035
title: "Frame scheduler fixed-timestep e pacing para mobile/browser"
priority: P1
status: open
kind: architecture-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e04-wasm-runtime-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-035 — Frame scheduler fixed-timestep e pacing para mobile/browser

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`web/client153/app.mjs:44-50`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/web/client153/app.mjs#L44-L50) agenda RAF continuamente sem intervalo de simulação independente, frame pacing, `visibilitychange` ou tier de refresh; PW23-019 cobre o bloqueio principal do loop legado.

## Impacto e hipótese técnica
Tentar renderizar na taxa máxima de painel 90/120Hz sem budget pode acelerar aquecimento, causar frame jitter e desperdiçar energia; simulação acoplada a FPS gera diferenças de jogabilidade.

## Implementação proposta
Separar simulação fixed dt (com clamp/limite de catch-up), interpolação de render, quality/fps tiers 30/60/adaptive, no hidden render, Worker para jobs não gráficos após validar overhead; timestamps monotônicos e clock sync da rede.

## Testes / plano de medição
60/90/120Hz, aba suspensa por minutos, lost focus, mudança de orientation, lag spikes 200ms, Mobile Safari/Chrome e RTT variable; simulação determinística dentro da tolerância.

## Critério de aceite
Sem runaway updates após background, sem input lag adicional indevido e pacing estável por tier.

## Dependências e governança
- Épico: [e04-wasm-runtime-v1](../epics/e04-wasm-runtime-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
