# Requisitos — Operações contínuas, versionamento, conteúdo e manutenção

## Requisitos funcionais (RF)
- **RF-01**: Especificar versão semântica cliente/asset/protocolo/realm e migração segura. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Criar catálogo de incidentes, suporte e telemetria minimizada. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar flags de rollout e checagem de compatibilidade em ambiente local. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Projetar manutenção programada, backup, rollback e escalabilidade medida. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Auditar incidentes e autorizar transição para beta/produção com checklist. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 VersionMatrix:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 IncidentRecord:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 PlayerSupportCase:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 MaintenanceWindow:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 FeatureFlag:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar rollbacks sem perda indevida; tempo de resposta incidente medido; compatibilidade por release.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de migrar dados jogadores de forma errada
- Mitigar risco de incompatibilidade patch/version
- Mitigar risco de custos de banda/infra e abuso
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
