# Requisitos — Desacoplamento de Win32, SDK e Direct3D

## Requisitos funcionais (RF)
- **RF-01**: Automatizar inventário de includes/API macros e grafo de módulos originais. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Especificar contratos de plataforma e stubs sem comportamento de jogo. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar adapters portáteis de tempo, memória, caminhos e arquivos virtuais. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Separar input/scheduler/threads e renderer por injeção de interfaces. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Criar testes cruzados e checklist de migração com limites Win32 remanescentes. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 IClock:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 IFileSystem:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 IWindowInput:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 IRenderDevice:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 INetworkTransport:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar zero includes Windows no core; testes de equivalência nativa e browser; matriz APIs substituídas por módulo.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de dependências ocultas em headers transversais
- Mitigar risco de ABI packing e UTF/GBK
- Mitigar risco de semântica de threads/cancelamento divergente
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
