---
id: OPT-017
title: "Evitar explosão multiplicativa de eventos visuais de skills AoE"
priority: P1
status: candidate
kind: static-optimization
phase: post-browser-port
blocking: false
source_sha: 20feaf047072f4322766109b5ddc88978bcb8338
first_observed: 2026-10-28
validation: static-only
epic: e08-avatar-animation-v1
scope: PR-23
---

# OPT-017 — Evitar explosão multiplicativa de eventos visuais de skills AoE

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Skills AoE — fanout de eventos**. [`src/client153/Element/CCommon/Gfx/A3DSkillGfxComposer2.cpp:453-483`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CCommon/Gfx/A3DSkillGfxComposer2.cpp#L453-L483) executa `AddOneTarget` para cada alvo de `Targets`; [`src/client153/Element/CCommon/Gfx/A3DSkillGfxComposer2.cpp:388-437`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CCommon/Gfx/A3DSkillGfxComposer2.cpp#L388-L437) encaminha o `m_FlyCluster.m_ulCount` ao gerenciador; [`src/client153/Element/CCommon/Gfx/A3DSkillGfxEvent2.cpp:678-741`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CCommon/Gfx/A3DSkillGfxEvent2.cpp#L678-L741) cria até `nFlyGfxCount` instâncias para cada evento. O código já evita repetir certos efeitos area-wide após o primeiro alvo, o que deve ser preservado.

## Impacto possível (hipótese, não resultado)
Em áreas com muitos alvos, a quantidade potencial de eventos/projéteis visuais é proporcional a alvos × cluster, especialmente com casts simultâneos. O número de hits/danos não deve ser limitado.

## Proposta experimental pós-port
Separar fanout de hit autoritativo de apresentação, impor orçamento apenas de VFX: coalescer partículas idênticas por célula, escolher representantes, manter projéteis do jogador, alvo selecionado, controles hostis e indicadores essenciais. Avaliar batching sem alterar IDs/timings de server.

## Plano de reprodução / benchmark
1/20/80 alvos × 1/5/20 projéteis visuais × 10/50 lançadores; contar eventos, draw calls, VRAM e ms/frame. Comparar resultado de dano, timestamps, alvo e legibilidade das skills.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e08-avatar-animation-v1](../epics/e08-avatar-animation-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
