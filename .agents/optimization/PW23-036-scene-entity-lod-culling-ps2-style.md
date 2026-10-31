---
id: PW23-036
title: "Culling hierárquico e orçamento por entidade/efeito estilo PS2"
priority: P1
status: open
kind: strategic-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e12-entities-gameplay-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-036 — Culling hierárquico e orçamento por entidade/efeito estilo PS2

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/Angelica2/Angelica3D/Source/A3DEngine.cpp:96-116`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/Source/A3DEngine.cpp#L96-L116) já tem contadores de render; [`src/client153/Angelica2/Angelica3D/Source/A3DTerrain2.cpp:880-927`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/Source/A3DTerrain2.cpp#L880-L927) já calcula LOD por bloco. O cliente integral ainda não tem um perfil comprovado de custo por classe de entidade no navegador. **Proposta de otimização**, não bug confirmado.

## Impacto e hipótese técnica
Otimizar microchamadas sem definir workset pode deixar CPU/GPU dominados por NPCs distantes, sombras, vegetação, partículas e animações.

## Implementação proposta
Inspirado em consoles de pouca RAM: workset residente estrito, visibility grids/portal/frustum, HLOD/impostors (com critério visual), effect LOD, crowd budgets e animação por distância, instancing para elementos repetidos, física colisão por proximidade; nunca remover entidades necessárias ao protocolo/colisão.

## Testes / plano de medição
Cenas com densidade crescente, distância de visão variável, NPC próximo/distante, 1000 objetos estáticos, densidade de partículas, screenshots e CPU/GPU/hitch before-after.

## Critério de aceite
Curvas de custo por entidade e thresholds por tier; mesmo comportamento de gameplay visível; nenhuma alegação de ganho sem benchmark.

## Dependências e governança
- Épico: [e12-entities-gameplay-v1](../epics/e12-entities-gameplay-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
