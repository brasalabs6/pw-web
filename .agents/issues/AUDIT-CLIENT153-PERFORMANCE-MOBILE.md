# Auditoria Angelica 1.5.3 — FPS máximo com orçamento de RAM/mobile
Data: 2026-10-28
Escopo: `brasalabs6/pw-web`, branch `feat/client153-common-containers`, baseline lida `6184be84d4d8766ac02f489c4867df2e0044539a`, PR #23.
Status: **primeiro ciclo de análise estática do código e workflows**. Nenhum benchmark, dispositivo Android/iOS ou exploração de vulnerabilidade foi executado neste ambiente. Não declarar FPS ou bugs reproduzidos sem evidência.

## Estado factual
- O `client153` browser atual é um viewer MOX/DDS/GLSL WebGL2. A Angelica original está em targets independentes, e o target `client153_original_link_probe` é diagnóstico, não game jogável.
- CI faz builds e testes de parsing. `client153-port.yml` reconhece que não executa assets completos, login, realm ou gameplay. Portanto qualquer meta de FPS é **alvo a medir**, não resultado.
- Já existiam 20 issues PW23-001…020. Não reabrir duplicatas; 021…037 focam principalmente performance, hardening e estratégia PS2. As issues de integração 007/014/019 permanecem bloqueadores reais.
- Precisa de baseline no cliente nativo e perfil do WebGL2; o código original isolado não permite inferir gargalo do game inteiro.

## Estratégia técnica — 'GTA San Andreas em hardware de PS2' aplicada corretamente
A regra é **não manter o mundo inteiro em RAM**. O trabalho é maximizar **pixels úteis, triângulos visíveis, dados residentes e simulações necessárias** por joule/MB/milisegundo; não somente 'C++ mais rápido'.
1. **Resident set com budgets:** cada região/asset tem prioridade, custo RAM/VRAM, data de último uso, ticket de prefetch e função de descarte. Não presumir que 9 GB do instalador precisam ficar no heap/browser cache.
2. **World streaming por células**: metadata mínima em RAM, carregar entorno e direção do jogador, cancelar requests obsoletos; recursos coarse antes dos finos; cache persistente apenas onde browser permite, com quotas e evicção. Cache de storage é best-effort.
3. **Occlusion/frustum/LOD/HLOD:** não enviar o que não aparece. Prédios/vegetação repetitiva podem ser instanced; objetos distantes usam materiais simplificados/impostors quando fidelidade permitir. Não sacrificar colisão/jogabilidade.
4. **Animações conscientes de visibilidade:** ossos de personagens próximos 60/30Hz, distantes menor frequência + interpolação; crowd budget. GPU skinning quando medir ganho, CPU fallback quando for melhor.
5. **Material and render queues:** camadas opacas com batch/material sorting, transparentes com ordenação, texture atlases e menos state changes. Minimizar WebGL JS↔WASM↔GPU calls. Nunca mascarar render com sucesso falso.
6. **Memória orientada a vida útil:** arenas frame/zone, pools para componentes, SoA para dados que iteram juntos, liberar VFS/CPU copy após GPU upload, evitar malloc e strings no frame loop.
7. **Mobile primeiro:** quality tiers reais, resolução adaptativa, limite de draw calls/overdraw/pixel density, ETC2/KTX2/BasisU e fallback, 30 FPS estáveis como opção energética em low-tier; 60 FPS como objetivo de tier capaz; desligar cargas não críticas sob pressão térmica.
8. **Evolução sem big-bang cego:** manter frontend C++/Angelica como semântica e implementar RenderDevice/SceneScheduler/AssetManager adaptáveis. WebGL2 baseline; considerar WebGPU quando suporte/dispositivos e benchmark justificarem. Dual-backend e golden frames diminuem regressão.

## Equipe de subagentes — contratos para paralelismo
Estas são **frentes de investigação propostas** para os executores Grok existentes, não subagentes que tenham sido lançados nesta sessão:
- **audit-render**: `A3DDevice/A3DEngine/A3DShader/A3DStream/A3DTerrain`, chamadas GPU, draw/state calls, fixed-function e render queue; e06/e07.
- **audit-memory-assets**: MOX/SKI/DDS/PCK, lifetime, deduplicação, budgets, cache/streaming; e05/e15.
- **audit-animation**: `A3DSkeleton/A3DSkinModel/animated_skin`, hot loops, SIMD/SIMT, GPU skinning e LOD; e08.
- **audit-security**: parsers, path traversal, count overflow, resource exhaustion, shims Win32, GNET/deserialização; e03/e10/e17.
- **audit-mobile**: DPR, input/touch, refresh, context-loss, GPU formats e aparelho low tier; e09/e18.
- **audit-benchmark**: orquestra CI, asserts, comparadores visuais, profiler Chrome e priorização por custo/ganho; e16.
- **audit-architect** (integrador): deduplica issues, valida baseline SHA, limita paralelismo, prioriza, exige evidência, não faz self-merge.

Cada auditor pode inspecionar código **em paralelo**, mas publication deve ser serializada por responsável e usar lock/concurrency para não sobrescrever atualizações do mesmo arquivo. Loop deve ser **limitado por execução**, com agendamento de nova execução quando houver capacidade: Github Actions não é um processo infinito. Sem tokens e runners habilitados na plataforma, o processo não roda sozinho.

## Evidência exigida em cada rodada
1. HEAD SHA e diff desde última rodada; não confiar em findings antigos sem revalidação.
2. Scan por área e dedup dos IDs antes de criar; `observed-code`, `hypothesis` e `reproduced` são estados distintos.
3. Reprodutor ou prova estática com link `file:line`, input mínimo, hardware e engine mode.
4. Antes/depois por cenário, sem trocar qualidade/objeto salvo objetivo explícito.
5. Métrica: frametime p50/p95/p99 em ms, FPS 1% low, tempo CPU, tempo GPU quando disponível, drawcalls, upload MB/s, RAM/VRAM pico, nº de allocs por frame, network bytes, hitches e temperatura/bateria sob soak.
6. Regressões de render por screenshots + comparação de gameplay, inclusive alpha, animação, sombras, UI e cor.
7. Sanitizers no nativo + testes Emscripten/WASM + Chrome real + Android físico; testar desktop sem supor equivalência com mobile.
8. Se houver confirmação de vulnerabilidade explorável, divulgar responsavelmente sem registrar segredos ou exploit operacional no repositório público.

## Alvos de desempenho a VALIDAR (não são medições)
- Render 60 FPS → 16,67 ms/frame total. Mobile low-tier → 30 FPS estáveis (33,33 ms) preferíveis a 60 com thermal throttling. High refresh opcional condicionado a margem térmica.
- Frame budget precisa distinguir simulação, CPU render, GPU render, rede, IO/decode e jitter de scheduling.
- Definir **por dispositivo/cena**, não budgets globais rígidos. Exemplo de hipótese de planejamento: CPU sim+game 3–5ms, CPU render 2–4ms, GPU 6–9ms e margem; ajustar só após trace.
- Critérios de regressão propostos: nenhuma queda >5% p95 nas cenas determinísticas sem justificativa, e nenhuma piora persistente de memória/energia; thresholds ficam no fixture, não neste texto.

## Ordem executável dos próximos ciclos
- Fase 0 (agora): registro estático dos achados 021–037 e deduplicação contra 001–020; atualizar índice.
- Fase 1: estabilizar correção P0 de ownership/shims + link do cliente completo (001–009/014/019/034). **Não confundir com otimização de FPS**.
- Fase 2: perf harness e dataset autorizados, hardware Android real, baseline (031), orçamento memory/asset (024/025/032/033).
- Fase 3: resolver custos de risco alto e ganho provável: 021, 022, 023, 027, 028, 029, 030, 035; benchmark por hipótese.
- Fase 4: render queue e arquitetura backend, culling hierárquico, streaming e perf policies; port de WebGPU apenas se profile e uso justificar.
- Fase 5: soak, comparações golden, CI de regressão, stress de rede, 30/60 FPS tier policy e envio de reports por SHA.

## Protocolo de auditoria contínua no GitHub Actions
O repositório já contém `agent-backlog.yml` (schedule 07:13/19:13 UTC, condicionado a `AGENT_BACKLOG_ENABLED`) e `agent-review.yml` (2 modelos, restrição a PR agent/* ou dispatch validado). **Esses workflows não fazem automaticamente esta auditoria cliente 24/7**. Para fazê-lo: criar workflow dedicado em `main` quando aprovado, com `schedule` e `workflow_dispatch`; matriz de seis roles, checkout do SHA da branch de cliente, `permissions: contents:read` para auditoria, GITHUB_TOKEN mínimo, artifact JSON estruturado por subagente, integrador separado deduplica e publica apenas em .agents/issues na branch do PR #23 via commit com lease; nunca publicar patches executáveis do PR sem validação; retries bounded e cron contínuo. Evitar conflitos com o agente que já codifica no PR, deploy/cert secrets e limites financeiros de execução. Registrar resultado/erros por run e permitir pausar com variable (ex.: `PW_CLIENT_AUDIT_ENABLED`).
- O setup acima **é especificação proposta**; não implica workflow instalado ou execução contínua.
- Respeitar `AGENTS.md`: NÃO abrir terceiro PR, não fazer merge automático, não tocar PR do servidor #17.
