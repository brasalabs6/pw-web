# Plano de implementação — Pipeline de recursos PCK/PKX, formatos e proveniência

## Ordem das workorders (cada uma corresponde a PR/uma prova)
### WO-01 — Catalogar magic/versionamento e limites de PCK/PKX e famílias de arquivos
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** IArchiveIndex, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** arquivos >2GB e offsets truncados.

### WO-02 — Especificar parsing streaming e GBK→Unicode sem confiar em nomes de caminho
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** AssetManifest, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** bomba de descompressão e path traversal.

### WO-03 — Implementar parsers defensivos em fixtures sintéticas e fuzzer de entrada
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** IAssetDecoder, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** incompatibilidade de versões e redistribuição.

### WO-04 — Gerar manifestos versionados com hashes, tamanhos, offsets e dependências
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** resource version map, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** arquivos >2GB e offsets truncados.

### WO-05 — Validar casos reais apenas com assets autorizados e decisão de distribuição
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** IArchiveIndex, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** bomba de descompressão e path traversal.

## Estratégia de entrega incremental
- Primeiro mocks/test doubles e formatos neutros/sintéticos; segundo interfaces; terceiro prova real autorizada; quarto comparação; quinto hardening.
- Cada arquivo alterado precisa estar dentro do `write_scopes` da workorder; `.github`, `.agents/automation`, `references/` e secrets são áreas protegidas.
- PR descreve explicitamente o que funciona, o que está faltando e como provar.
- Falha em qualquer gate leva a reparo limitado (máximo 3 tentativas), depois triagem humana.

## Integração entre subsistemas
- `IArchiveIndex`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `AssetManifest`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `IAssetDecoder`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `resource version map`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.

## Critério de finalização
- fixtures sintéticas com limites validados
- 0 path escapes
- 100% arquivos emitidos com hash e versão
Não liberar produção nem assinalar progressão real do MMO sem teste no cliente/servidor autorizado e revisão de titularidade.
