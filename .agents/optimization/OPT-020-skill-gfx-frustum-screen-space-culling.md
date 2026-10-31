---
id: OPT-020
title: "Complementar culling de VFX por raio com frustum e tamanho projetado"
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

# OPT-020 — Complementar culling de VFX por raio com frustum e tamanho projetado

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**VFX — GPU fillrate e visibilidade**. [`src/client153/Element/CElementClient/EC_GFXCaster.cpp:434-507`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_GFXCaster.cpp#L434-L507) compara distância do AABB de GFX à câmera com `GetSceneLoadRadius()` (e verifica `Is2DRender` para auto GFX) antes de registrar para desenho. A verificação nesse trecho é por raio e estado, não por frustum completo ou área projetada.

## Impacto possível (hipótese, não resultado)
VFX dentro do raio, mas atrás da câmera/fora do enquadramento, podem ser registrados e depois descartados pelo renderer, desperdiçando CPU/GPU; transparências grandes agravam o custo mobile. Não alegar ausência de culling no gerenciador gráfico posterior.

## Proposta experimental pós-port
Inspecionar `A3DGFXExMan::RegisterGfx` e culling interno antes de adicionar culling redundante; testar frustum AABB/circulo projetado e thresholds por material/efeito, com exceções explícitas para indicadores críticos.

## Plano de reprodução / benchmark
Batalha com câmera alternando frente/atrás, 1/4 viewport, grande número de partículas alpha; capturas GPU, chamadas de RegisterGfx, ms CPU/GPU e screenshots.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
