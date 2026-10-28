---
id: OPT-018
title: "Orçamento de capacidade e reciclagem do pool de eventos SkillGfx"
priority: P1
status: candidate
kind: static-optimization
phase: post-browser-port
blocking: false
source_sha: 20feaf047072f4322766109b5ddc88978bcb8338
first_observed: 2026-10-28
validation: static-only
epic: e15-assets-streaming-perf-v1
scope: PR-23
---

# OPT-018 — Orçamento de capacidade e reciclagem do pool de eventos SkillGfx

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Skills — event pool / alocação**. [`src/client153/Element/CElementClient/EC_ManSkillGfx.h:80-132`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ManSkillGfx.h#L80-L132) traz free list por modo de movimento, mas `GetEmptyEvent` faz `new CECSkillGfxEvent` quando vazia, e `PushEvent` adiciona a uma lista de ativos. Cada Tick percorre todos os ativos e a reciclagem só ocorre quando `IsFinished`.

## Impacto possível (hipótese, não resultado)
A memória e o custo por frame acompanham o high-watermark de VFX ativos; isto é diferente do fanout por alvo (OPT-017), e o pool melhora a alocação mas não limita o número de instâncias nem a retenção dos objetos livres.

## Proposta experimental pós-port
Instrumentar active/free/peak e custo por modo; reutilizar objetos com ownership seguro, limite de retenção de pool por tier e graceful degradation cosmética quando budget excedido. Manter eventos essenciais e sua ordem temporal; não converter skip de VFX em skip de gameplay.

## Plano de reprodução / benchmark
Simular 10/50/200 lançadores com bursts, medir alocações novas, cache livre após 1 min, heap/WASM/VRAM e frames. Verificar que eventos reciclados não herdam alvo/compositor/escala anterior.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
