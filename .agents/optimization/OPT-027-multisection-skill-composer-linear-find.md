---
id: OPT-027
title: "Indexar compositores de skills multietapa por (skill,section)"
priority: P2
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

# OPT-027 — Indexar compositores de skills multietapa por (skill,section)

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Skills — lookup de compositor**. [`src/client153/Element/CElementClient/EC_ManAttacks.cpp:856-890`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ManAttacks.cpp#L856-L890) percorre linearmente `m_MultiSectionSkillComposerVec` tanto em `Play`, `GetSkillGfxComposer` e `GetSecionInfo`. No caminho de conjuração, [`src/client153/Element/CElementClient/EC_ManAttacks.cpp:123-145`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ManAttacks.cpp#L123-L145) faz Play seguido de Get para a mesma chave.

## Impacto possível (hipótese, não resultado)
Duas buscas lineares no caminho de skills multietapa são overhead potencial em alta taxa de casts, mas catálogo pode ser pequeno; provavelmente menor impacto que partículas/ossos.

## Proposta experimental pós-port
Verificar tamanho do catálogo e frequência; se relevante, mapa chave par estável e uma API lookup única para Play+Get; preservar lifetime de ponteiros e resolução de versões de skill.

## Plano de reprodução / benchmark
100/1.000/10.000 casts/s em catálogos de tamanho real e sintéticos; profiler, miss/hit, tempo de lookup e equivalência de ações.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e08-avatar-animation-v1](../epics/e08-avatar-animation-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
