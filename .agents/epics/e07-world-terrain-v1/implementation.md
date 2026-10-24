# Plano de implementação — Terreno, cidades, vegetação, água e visibilidade do mundo

## Ordem das workorders (cada uma corresponde a PR/uma prova)
### WO-01 — Definir matrizes de coordenadas, units/axis e modelo de chunks/scenegraph
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** WorldChunk, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** coordenadas, escala e eixos divergentes.

### WO-02 — Implementar heightfield e malha procedural com LOD e stitching
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** HeightField, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** costuras de terreno.

### WO-03 — Implementar quadtree/frustum culling e carregamento de objetos estáticos
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** VisibilitySet, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** carregamento de chunks gera stutter.

### WO-04 — Adicionar estados para água, vegetação, skybox, iluminação e clima
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** SceneGraph, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** coordenadas, escala e eixos divergentes.

### WO-05 — Comparar cenário autorizado e testar teleporte/streaming sem corruptelas
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** IChunkLoader, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** costuras de terreno.

## Estratégia de entrega incremental
- Primeiro mocks/test doubles e formatos neutros/sintéticos; segundo interfaces; terceiro prova real autorizada; quarto comparação; quinto hardening.
- Cada arquivo alterado precisa estar dentro do `write_scopes` da workorder; `.github`, `.agents/automation`, `references/` e secrets são áreas protegidas.
- PR descreve explicitamente o que funciona, o que está faltando e como provar.
- Falha em qualquer gate leva a reparo limitado (máximo 3 tentativas), depois triagem humana.

## Integração entre subsistemas
- `WorldChunk`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `HeightField`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `VisibilitySet`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `SceneGraph`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `IChunkLoader`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.

## Critério de finalização
- alinhamento de posição e altura
- p95 frame time ao trocar setor
- memória limitada por janela de mundo
Não liberar produção nem assinalar progressão real do MMO sem teste no cliente/servidor autorizado e revisão de titularidade.
