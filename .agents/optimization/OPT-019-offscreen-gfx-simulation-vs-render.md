---
id: OPT-019
title: "Desacoplar simulação de efeitos invisíveis do registro para render"
priority: P1
status: candidate
kind: static-optimization
phase: post-browser-port
blocking: false
source_sha: 20feaf047072f4322766109b5ddc88978bcb8338
first_observed: 2026-10-28
validation: static-only
epic: e06-render-webgl2-v1
scope: PR-23
---

# OPT-019 — Desacoplar simulação de efeitos invisíveis do registro para render

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**VFX — CPU fora de tela**. [`src/client153/Element/CElementClient/EC_GFXCaster.cpp:282-338`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_GFXCaster.cpp#L282-L338) faz `TickAnimation` dos GFX ativos nas tabelas automática/normal/top; [`src/client153/Element/CElementClient/EC_GFXCaster.cpp:434-506`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_GFXCaster.cpp#L434-L506) restringe o registro de desenho por distância/visibilidade. Portanto, ocultação de render não significa eliminação automática de custo de simulação visual.

## Impacto possível (hipótese, não resultado)
Em batalhas com muitos casts fora da câmera, animações de partículas que não são desenhadas podem continuar usando CPU/heap; custos exatos desconhecidos.

## Proposta experimental pós-port
Adicionar estado de relevância perceptual, separando atualização lógica de TTL/efeito e simulação de partículas. Para efeitos não essenciais offscreen, atualização temporal baixa ou fast-forward determinístico no retorno, sem alterar hit timing ou flags do cliente.

## Plano de reprodução / benchmark
Duas câmeras (virada para guerra e oposta), mesma carga 50/200 skills/s; comparar CPU gfx tick, draw calls, memória, reaparecimento e tempo de vida dos efeitos.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
