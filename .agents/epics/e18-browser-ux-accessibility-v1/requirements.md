# Requisitos — Compatibilidade de navegadores, acessibilidade e dispositivos

## Requisitos funcionais (RF)
- **RF-01**: Definir navegadores alvo e matriz API/feature flags com critérios de fallback. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar capacidade/diagnóstico e mensagens específicas ao hardware. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar UX responsivo, foco, legendas, remapeamento de entradas e i18n. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Criar testes de perda de contexto, background tab, storage eviction e touch. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Executar campanha manual em dispositivos reais e corrigir divergências. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 CapabilityMatrix:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 GraphicsFallback:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 AccessibilityState:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 InputBindings:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 LocaleBundle:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar matriz browser/dispositivo preenchida; fluxos HUD navegáveis por teclado; sem crashes após pause/resume.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de WebGL2 não suportado ou driver instável
- Mitigar risco de limites iOS/OPFS
- Mitigar risco de acessibilidade insuficiente e ações presas no browser
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
