---
id: OPT-028
title: "Acelerar seleção de alvo em guerras com índice espacial de jogadores"
priority: P2
status: candidate
kind: static-optimization
phase: post-browser-port
blocking: false
source_sha: 20feaf047072f4322766109b5ddc88978bcb8338
first_observed: 2026-10-28
validation: static-only
epic: e12-entities-gameplay-v1
scope: PR-23
---

# OPT-028 — Acelerar seleção de alvo em guerras com índice espacial de jogadores

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Interação — alvo/ray tracing**. [`src/client153/Element/CElementClient/EC_ManPlayer.cpp:2123-2185`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ManPlayer.cpp#L2123-L2185) inicia `RayTrace` percorrendo todos os jogadores visíveis na tabela para checar AABBs, com distinções de tipo de clique e estado morto. Não se identificou broadphase anterior nesse trecho.

## Impacto possível (hipótese, não resultado)
Com centenas de jogadores, cada mouse/touch pick pode testar muitas AABBs; custo depende da frequência de picking. Não confundir com os traces de terreno, que já usam DDA.

## Proposta experimental pós-port
Medir chamadas de RayTrace; avaliar tile grid/BVH com lista de candidatos próxima do raio, preservar precedência de pick, estados ghost/dead, target cycling e hover; manter fallback linear quando índice inconsistente.

## Plano de reprodução / benchmark
Alvos em 50/200/500 players com toques e mouse, pick rate 10/60/s, falso positivo/negativo e p95 latency.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e12-entities-gameplay-v1](../epics/e12-entities-gameplay-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
