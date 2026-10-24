# Requisitos — Segurança, infraestrutura, CI/CD e entrega

## Requisitos funcionais (RF)
- **RF-01**: Desenhar topologia por ambiente e segurança das portas/rede. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Criar manifests IaC para sandbox sem produção e secrets externos. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar TLS/CSP/CORS/cabeçalhos e limites de ataque do gateway mock. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Definir release pipeline assinado com stage e aprovação jurídica. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Testar incident response, backup/restore, rollback e deploy controlado. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 ReleaseManifest:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 DeploymentConfig:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 HealthCheck:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 SecurityGate:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 RollbackPlan:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar 0 portas internas expostas; releases com SBOM/proveniência; tempo rollback testado.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de expor pw-gs/postgres à internet
- Mitigar risco de supply chain de actions
- Mitigar risco de deploy de conteúdo sem direitos
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
