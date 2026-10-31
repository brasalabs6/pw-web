---
id: OPT-016
title: "Reduzir retenção potencial de GFX de impacto parados durante guerras"
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

# OPT-016 — Reduzir retenção potencial de GFX de impacto parados durante guerras

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Efeitos — lifetime e memória**. [`src/client153/Element/CElementClient/EC_ManSkillGfx.cpp:376-395`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ManSkillGfx.cpp#L376-L395) passa `dwWaitTime=60000` em efeitos de impacto de runas; [`src/client153/Element/CElementClient/EC_GFXCaster.cpp:151-160`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_GFXCaster.cpp#L151-L160) documenta esse argumento em **segundos**, [`src/client153/Element/CElementClient/EC_GFXCaster.cpp:184-216`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_GFXCaster.cpp#L184-L216) multiplica por 1000 e [`src/client153/Element/CElementClient/EC_GFXCaster.cpp:282-316`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_GFXCaster.cpp#L282-L316) só destrói instância parada após o prazo quando não tiver sido reutilizada.

## Impacto possível (hipótese, não resultado)
16h40min de cache pós-stop por instância não reutilizada, com possível high-watermark de recursos em guerras com muitos efeitos simultâneos. Não é vazamento confirmado: o gerenciador reutiliza instâncias ST_STOP.

## Proposta experimental pós-port
Confirmar documentação e semântica da API no game completo; medir quantas instâncias ficam resident após picos de combate, CPU/GPU/heap. Se necessário, cache LRU por bytes e instâncias com TTL configurável por efeito, pooling limitado e cleanup por mudança de zona/memória.

## Plano de reprodução / benchmark
Sequências 100/1.000/10.000 impactos repetidos e diferentes, esperar o stop, medir residentes após 1, 5, 30 min e cenas pós-combate. Verificar que efeitos reutilizados não sofrem truncamento e não são recomputados sem necessidade.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
