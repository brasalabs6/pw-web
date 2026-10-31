---
id: PW23-026
title: "Cachear uniforms e reduzir chamadas WebGL/mesh"
priority: P2
status: open
kind: observed-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e06-render-webgl2-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-026 — Cachear uniforms e reduzir chamadas WebGL/mesh

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/platform/web/runtime.cpp:300-340`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/platform/web/runtime.cpp#L300-L340) faz `glGetUniformLocation` a cada frame (4 uniforms), muda `GL_CULL_FACE` por malha e executa `glDrawElements` individualmente por mesh.

## Impacto e hipótese técnica
Overhead de CPU→WebGL e estado redundante cresce com número de meshes; modelo atual é uma POC, não extrapolar resultado de FPS para jogo completo.

## Implementação proposta
Resolver locations 1x após link, cache de estado render e texturas, ordenar comandos por pipeline/material sem violar transparência; estudar atlas/instancing e buffers compactados, sem merging que quebre animação.

## Testes / plano de medição
Contar draw calls, binds, uniform calls e tempo CPU com 100/500/2.000 meshes; benchmark em Snapdragon/Adreno, Mali, iGPU e desktop.

## Critério de aceite
Redução mensurada de chamadas e CPU render sem divergências visuais ou ordenação incorreta.

## Dependências e governança
- Épico: [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
