# Plano de implementação — Renderer WebGL2 da engine Angelica

## Ordem das workorders (cada uma corresponde a PR/uma prova)
### WO-01 — Inventariar render states realmente usados na Angelica2 e dependências D3D8
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** IRenderDevice, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** semântica D3D↔GL divergente.

### WO-02 — Definir tabelas de equivalência de vertex formats e estados blend/cull/depth
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** ITexture, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** shaders D3DX indisponíveis.

### WO-03 — Implementar backend WebGL2 incremental com shaders do subconjunto utilizado
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** IMeshBuffer, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** perda do contexto GPU e leaking buffers.

### WO-04 — Integrar materiais, texturas, render targets, transparência e context restoration
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** IPipelineState, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** semântica D3D↔GL divergente.

### WO-05 — Validar paridade visual com screenshot Windows e GPU capture autorizados
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** IMaterial, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** shaders D3DX indisponíveis.

## Estratégia de entrega incremental
- Primeiro mocks/test doubles e formatos neutros/sintéticos; segundo interfaces; terceiro prova real autorizada; quarto comparação; quinto hardening.
- Cada arquivo alterado precisa estar dentro do `write_scopes` da workorder; `.github`, `.agents/automation`, `references/` e secrets são áreas protegidas.
- PR descreve explicitamente o que funciona, o que está faltando e como provar.
- Falha em qualquer gate leva a reparo limitado (máximo 3 tentativas), depois triagem humana.

## Integração entre subsistemas
- `IRenderDevice`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `ITexture`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `IMeshBuffer`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `IPipelineState`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `IMaterial`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.

## Critério de finalização
- golden images com tolerância registrada
- frametime e draw calls
- recursos GPU liberados sem vazamento
Não liberar produção nem assinalar progressão real do MMO sem teste no cliente/servidor autorizado e revisão de titularidade.
