# Plano de implementação — Direitos, proveniência, licenciamento e decisões de produto

## Ordem das workorders (cada uma corresponde a PR/uma prova)
### WO-01 — Catalogar arquivos, bibliotecas, símbolos e conteúdos protegidos por origem e versão
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** registro origem→hash→titular→uso permitido, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** licença de terceiro não comprova titularidade.

### WO-02 — Validar direitos de reprodução/modificação/distribuição e termos comerciais por titular
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** gate de publicação por artefato, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** redistribuição pública de assets.

### WO-03 — Desenhar estratégia de implementação independente para componentes sem autorização
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** planos de substituição por componente, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** licença incompatível com distribuição comercial.

### WO-04 — Construir SBOM, política de attribution e gates de PR/build/release
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** registro origem→hash→titular→uso permitido, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** licença de terceiro não comprova titularidade.

### WO-05 — Aprovar matriz de distribuição, marcas, privacy e revisão periódica
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** gate de publicação por artefato, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** redistribuição pública de assets.

## Estratégia de entrega incremental
- Primeiro mocks/test doubles e formatos neutros/sintéticos; segundo interfaces; terceiro prova real autorizada; quarto comparação; quinto hardening.
- Cada arquivo alterado precisa estar dentro do `write_scopes` da workorder; `.github`, `.agents/automation`, `references/` e secrets são áreas protegidas.
- PR descreve explicitamente o que funciona, o que está faltando e como provar.
- Falha em qualquer gate leva a reparo limitado (máximo 3 tentativas), depois triagem humana.

## Integração entre subsistemas
- `registro origem→hash→titular→uso permitido`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `gate de publicação por artefato`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `planos de substituição por componente`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.

## Critério de finalização
- 100% componentes distribuídos autorizados
- zero binários/arquivos originais no Git
- documentos de aprovação por release
Não liberar produção nem assinalar progressão real do MMO sem teste no cliente/servidor autorizado e revisão de titularidade.
