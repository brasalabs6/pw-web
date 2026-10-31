---
id: OPT-023
title: "Orçamento de nomes, barras e textos de combate em multidões"
priority: P2
status: candidate
kind: static-optimization
phase: post-browser-port
blocking: false
source_sha: 20feaf047072f4322766109b5ddc88978bcb8338
first_observed: 2026-10-28
validation: static-only
epic: e09-interface-input-audio-v1
scope: PR-23
---

# OPT-023 — Orçamento de nomes, barras e textos de combate em multidões

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Crowd — UI e text**. [`src/client153/Element/CElementClient/EC_ManPlayer.cpp:2029-2078`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ManPlayer.cpp#L2029-L2078) ativa nomes para jogadores a até 20m ou hover/selecionado, independentemente da densidade; [`src/client153/Element/CElementClient/EC_ElsePlayer.cpp:881-907`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ElsePlayer.cpp#L881-L907) desenha barras/nomes por avatar. O sistema já limita nomes por raio, logo o problema é overlap em multidões dentro dele.

## Impacto possível (hipótese, não resultado)
Em concentração de dezenas/centenas a 20m, projeções de texto, layout e overdraw de placas podem competir com sprites e skills; sem contagem de label calls não provar impacto.

## Proposta experimental pós-port
Priorizar alvo atual, party/squad, adversários perigosos e mouse/touch hover; orçamento screen-space, clusterização de nomes cosméticos, escalonamento/occlusion de placas e culling sem alterar identificação acessível ou vida/buff essencial.

## Plano de reprodução / benchmark
50/100/200/500 jogadores num raio de 20m, zoom e telas mobile pequenas, screenshot QA de legibilidade, tempo de UI render, draw calls e acessibilidade.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e09-interface-input-audio-v1](../epics/e09-interface-input-audio-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
