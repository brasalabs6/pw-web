# Plano de implementação — Compatibilidade de navegadores, acessibilidade e dispositivos

## Ordem das workorders (cada uma corresponde a PR/uma prova)
### WO-01 — Definir navegadores alvo e matriz API/feature flags com critérios de fallback
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** CapabilityMatrix, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** WebGL2 não suportado ou driver instável.

### WO-02 — Implementar capacidade/diagnóstico e mensagens específicas ao hardware
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** GraphicsFallback, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** limites iOS/OPFS.

### WO-03 — Implementar UX responsivo, foco, legendas, remapeamento de entradas e i18n
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** AccessibilityState, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** acessibilidade insuficiente e ações presas no browser.

### WO-04 — Criar testes de perda de contexto, background tab, storage eviction e touch
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** InputBindings, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** WebGL2 não suportado ou driver instável.

### WO-05 — Executar campanha manual em dispositivos reais e corrigir divergências
1. Verificar dependências e base do código; não inferir funcionamento.
2. Definir interfaces, casos limite, erros estruturados e fixtures legítimas.
3. Implementar menor corte vertical observável sem dependências proprietárias não autorizadas.
4. Criar testes positivos, negativos, regressão e diagnóstico para CI.
5. Documentar como reproduzir, comparar medições e submeter PR (sem merge automático).

**Artefato esperado:** LocaleBundle, teste correspondente e relatório de evidência.
**Sinal de bloqueio:** limites iOS/OPFS.

## Estratégia de entrega incremental
- Primeiro mocks/test doubles e formatos neutros/sintéticos; segundo interfaces; terceiro prova real autorizada; quarto comparação; quinto hardening.
- Cada arquivo alterado precisa estar dentro do `write_scopes` da workorder; `.github`, `.agents/automation`, `references/` e secrets são áreas protegidas.
- PR descreve explicitamente o que funciona, o que está faltando e como provar.
- Falha em qualquer gate leva a reparo limitado (máximo 3 tentativas), depois triagem humana.

## Integração entre subsistemas
- `CapabilityMatrix`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `GraphicsFallback`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `AccessibilityState`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `InputBindings`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.
- `LocaleBundle`: versão declarada; validação de entrada/saída; testes cruzados com vizinhos e mensagens de erro serializáveis.

## Critério de finalização
- matriz browser/dispositivo preenchida
- fluxos HUD navegáveis por teclado
- sem crashes após pause/resume
Não liberar produção nem assinalar progressão real do MMO sem teste no cliente/servidor autorizado e revisão de titularidade.
