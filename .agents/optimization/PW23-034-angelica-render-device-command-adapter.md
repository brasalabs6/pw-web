---
id: PW23-034
title: "Construir backend da Angelica com render queue tipada e adapter"
priority: P0
status: open
kind: architecture-blocker
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e06-render-webgl2-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-034 — Construir backend da Angelica com render queue tipada e adapter

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/CMakeLists.txt:43-80`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/CMakeLists.txt#L43-L80) mantém bibliotecas originais OBJECT separadas; [`src/client153/CMakeLists.txt:209-225`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/CMakeLists.txt#L209-L225) produz `client153` a partir do visualizador, sem link ao target integral; [`src/client153/Angelica2/Angelica3D/Source/A3DDevice.cpp:600-636`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/Source/A3DDevice.cpp#L600-L636) mostra chamadas D3D8 a abstrair. **Complementa** PW23-014 (falta de integração), focando arquitetura para performance sustentável.

## Impacto e hipótese técnica
Portar cada SetRenderState como chamada GL direta pode amplificar mudanças de estado e impedir backend alternativo; reimplementar toda a engine do zero tem custo e risco altos.

## Implementação proposta
Manter semântica A3DEngine/A3DDevice, introduzir um RenderDevice/backend WebGL2 com comandos para pipeline, buffers, texturas, passes, materiais, UV/iluminação/fog, estado cacheado; render queue classificada por material/opacidade; testes golden frame; WebGPU posterior como segundo backend SEM duplicar gameplay.

## Testes / plano de medição
Testes contract para estado D3D8 → WebGL2, screenshots baseline modelo/skin/terrain/água/sombra/UI; rastrear state changes e custo de GPU em PR real.

## Critério de aceite
Angelica original emite comandos de draw reais; sem caminhos no-op mascarados; backend pluggable e medido, sem regressões de estados.

## Dependências e governança
- Épico: [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
