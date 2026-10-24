# Plano de implementação — GNET, protocolo de mundo e gateway WSS seguro

## Ordem das workorders (cada uma corresponde a PR/uma prova)
### WO-01 — Registrar opcode/length/endian por versão e declarar fonte de evidência
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** PacketReader/Writer, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** endianness e framing.

### WO-02 — Implementar leitor/escritor bounds-checked e protocolo mock em memória
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** ProtocolVersion, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** compatibilidade 1.5.3↔realm v155.

### WO-03 — Implementar WSS gateway de testes com limites, origem, sessão e backpressure
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** ClientSession, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** flood/malformed packets e auth bypass.

### WO-04 — Integrar autenticação e transporte do servidor alvo em ambiente local isolado
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** WsGateway, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** endianness e framing.

### WO-05 — Validar captures autorizadas, reconexão, reorder, timeout e semântica GNET
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** PacketTrace, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** compatibilidade 1.5.3↔realm v155.

## Estratégia de entrega incremental
- Primeiro mocks/test doubles e formatos neutros/sintéticos; segundo interfaces; terceiro prova real autorizada; quarto comparação; quinto hardening.
- Cada arquivo alterado precisa estar dentro do `write_scopes` da workorder; `.github`, `.agents/automation`, `references/` e secrets são áreas protegidas.
- PR descreve explicitamente o que funciona, o que está faltando e como provar.
- Falha em qualquer gate leva a reparo limitado (máximo 3 tentativas), depois triagem humana.

## Integração entre subsistemas
- `PacketReader/Writer`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `ProtocolVersion`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `ClientSession`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `WsGateway`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `PacketTrace`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.

## Critério de finalização
- fixtures de ida/volta byte-a-byte
- max payload definido
- gateway sem exposição do barramento interno
Não liberar produção nem assinalar progressão real do MMO sem teste no cliente/servidor autorizado e revisão de titularidade.
