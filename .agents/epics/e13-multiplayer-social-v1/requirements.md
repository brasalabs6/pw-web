# Requisitos — Sincronização MMO, presença, chat, grupo e guilda

## Requisitos funcionais (RF)
- **RF-01**: Definir snapshots/deltas/clock e idempotência de evento por entidade. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar interpolação e reconciliation em peers mockados. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar AOI, canais e filas limitadas do gateway de testes. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Integrar grupo, guilda, chat e presença ao servidor com proteção de dados. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Validar multi-client, desconexão, spam, teleportes e consistência social. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 EntitySnapshot:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 AOISubscription:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 ChannelMessage:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 PartyRoster:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 GuildState:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar 2 clientes sincronizados sob cenários de movimentação; limite de msgs/s; ressincronização após perda.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de pacotes fora de ordem
- Mitigar risco de vazamento de chat ou identidade
- Mitigar risco de AOI e tempo real sobrecarregam frame
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
