---
id: OPT-022
title: "Reduzir submissão de sombras por personagem sem afetar combate"
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

# OPT-022 — Reduzir submissão de sombras por personagem sem afetar combate

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Crowd — sombras GPU**. [`src/client153/Element/CElementClient/EC_ElsePlayer.cpp:844-872`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ElsePlayer.cpp#L844-L872) desenha o modelo e quando `m_bCastShadow` está ativo adiciona um shadower individual ao renderizador de sombras. Isso pode ocorrer em massa na mesma cena de guerra.

## Impacto possível (hipótese, não resultado)
Centenas de sombras individuais podem gerar custo alto de CPU submissions e depth/render target; o efeito real depende do shadow renderer e do número de personagens com flag ativa.

## Proposta experimental pós-port
Após baseline, aplicar distância em tela, importância do avatar e budget de sombra por frame. Shadow atlas/instancing e impostor blob só quando o teste visual indicar; host, alvo e personagens próximos prioritários. Não alterar colisão/gameplay.

## Plano de reprodução / benchmark
50/100/200/500 avatares com shadow ON/OFF, sombras próximas e distantes; medir passes GPU, draw calls, textura, CPU e qualidade espacial.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e06-render-webgl2-v1](../epics/e06-render-webgl2-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
