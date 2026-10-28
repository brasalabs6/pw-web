---
id: PW23-027
title: "Reprojetar streaming, cache e LOD do A3DTerrain2 por budget"
priority: P1
status: open
kind: architecture-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e07-world-terrain-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-027 — Reprojetar streaming, cache e LOD do A3DTerrain2 por budget

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/Angelica2/Angelica3D/Source/A3DTerrain2.cpp:107-130`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/Source/A3DTerrain2.cpp#L107-L130) define defaults `m_iMaxCachedBlk=128`, `m_iBlkLoadSpeed=8`, `m_fViewRadius=100`; [`src/client153/Angelica2/Angelica3D/Source/A3DTerrain2.cpp:919-997`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/Source/A3DTerrain2.cpp#L919-L997) contém carregamento síncrono ou fila candidata limitada por quantidade de blocos, não por tempo/bytes.

## Impacto e hipótese técnica
Valores estáticos não respeitam RAM, VRAM, rede, velocidade de câmera nem orçamento por frame. A eficácia do culling e custo de IO precisam de baseline executável.

## Implementação proposta
Paginação espacial/tiles, índices de visibilidade, culling frustum/occlusion, HLOD e geometria progressiva; worker para decode, cache LRU por bytes, priorização por direção/posição, descarte de regiões frias, prefetch com cancelamento. Preservar a geometria e colisão de gameplay.

## Testes / plano de medição
Capturas de tráfego e memory timeline ao caminhar/cavalgar/voar/teleportar em 2 GB/4 GB/8 GB; p95 loading hitch, RAM/VRAM e estabilidade de frame.

## Critério de aceite
Sem fetch/decode síncrono na thread de apresentação e budgets configuráveis medidos por tier.

## Dependências e governança
- Épico: [e07-world-terrain-v1](../epics/e07-world-terrain-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
