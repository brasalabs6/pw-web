# Plano de implementação — Orquestração autônoma, isolamento e governança de agentes

## Ordem das workorders (cada uma corresponde a PR/uma prova)
### WO-01 — Definir máquina de estados, schemas, escopos, dependências e critérios de evidência
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** workorder JSON + allowlist, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** prompt injection via PR e logs.

### WO-02 — Implementar execução manual isolada, secret boundary e limites de patch
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** Grok Build patch artifact, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** chave API exposta a código não confiável.

### WO-03 — Implementar controlador CI/feedback, limites de retries e parada segura
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** CI report.json, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** loops, race conditions e custo descontrolado.

### WO-04 — Adicionar fila determinística programada, anti-duplicação e orçamento/concurrency
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** PR comments e controladores workflow_run, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** prompt injection via PR e logs.

### WO-05 — Testar incidentes, rollback, falha de API, injection e auditoria dos agentes
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** workorder JSON + allowlist, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** chave API exposta a código não confiável.

## Estratégia de entrega incremental
- Primeiro mocks/test doubles e formatos neutros/sintéticos; segundo interfaces; terceiro prova real autorizada; quarto comparação; quinto hardening.
- Cada arquivo alterado precisa estar dentro do `write_scopes` da workorder; `.github`, `.agents/automation`, `references/` e secrets são áreas protegidas.
- PR descreve explicitamente o que funciona, o que está faltando e como provar.
- Falha em qualquer gate leva a reparo limitado (máximo 3 tentativas), depois triagem humana.

## Integração entre subsistemas
- `workorder JSON + allowlist`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `Grok Build patch artifact`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `CI report.json`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `PR comments e controladores workflow_run`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.

## Critério de finalização
- 100% patches validados fora do job com chave
- tentativas por workorder <=3
- 100% PRs com log e hash de CI
Não liberar produção nem assinalar progressão real do MMO sem teste no cliente/servidor autorizado e revisão de titularidade.
