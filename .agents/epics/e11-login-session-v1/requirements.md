# Requisitos — Identidade, login, personagem e sessão

## Requisitos funcionais (RF)
- **RF-01**: Especificar sequência login/realm/lista/personagem/mundo e estados de erro. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar máquina de estados com mocks e sanitização de segredos. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Criar frontend login/seleção com estados loading/error/logout e cancelamento. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Integrar endpoint auth real autorizado com TLS e validade de sessão. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Testar múltiplas sessões, expirados, rate limiting e entrada no mundo. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 AuthStateMachine:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 SessionTicket:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 CharacterList:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 EnterWorldRequest:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 SessionError:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar sem credencial em client logs; testes de retry/expiry; fluxo login→world repetível.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de sessão roubada
- Mitigar risco de ticket em URL ou log
- Mitigar risco de duplicidade de login e replay
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
