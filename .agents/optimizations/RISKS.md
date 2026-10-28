# Riscos, trade-offs e decisões

| Risco | Impacto | Mitigação |
|---|---|---|
| Port ainda não jogável | Benchmarks irreais | Microbenchmarks agora; cenas reais depois |
| WebGPU heterogêneo | Incompatibilidade | WebGL2 baseline, feature detection |
| Cache removido pelo navegador | Redownload | Manifests, streaming e recuperação |
| Limites térmicos mobile | FPS instável | Perfis adaptativos, testes prolongados |
| JS/WASM cópias | CPU/memória | Reuso de buffers e profiling |
| LOD/animação divergentes | Bugs de combate | Separar visual de gameplay |
| Multithreading browser | Falha de suporte | Fallback sem SharedArrayBuffer |
| C++ server comunitário | Divergência/proveniência | Testes golden, auditoria e autorização |
| Reescrita prematura | Atraso e regressões | Interfaces e benchmark antes de refatorar |
| Direitos sobre assets/código | Bloqueio de distribuição | Revisão jurídica e proveniência |

## Decisões recomendadas
- ADOPT: preservar Angelica original como referência de comportamento.
- ADOPT: WebGL2 como baseline, WebGPU experimental.
- ADOPT: streaming/cache com tolerância à evicção.
- DEFER: Rust para core do servidor.
- DEFER: WebTransport até haver testes de compatibilidade e ganho.
- REJECT: alegações de 2x/5x FPS sem benchmark reproduzível.
