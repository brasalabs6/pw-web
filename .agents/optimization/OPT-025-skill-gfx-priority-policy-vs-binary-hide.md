---
id: OPT-025
title: "Evoluir flags binárias de GFX para orçamento adaptativo sem ocultar telegraphs"
priority: P1
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

# OPT-025 — Evoluir flags binárias de GFX para orçamento adaptativo sem ocultar telegraphs

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Combate — política de relevância**. [`src/client153/Element/CElementClient/EC_Optimize.cpp:65-149`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_Optimize.cpp#L65-L149) implementa flags como `bHidePlayerCast/Attack/Fly/Hit` e `bExcludeHost` sem prioridade explícita por ameaça na decisão mostrada; [`src/client153/Element/CElementClient/DlgOptimizeGfx.cpp:105-164`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/DlgOptimizeGfx.cpp#L105-L164) tem preset que oculta cast/attack por categoria, mantendo fly/hit.

## Impacto possível (hipótese, não resultado)
Desligar classes completas alivia custo, mas pode ocultar pistas importantes em PvP e ser pouco eficiente quando muitos efeitos irrelevantes pertencem à mesma classe.

## Proposta experimental pós-port
Em vez de substituir o CECOptimize, evoluí-lo para 3 classes: ALWAYS host/alvo/ataques hostis que afetam o usuário; IMPORTANT party/nearby; COSMETIC offscreen/distante/repetido. Budget por CPU/GPU/particles sem interferir no protocolo. Criar whitelist de telegraphs essenciais antes de redução visual.

## Plano de reprodução / benchmark
Comparar original/high-quality/preset original/adaptive em cenário 200-500 players com ataques controlados; avaliar percepção de ataques perigosos, alvo, hit timing, screen clutter e p95 frame.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e09-interface-input-audio-v1](../epics/e09-interface-input-audio-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
