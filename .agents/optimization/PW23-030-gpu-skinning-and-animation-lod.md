---
id: PW23-030
title: "Migrar skinning massivo para GPU e criar animation LOD"
priority: P1
status: open
kind: architecture-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e08-avatar-animation-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-030 — Migrar skinning massivo para GPU e criar animation LOD

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/Angelica2/Angelica3D/port/skin_binding.cpp:42-68`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/port/skin_binding.cpp#L42-L68) deforma vértices em CPU; [`src/client153/Angelica2/Angelica3D/port/animated_skin.cpp:96-131`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/port/animated_skin.cpp#L96-L131) executa isso por mesh/vertex em cada update. A Angelica original já considera modos de render por vertex shader/CPU em [`src/client153/Angelica2/Angelica3D/Source/A3DDevice.cpp:532-576`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/Source/A3DDevice.cpp#L532-L576), mas não há prova do pipeline de skinning GPU no host atual.

## Impacto e hipótese técnica
Custo CPU escala com vértices e personagens, especialmente em encontros com muitos jogadores. GPU skinning pode não ser ótimo para todo tier, logo necessita caminho híbrido.

## Implementação proposta
Paletas de ossos em UBO/texture (respeitando limites), índices e pesos compactados, dual-quaternion opcional apenas com comparação; reduzir avaliação de animações fora da tela/distantes, compartilhar poses compatíveis, atualizar LOD de NPC em 15/30Hz com interpolação visual.

## Testes / plano de medição
10/50/100 personagens com poses reais, p95 CPU/GPU, latência de pose, qualidade da animação e limites de uniformes de WebGL2.

## Critério de aceite
Matriz de escolhas CPU/GPU por tier com ganhos medidos e sem deformação incorreta.

## Dependências e governança
- Épico: [e08-avatar-animation-v1](../epics/e08-avatar-animation-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
