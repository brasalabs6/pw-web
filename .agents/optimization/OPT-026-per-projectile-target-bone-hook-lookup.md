---
id: OPT-026
title: "Cache de posição/hook por entidade e frame para projéteis de skill"
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

# OPT-026 — Cache de posição/hook por entidade e frame para projéteis de skill

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Skills — follow target**. [`src/client153/Element/CElementClient/EC_ManSkillGfx.cpp:307-374`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ManSkillGfx.cpp#L307-L374) invoca `_get_pos_by_id` tanto para host quanto target e `_get_dir_and_up_by_id` a cada Tick de evento; [`src/client153/Element/CElementClient/EC_ManSkillGfx.cpp:23-70`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ManSkillGfx.cpp#L23-L70) pode procurar modelo, child hanger, skeleton hook e calcular matrizes por consulta.

## Impacto possível (hipótese, não resultado)
Se vários projéteis perseguirem o mesmo alvo, lookups de jogadores e hooks/esqueleto podem ser repetidos no mesmo frame; a necessidade de precisão do movimento varia por modo e proximidade.

## Proposta experimental pós-port
Resolver target/host com identificador + geração (para evitar ponteiro obsoleto), cachear posição de hooks por entidade/frame, invalidar em troca de modelo/hanger/despawn; calcular follow com frequência por importância e interpolação, nunca alterar hit lógico.

## Plano de reprodução / benchmark
100/1.000/10.000 instâncias de projéteis visuais em 1/20/200 alvos; medir GetSkeletonHook calls e ms por tick, validar atach/hook durante troca de arma/skin e teleport.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e08-avatar-animation-v1](../epics/e08-avatar-animation-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
