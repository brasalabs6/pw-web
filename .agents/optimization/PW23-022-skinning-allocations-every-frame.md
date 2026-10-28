---
id: PW23-022
title: "Eliminar alocações O(vértices) por frame em AnimatedSkin"
priority: P1
status: open
kind: observed-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e08-avatar-animation-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-022 — Eliminar alocações O(vértices) por frame em AnimatedSkin

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/Angelica2/Angelica3D/port/animated_skin.cpp:100-131`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/port/animated_skin.cpp#L100-L131) cria `matrices` e `next` a cada `update()`, faz `push_back` por osso/vértice e `impl->positions = std::move(next)`; [`src/client153/Angelica2/Angelica3D/port/skin_binding.cpp:42-67`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/port/skin_binding.cpp#L42-L67) executa blending escalar por vértice.

## Impacto e hipótese técnica
Churn do heap, cópias/realocações, pressão de cache e pausas de memória crescem com entidades animadas. Ainda não foi perfilado o peso em milissegundos.

## Implementação proposta
Pré-alocar arrays e matrizes no load, reusar posições e buffers com capacidade estável, atualizar apenas ossos ativos, layout SoA/compacto onde demonstrado; preparar upload persistente ao GPU, sem sacrificar corretude das 4 influências.

## Testes / plano de medição
Bench com 1/10/50/100 personagens, contagem de alocações em 1.000 ticks, median/p95 CPU ms, heap peak, comparação de hashes/erro numérico em poses.

## Critério de aceite
Zero alocações dinâmicas no hot path de update estável e equivalência visual/numerical validada.

## Dependências e governança
- Épico: [e08-avatar-animation-v1](../epics/e08-avatar-animation-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
