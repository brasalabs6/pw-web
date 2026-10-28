---
id: OPT-021
title: "Separar atualização de combate de apresentação de centenas de jogadores"
priority: P1
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

# OPT-021 — Separar atualização de combate de apresentação de centenas de jogadores

**Observação:** candidato estático para otimização após o port WebAssembly **funcionar no navegador desktop e mobile**, não blocker real. A prioridade P1/P2 é relativa ao backlog pós-port. Não confundir FX com combate autoritativo: não suprimir dano, alvo, cooldown, estado de rede, hit timing ou telegraphs essenciais. Não há benchmark executado nesta revisão.

## Domínio e evidência estática
**Crowd — scheduler CPU**. [`src/client153/Element/CElementClient/EC_ManPlayer.cpp:1512-1530`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ManPlayer.cpp#L1512-L1530) itera todos os `m_PlayerTab` e chama `pPlayer->Tick`; [`src/client153/Element/CElementClient/EC_ElsePlayer.cpp:642-759`](https://github.com/brasalabs6/pw-web/blob/20feaf047072f4322766109b5ddc88978bcb8338/src/client153/Element/CElementClient/EC_ElsePlayer.cpp#L642-L759) faz trabalho de state/model, work manager, animação/face, distância, GFX de posição. Já existe `SetSlowestUpdateFlag` em linhas 703-717, não é correto dizer que o cliente não possui otimização alguma.

## Impacto possível (hipótese, não resultado)
Atualizações cosméticas independentes por entidade podem custar muito em wars grandes, mesmo com simplificação do esqueleto. A necessidade de update de gameplay e rede continua integral.

## Proposta experimental pós-port
Mapear call graph e dividir tick em estado autoritativo/interpolação e jobs visuais com scheduler por importância: host, party, inimigo selecionado, ataques dirigidos ao host, próximos e fundo; atualizar poses distantes em cadence menor e interpolar. Sem atrasar comando/cooldown/hit.

## Plano de reprodução / benchmark
50/100/200/500 avatares com animações e casts simultâneos, near/med/far; verificar timing/hit, animação, network state e CPU per subsistema p95; variar qualidade/tier.

**Critério de aceite:** primeiro estabelecer baseline repetível com dispositivo, navegador, SHA, cena e dataset autorizado; medir p50/p95/p99 frametime CPU/GPU quando suportado, alocações/heap, VRAM quando mensurável, draw calls e legibilidade do combate. Apenas implementar com comparação before/after e teste de gameplay/rede/frames. Se confirmar crash/corrupção/vulnerabilidade impeditiva, promover a `.agents/issues` com evidência.

## Rastreabilidade e limites
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA lido `20feaf047072f4322766109b5ddc88978bcb8338`.
- [e12-entities-gameplay-v1](../epics/e12-entities-gameplay-v1/README.md).
- Deduplicar com OPT-001…015 e PW23-021…037; detalhes de culling/esqueleto aqui são **pontos específicos de combate** e complementam backlog genérico, não o substituem.
- Apenas inspeção de código: sem teste WASM, Android/iOS, profiler real, agentes paralelos ou FPS alegados nesta sessão. Nenhum código-fonte modificado.
