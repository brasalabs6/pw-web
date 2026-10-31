# Auditoria estática dedicada — combate PvP, skills, animações e multidões

**Data:** 2026-10-28  **Repo:** `brasalabs6/pw-web`  **PR:** [#23](https://github.com/brasalabs6/pw-web/pull/23)  **HEAD auditado:** `20feaf047072f4322766109b5ddc88978bcb8338`.

**Classificação:** análise documental, pós-port. Primeiro fazer funcionar no navegador desktop e mobile; nenhuma implementação de FPS ou alteração de gameplay está autorizada por este relatório. Não houve build/teste real ou benchmark mobile nem execução de subagentes.

## Principais descobertas do código (sem extrapolações)

1. **Mecanismo preexistente de qualidade de combate.** `CECOptimize::GFX` já oferece filtros separados cast/attack/fly/hit/state, com exceção para host; `DlgOptimizeGfx` tem presets. Não reescrever antes de instrumentar.
2. **Pipeline visual de skills:** `CECAttackEvent::DoFire` chama compositor SGC; `A3DSkillGfxComposer::Play` percorre alvos; `AddOneTarget` abre evento para cada alvo; `A3DSkillGfxMan::AddSkillGfxEvent` cria N projéteis de cluster; `CECSkillGfxEvent::Tick` localiza host, alvo e hooks e anima trajetórias. **Não mexer no pipeline lógico de dano.**
3. **Life cycle:** `SkillGfxMan` tem pool por tipo, mas aloca novas instâncias se o pool acabar. `GFXCaster` tem pool de efeitos automáticos/parados com wait time em segundos: quatro efeitos de runas passam `60000`, equivalentes a 16h40m de TTL após STOP se não reutilizados.
4. **Draw/CPU:** `GFXCaster::Render` testa raio até `GetSceneLoadRadius`, mas `GFXCaster::Tick` ainda atualiza as instâncias ativas (verificar eventual culling downstream). Jogadores/NP C têm `Tick` por entidade, e `CECElsePlayer` já tem flag `SetSlowestUpdateFlag`, então política nova deve compor, não duplicar.
5. **Custos adicionais de wars:** sombras opcionais individuais por avatar, nomes e barras até 20m, RayTrace linear no mouse/touch pick, carregamento de compositores de todas as skills no construtor e busca por pares skill/section em vetor.

## Estratégia de evolução (após port funcional)

**Separar três planos**, com limites rigorosos:
- **P0, integridade de combate:** pacotes de rede, states, cooldowns, hit timing, aplicação de dano, seleção real de alvo, posições interpoladas para gameplay e indicadores de ataques que ameaçam o jogador. Sem drop, cooldown fake ou fast-forward autoritativo.
- **P1, apresentação relevante:** animações legíveis de host/party/alvo/ameaças próximas; projéteis hostis diretos, efeitos de casting de ataques perigosos, controle de multidão, telegraphs de AoE, efeitos no jogador.
- **P2, cosmético degradável:** partículas repetidas distantes, ecos de impacto, detalhamento de acessórios, sombras secundárias, placas pouco importantes, VFX fora de tela e cache de longa retenção.

Implementar **uma só política de relevância visual**, capaz de acionar GFX event manager, particle tick, rendering, sombras, animações e interface **sem dar permissões a cada subsistema de cortar gameplay**. Inputs: distance, frustum, projected area, host/team/enemy/selected target, hostile damage threat, quality tier, memory, thermal hints e prioridade de comunicação visual. Garantir ordem determinística para ao menos elementos essenciais.

## Instrumentação sugerida

| Fase | Instrumentos | Unidades |
|---|---|---|
| `CECWorld::Tick` | network event dispatch, player/NPC ticks, AI cosmetics, skill scheduler | ms/frame, p95/p99 |
| `CECAttacksMan::Tick` | eventos lógicos/visuais, DoFire, DoDamage, active list | ns/event, active counts |
| `SkillGfxMan::Tick/Render` | ativos/livres por modo, hooks/target queries, clusters, loads | events/frame, malloc/frame |
| `GFXCaster::Tick/Render` | active/stopped, offscreen sim, registered/culled, auto-cache age | ms, bytes, counts |
| `PlayerMan::Tick/Render` | near/far tiers, face/hair, shadow submissions, text draw | ms/entity, shadows, labels |
| browser WebGL2 | draw calls, alpha pixels, texture upload, JS/WASM crossing | ms CPU/GPU, VRAM approx |

## Matriz de cenários a reproduzir **no futuro**

| Cenário | Jogadores | Skills/AoE | Objetivo |
|---|---:|---|---|
| Cidade cheia | 50/200/500 | ocasional | modelos, outfits, texto e sombra |
| Guerra em linha de frente | 50/100/200/500 | 20/100/500 casts por janela medida | animações, hits e overdraw |
| AoE extremo | 100/200 | 1/20/80 alvos, cluster 1/5/20 | crescimento multiplicativo de VFX e memória |
| Cam virada / offscreen | 200 | VFX ativos atrás da câmera | separar tick de render |
| Aproximação/afastamento/teleporte | 200 | skills com projéteis rastreadores | invalidar caches corretamente |
| Mobile térmico | 50/100/200 | mesma seed e viewport | 30/60 target, energy e long-task |

**As taxas e quantidades são cargas sintéticas propostas, não métricas observadas.** Em todas as cenas: mesma seed, asset pack autorizado, resolução/quality tier declarado, browser/version/hardware registrado, 5+ repetições, soak, screenshots, dinâmica de eventos/log de danos, motion interpolation, 1% low e frame-time p95/p99.

## Ordem pós-port por hipótese
1. Diagnosticar e limitar `OPT-016` (stopped-cache TTL) e telemetria `OPT-018` (event capacity).
2. Medir AoE `OPT-017` e VFX offscreen `OPT-019`/`020`; conservar sinais críticos `OPT-025`.
3. Avaliar scheduler de avatares `OPT-021`, lookup de hooks `OPT-026`, sombra `OPT-022`, labels `OPT-023`.
4. Resolver cold-start do catálogo `OPT-024`, lookup `OPT-027` e picking `OPT-028` se profiling demonstrar impacto.

## Deduplicação e integridade do backlog
- `PW23-030` já cobre skinning GPU/LOD; `OPT-021` é segmentação de Update gameplay vs cosmético, não reimplementação de skinning.
- `PW23-036` já propõe scene culling genérico; `OPT-022` e `OPT-023` focam sombra por avatar e texto in-game em crowds.
- `PW23-029` já cobre overdraw geral; `OPT-020` mede culling de efeitos de skill especificamente no `GFXCaster`.
- `OPT-004` cobre track cache; `OPT-024` cobre compositores SGC ao iniciar.
- `OPT-010` cobre busca de hanger em child skin; `OPT-026` cobre consulta por projétil por frame.
- `OPT-017` cobre a multiplicação N-alvos × cluster; `OPT-018` cobre capacidade da free list e pooling, sem duplicação funcional.
- `OPT-016` foca caching STOP em segundos, distinto de `OPT-018` pooling de eventos.

**Sem alterar `.agents/issues`**: só novos achados estáticos em `.agents/optimization`. Problemas do port que impedem browser continuam sendo prioridade; revalidar este HEAD antes de implementar qualquer hipótese.
