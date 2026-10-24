# Requisitos — Estratégia de testes, tracing, replay e QA automatizado

## Requisitos funcionais (RF)
- **RF-01**: Criar matriz de testes por camada, severidade e definição de done. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Automatizar fixtures sintéticas, fuzz, ASAN/UBSAN e testes de ABI. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Criar harness browser E2E WebGL/WASM/viewport e screenshot diffs. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Criar evidência JSON/artifacts, traces e perf budgets versionados. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Executar campanhas de compatibilidade e confrontar builds Windows autorizados. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 TestMatrix:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 EvidenceReport:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 PacketReplay:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 FrameTrace:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 BuildProvenance:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar gates por cenário reais; flakiness registrada; evidências por commit e navegador.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de falso verde do scaffold
- Mitigar risco de fixtures com direitos incertos
- Mitigar risco de testes flaky e builds não determinísticas
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
