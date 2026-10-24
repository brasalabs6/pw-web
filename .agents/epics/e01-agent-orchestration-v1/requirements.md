# Requisitos — Orquestração autônoma, isolamento e governança de agentes

## Requisitos funcionais (RF)
- **RF-01**: Definir máquina de estados, schemas, escopos, dependências e critérios de evidência. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar execução manual isolada, secret boundary e limites de patch. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar controlador CI/feedback, limites de retries e parada segura. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Adicionar fila determinística programada, anti-duplicação e orçamento/concurrency. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Testar incidentes, rollback, falha de API, injection e auditoria dos agentes. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 workorder JSON + allowlist:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 Codex patch artifact:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 CI report.json:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 PR comments e controladores workflow_run:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar 100% patches validados fora do job com chave; tentativas por workorder <=3; 100% PRs com log e hash de CI.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de prompt injection via PR e logs
- Mitigar risco de chave API exposta a código não confiável
- Mitigar risco de loops, race conditions e custo descontrolado
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
