---
id: PW23-021
title: "Desabilitar preserveDrawingBuffer em render interativo"
priority: P1
status: open
kind: observed-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e06-render-webgl2-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-021 — Desabilitar preserveDrawingBuffer em render interativo

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
Em [`src/client153/platform/web/runtime.cpp:84-92`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/platform/web/runtime.cpp#L84-L92) o contexto WebGL2 é inicializado com `attrs.preserveDrawingBuffer = true`. A aplicação apresenta loop de render em [`web/client153/app.mjs:44-50`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/web/client153/app.mjs#L44-L50).

## Impacto e hipótese técnica
Preservar o backbuffer pode impedir otimizações de composição/descarte, elevando consumo de memória, bandwidth e energia em GPUs móveis. O ganho real depende do browser/GPU; ainda não foi medido.

## Implementação proposta
Alterar para false no caminho normal e usar captura de screenshot explícita via framebuffer dedicado, sem preservar buffer todos os frames. Testar compatibilidade com context loss e screenshot.

## Testes / plano de medição
Comparar A/B Chrome Android + desktop: GPU frame time, memory, watts/temperatura, render correto, screenshots e context restore; habilitar flag reversível.

## Critério de aceite
Modo padrão usa false sem regressão visual e tem benchmark documentado; qualquer exceção precisa de justificativa.

## Dependências e governança
- Épico: [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
