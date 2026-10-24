# Requisitos — Movimento, combate, skills, NPCs, quests e interação

## Requisitos funcionais (RF)
- **RF-01**: Definir máquina de entidades e componentes com fixtures sem IP proprietário. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar comando de movimento, colisão e predição/correção em simulação. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar targeting, cast timeline e feedback visual de skills mockadas. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Integrar NPC/quests/loot/voo com pacotes e semântica de servidor reais. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Executar regressão contra cliente Windows autorizado com divergências anotadas. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 EntityState:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 ActionCommand:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 SkillCast:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 TargetSelection:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 QuestState:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar estado cliente converge com servidor; latência ação→ack medida; regressões por habilidade.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de cliente altera estado servidor-autoritativo
- Mitigar risco de desync movimento/skill
- Mitigar risco de protocol opcode incompleto
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
