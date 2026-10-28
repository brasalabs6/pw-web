# Roadmap incremental

## P0 — Fundação, imediatamente
- [ ] Criar harness de benchmark, instrumentação e cenários sintéticos.
- [ ] Inventário real de assets e relatório de distribuição de tamanhos.
- [ ] Interfaces Renderer/Assets/Simulation/Transport e política de feature flags.
- [ ] Perfil de memória e context-loss da demo WebGL2 existente.
- [ ] Definir matriz de dispositivos de teste.

## P1 — Quando skeleton/skinning e mundo estiverem disponíveis
- [ ] LOD e culling de personagens/terreno.
- [ ] Batching, instancing e ordenação de comandos.
- [ ] Streaming regional e cache com versionamento.
- [ ] Skinning CPU versus GPU comparado.
- [ ] Perfis mobile e resolução dinâmica.

## P2 — Com gameplay integrado
- [ ] Testar cidade e batalha em dispositivos reais.
- [ ] Otimizar hotspots comprovados com flamegraphs.
- [ ] Paralelizar jobs seguros e reduzir transferências JS/WASM.
- [ ] Validar protocolo e carga do servidor original.

## P3 — Experimental
- [ ] Backend WebGPU com fallback WebGL2.
- [ ] WebTransport contra baseline WebSocket.
- [ ] Refactors estruturais apenas com ROI demonstrado.

## Critério de conclusão
Cada item deve ter owner, PR, baseline, hipótese, testes de correção, comparação quantitativa, risco e rollback. Não antecipar P2/P3 ao custo de impedir port funcional.
