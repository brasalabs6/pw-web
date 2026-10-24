# Plano de implementação — Avatares, esqueletos, animações e customização

## Ordem das workorders (cada uma corresponde a PR/uma prova)
### WO-01 — Especificar bones, transforms, clips, markers e blend trees com fixtures próprias
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** Skeleton, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** formato binário incompatível.

### WO-02 — Implementar amostragem e interpolação determinística de animação
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** SkinMesh, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** hierarquias de bones divergentes.

### WO-03 — Implementar skinning em shader WebGL2 com fallback CPU e testes
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** AnimationClip, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** diferença GPU/CPU skinning e retargeting.

### WO-04 — Implementar montagem de equipamento, aparência e máquina de estados
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** AvatarAppearance, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** formato binário incompatível.

### WO-05 — Validar avatar/autorização, caminhada, ataque, montaria e voo em comparação
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** AnimationStateMachine, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** hierarquias de bones divergentes.

## Estratégia de entrega incremental
- Primeiro mocks/test doubles e formatos neutros/sintéticos; segundo interfaces; terceiro prova real autorizada; quarto comparação; quinto hardening.
- Cada arquivo alterado precisa estar dentro do `write_scopes` da workorder; `.github`, `.agents/automation`, `references/` e secrets são áreas protegidas.
- PR descreve explicitamente o que funciona, o que está faltando e como provar.
- Falha em qualquer gate leva a reparo limitado (máximo 3 tentativas), depois triagem humana.

## Integração entre subsistemas
- `Skeleton`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `SkinMesh`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `AnimationClip`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `AvatarAppearance`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `AnimationStateMachine`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.

## Critério de finalização
- erro de pose dentro de tolerância
- custo ms/avatar
- 100 personagens simulados no teste de carga
Não liberar produção nem assinalar progressão real do MMO sem teste no cliente/servidor autorizado e revisão de titularidade.
