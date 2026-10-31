---
id: OPT-024
title: "Avaliar eager load de compositores de skills e cache lazy por cena"
priority: P2
status: candidate
kind: static-optimization
phase: post-browser-port
blocking: false
source_sha: 20feaf047072f4322766109b5ddc88978bcb8338
first_observed: 2026-10-28
validation: static-only
epic: e05-asset-pipeline-v1
scope: PR-23
---

# OPT-024 — Avaliar eager load de compositores de skills e cache lazy por cena

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Skills — cold-start e memória**. [`src/client153/Element/CElementClient/EC_ManAttacks.cpp:569-606`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ManAttacks.cpp#L569-L606) itera todos os IDs de skill no construtor via `NextSkill`, resolve arquivos SGC e chama `LoadOneComposer`; [`src/client153/Element/CCommon/Gfx/A3DSkillGfxComposer2.cpp:492-518`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CCommon/Gfx/A3DSkillGfxComposer2.cpp#L492-L518) cria um compositor para cada arquivo carregado e guarda no mapa até `Release`.

## Impacto possível (hipótese, não resultado)
Carregar metadados de todas as skills na entrada da sessão pode aumentar IO/latência inicial e resident set. Não confundir com texturas/partículas em si, que podem ser lazy em outras camadas.

## Proposta experimental pós-port
Medir bytes/alocações de SGC e cold-start; avaliar manifest pré-indexado e lazy parse sob demanda com warm set para skills equipadas/visíveis/da área, prefetch para novos players e fallback instantâneo a versão simples de telegraph. Sem perder a primeira conjuração.

## Plano de reprodução / benchmark
Cold load de 100/1.000/total skills com cache frio/quente; tempo até primeira cena, heap, tempo da primeira conjuração e stutter.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
