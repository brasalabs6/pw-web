# Plano de implementação — Operações contínuas, versionamento, conteúdo e manutenção

## Ordem das workorders (cada uma corresponde a PR/uma prova)
### WO-01 — Especificar versão semântica cliente/asset/protocolo/realm e migração segura
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** VersionMatrix, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** migrar dados jogadores de forma errada.

### WO-02 — Criar catálogo de incidentes, suporte e telemetria minimizada
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** IncidentRecord, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** incompatibilidade patch/version.

### WO-03 — Implementar flags de rollout e checagem de compatibilidade em ambiente local
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** PlayerSupportCase, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** custos de banda/infra e abuso.

### WO-04 — Projetar manutenção programada, backup, rollback e escalabilidade medida
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** MaintenanceWindow, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** migrar dados jogadores de forma errada.

### WO-05 — Auditar incidentes e autorizar transição para beta/produção com checklist
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** FeatureFlag, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** incompatibilidade patch/version.

## Estratégia de entrega incremental
- Primeiro mocks/test doubles e formatos neutros/sintéticos; segundo interfaces; terceiro prova real autorizada; quarto comparação; quinto hardening.
- Cada arquivo alterado precisa estar dentro do `write_scopes` da workorder; `.github`, `.agents/automation`, `references/` e secrets são áreas protegidas.
- PR descreve explicitamente o que funciona, o que está faltando e como provar.
- Falha em qualquer gate leva a reparo limitado (máximo 3 tentativas), depois triagem humana.

## Integração entre subsistemas
- `VersionMatrix`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `IncidentRecord`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `PlayerSupportCase`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `MaintenanceWindow`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `FeatureFlag`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.

## Critério de finalização
- rollbacks sem perda indevida
- tempo de resposta incidente medido
- compatibilidade por release
Não liberar produção nem assinalar progressão real do MMO sem teste no cliente/servidor autorizado e revisão de titularidade.
