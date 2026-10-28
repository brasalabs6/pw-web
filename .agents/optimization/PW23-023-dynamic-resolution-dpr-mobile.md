---
id: PW23-023
title: "Substituir DPR fixo por resolução dinâmica com histerese"
priority: P1
status: open
kind: observed-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e15-assets-streaming-perf-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-023 — Substituir DPR fixo por resolução dinâmica com histerese

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`web/client153/app.mjs:43-48`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/web/client153/app.mjs#L43-L48) define `Math.min(devicePixelRatio,2)`, sem acompanhar custo de GPU, estado térmico ou visibilidade.

## Impacto e hipótese técnica
DPR 2 quadruplica pixels relativos a DPR 1; em celular com muitas camadas, fill-rate e bandwidth podem dominar. Este fator é geométrico, não afirma ganho fixo de FPS.

## Implementação proposta
Render scale adaptativo independente do tamanho CSS, metas de frame 16,67/33,33ms, limites por tier, histerese de 2–5 s, upscale com estratégia mensurável; não alterar escala da UI/áreas de toque.

## Testes / plano de medição
Teste em DPR 1–3, modos econômico/equilibrado/qualidade; medir estabilidade de p95 e perda de nitidez; simular long-task e background.

## Critério de aceite
O controller reduz pixels sob sobrecarga sustentada, recupera qualidade em estabilidade e não oscila mais de uma vez por janela configurada.

## Dependências e governança
- Épico: [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
