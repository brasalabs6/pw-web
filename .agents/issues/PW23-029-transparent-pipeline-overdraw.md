---
id: PW23-029
title: "Separar passes opacos e transparentes; minimizar overdraw"
priority: P1
status: open
kind: observed-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e06-render-webgl2-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-029 — Separar passes opacos e transparentes; minimizar overdraw

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/platform/web/runtime.cpp:305-341`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/platform/web/runtime.cpp#L305-L341) ativa `GL_BLEND` globalmente e desenha meshes em ordem de arquivo; `two_sided` altera culling individualmente. Não aparece separação por opacidade/transparência neste visualizador.

## Impacto e hipótese técnica
Pode gerar trabalho extra de blend e erros de ordenação de alpha; o ganho depende de fill rate, geometria e conteúdo, e ainda precisa de medição.

## Implementação proposta
Classificar materiais por alpha mode; pass opaco front-to-back com depth-write e blend desligado, pass transparente back-to-front ou OIT só se justificar custo; alpha test/cutout e impostors para vegetação; analisar overdraw via GPU capture.

## Testes / plano de medição
Benchmark com vegetação/personagens/partículas, screenshot diff e GPU fragment workload; comparação em mobile.

## Critério de aceite
Sem regressão de alpha, estabilidade visual e redução demonstrada de tempo GPU em cenas relevantes.

## Dependências e governança
- Épico: [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
