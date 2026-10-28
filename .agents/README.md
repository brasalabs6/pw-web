# Planejamento executável do PW Web

## Método técnico para portar a Angelica

Antes de executar e02–e06, consultar [POC-001](../docs/POC-ANGELICA.md) e
[método de port com contratos e tarefas](../docs/ANGELICA-PORTING-METHOD.md).
P1 é o próximo pacote autocontido para Grok/GLM. As workorders genéricas abaixo
não substituem seus contratos; o catálogo/scheduler não foi alterado por esta prova.

**20 épicos** versionados em `.agents/epics/<slug>-v1/`, cada um com charter, requisitos, arquitetura, implementação, testes, riscos, `tasks.md`, `task.md` workorders em JSON e Markdown, conforme o catálogo de cada épico. **Total: 135 workorders após a expansão e02–e06 do PR #15 (WIP).**

Use `python3 .agents/automation/epic_check.py` para validar contratos/dependências. Use `docs/AUTONOMY.md` para runbook Grok Build/GitHub Actions. Somente workorders com `automation: true` são elegíveis para disparo automático; todos os PRs exigem revisão humana. Direitos de terceiros são gate independente.

## Ordem de evolução
1. [e00-rights-governance-v1](epics/e00-rights-governance-v1/README.md) — Direitos, proveniência, licenciamento e decisões de produto
2. [e01-agent-orchestration-v1](epics/e01-agent-orchestration-v1/README.md) — Orquestração autônoma, isolamento e governança de agentes
3. [e02-native-client-baseline-v1](epics/e02-native-client-baseline-v1/README.md) — Baseline do cliente Windows e seleção validada da Angelica
4. [e03-platform-portability-v1](epics/e03-platform-portability-v1/README.md) — Desacoplamento de Win32, SDK e Direct3D
5. [e04-wasm-runtime-v1](epics/e04-wasm-runtime-v1/README.md) — Toolchain C++/Emscripten e runtime WebAssembly
6. [e05-asset-pipeline-v1](epics/e05-asset-pipeline-v1/README.md) — Pipeline de recursos PCK/PKX, formatos e proveniência
7. [e06-render-webgl2-v1](epics/e06-render-webgl2-v1/README.md) — Renderer WebGL2 da engine Angelica
8. [e07-world-terrain-v1](epics/e07-world-terrain-v1/README.md) — Terreno, cidades, vegetação, água e visibilidade do mundo
9. [e08-avatar-animation-v1](epics/e08-avatar-animation-v1/README.md) — Avatares, esqueletos, animações e customização
10. [e09-interface-input-audio-v1](epics/e09-interface-input-audio-v1/README.md) — UI do jogo, HUD, entrada, câmera e áudio
11. [e10-network-transport-v1](epics/e10-network-transport-v1/README.md) — GNET, protocolo de mundo e gateway WSS seguro
12. [e11-login-session-v1](epics/e11-login-session-v1/README.md) — Identidade, login, personagem e sessão
13. [e12-entities-gameplay-v1](epics/e12-entities-gameplay-v1/README.md) — Movimento, combate, skills, NPCs, quests e interação
14. [e13-multiplayer-social-v1](epics/e13-multiplayer-social-v1/README.md) — Sincronização MMO, presença, chat, grupo e guilda
15. [e14-items-economy-v1](epics/e14-items-economy-v1/README.md) — Inventário, equipamentos, crafting, loja e economia
16. [e15-assets-streaming-perf-v1](epics/e15-assets-streaming-perf-v1/README.md) — Streaming, CDN, cache OPFS e desempenho do navegador
17. [e16-qa-observability-v1](epics/e16-qa-observability-v1/README.md) — Estratégia de testes, tracing, replay e QA automatizado
18. [e17-security-deployment-v1](epics/e17-security-deployment-v1/README.md) — Segurança, infraestrutura, CI/CD e entrega
19. [e18-browser-ux-accessibility-v1](epics/e18-browser-ux-accessibility-v1/README.md) — Compatibilidade de navegadores, acessibilidade e dispositivos
20. [e19-liveops-evolution-v1](epics/e19-liveops-evolution-v1/README.md) — Operações contínuas, versionamento, conteúdo e manutenção

## Como criar um novo épico
1. Copiar estrutura v1, mudar slug e versionar.
2. Detalhar requisitos verificáveis, interfaces e riscos **específicos**.
3. Criar workorders JSON com scopes, dependências e testes; Markdown com execução.
4. Checar `epic_check.py`, adicionar ADR se afetar versões ou arquitetura.
5. Evitar marcar tarefas sem CI e prova apropriada (o scaffold não testa o MMORPG).

## Build/deploy Cloudflare e streaming

[Plano de entrega Cloudflare](../docs/CLOUDFLARE-DELIVERY.md), ADR-0005 e tarefas
CF-01–24 detalham e15/e17 com contratos novos. Planejado, não implantado; não altera
catálogo nem conclui workorders. Source portada será mantida em src/client153.


## Separação entre port e otimização

- [`issues/README.md`](issues/README.md): correções e lacunas que impedem ou comprometem o cliente WebAssembly funcional no navegador desktop/mobile.
- [`optimization/README.md`](optimization/README.md): candidatos de performance/Angelica/mobile/hardening para depois do baseline jogável, sem desviar a equipe do port.

Vulnerabilidades ou crashes efetivamente comprovados devem ser classificados como issues do port mesmo se descobertos em auditoria de performance.
