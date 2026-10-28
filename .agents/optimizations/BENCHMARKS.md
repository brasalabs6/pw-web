# Benchmark e gates de performance

## Premissa
Sem jogo integral jogável não existe benchmark representativo de cidade/batalha. Criar harness agora, medir microcenas, depois atualizar resultados com gameplay real.

## Cenas fixas
A. Exploração: terreno, vegetação, streaming e câmera em percurso gravado.
B. Cidade: densidade controlada de NPCs/jogadores, animações e materiais.
C. Batalha: personagens, efeitos, partículas, habilidades e tráfego.
D. Loading: cold cache, warm cache, rede limitada e troca de região.
E. Stress: sessão longa, context loss, resize e mobile suspend/resume.

## Instrumentação
Registrar commit SHA, versão do navegador, SO, CPU/GPU, RAM, resolução interna, preset, seed, número de entidades, temperatura/throttling quando acessível, build flags e rede. Medir FPS médio, 1% low, frame time p50/p95/p99, CPU/GPU timing, draw calls, triângulos, memória, TTFI/TTP, bytes e latência.

## Frame budget
30 FPS: 33,3 ms; 60 FPS: 16,7 ms; 120 FPS: 8,3 ms. São budgets totais, não garantias. Medir em execução prolongada após warmup.

## Protocolo A/B
Executar baseline e candidato em hardware idêntico, mesma cena e seed, múltiplas repetições, mediana e dispersão; isolar mudanças; anexar artefatos JSON, traces, screenshots e logs. Definir limites de regressão por cenário antes da implementação. Nunca declarar ganho em porcentagem sem medições.

## Gate mínimo de PR
- Compila nativo/WASM, testes de compatibilidade e sanitizers quando aplicáveis.
- Sem divergência visual ou funcional relevante.
- Benchmark comparativo reproduzível anexado.
- Sem regressão significativa de p95 frame time, RAM, TTP ou bateria.
- Fallback e rollback testados.
