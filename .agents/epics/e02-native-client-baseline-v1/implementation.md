# Plano de implementação — Baseline do cliente Windows e seleção validada da Angelica

## Ordem das workorders (cada uma corresponde a PR/uma prova)
### WO-01 — Construir inventário estático de solutions, arquivos e bibliotecas da fonte 1.5.3/1.5.2
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** matriz de compiladores e SDK, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** repositório incompleto.

### WO-02 — Criar matriz de compatibilidade D3D8/9, headers, ABI, asset e protocolo por versão
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** log/sha de executável, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** SDK legado indisponível.

### WO-03 — Compilar cliente e engine em ambiente Windows autorizado com logs reproduzíveis
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** matriz client↔assets↔protocol↔server, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** cliente não autentica em servidor atual.

### WO-04 — Verificar inicialização, carregamento da cena, handshake e comportamento visual
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** matriz de compiladores e SDK, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** repositório incompleto.

### WO-05 — Registrar ADR de seleção final, fallback e bloqueios por componente
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** log/sha de executável, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** SDK legado indisponível.

## Estratégia de entrega incremental
- Primeiro mocks/test doubles e formatos neutros/sintéticos; segundo interfaces; terceiro prova real autorizada; quarto comparação; quinto hardening.
- Cada arquivo alterado precisa estar dentro do `write_scopes` da workorder; `.github`, `.agents/automation`, `references/` e secrets são áreas protegidas.
- PR descreve explicitamente o que funciona, o que está faltando e como provar.
- Falha em qualquer gate leva a reparo limitado (máximo 3 tentativas), depois triagem humana.

## Integração entre subsistemas
- `matriz de compiladores e SDK`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `log/sha de executável`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `matriz client↔assets↔protocol↔server`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.

## Critério de finalização
- reprodução com toolchain documentada
- 0 erros não documentados no build qualificado
- versão por screenshot/hash/protocolo
Não liberar produção nem assinalar progressão real do MMO sem teste no cliente/servidor autorizado e revisão de titularidade.
