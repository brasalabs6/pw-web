# Requisitos — UI do jogo, HUD, entrada, câmera e áudio

## Requisitos funcionais (RF)
- **RF-01**: Documentar inventário de telas, fluxos UI e inputs com baseline autorizado. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar mapeamento de ações e câmera sem prender foco do browser. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Criar shell responsivo HUD/inventário/chat/configurações com dados mockados. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Integrar Web Audio lifecycle/volume/spatialization e suspensão por aba. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Validar interação e acessibilidade, toque, fullscreen e fluxos reais do cliente. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 InputAction:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 HUDState:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 CameraController:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 AudioEvent:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 UIMessageBus:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar contratos de UX reproduzíveis; latência input→frame; nenhum asset de áudio sem autorização.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de web browser consome teclas/scroll
- Mitigar risco de latência de UI
- Mitigar risco de recurso sonoro com direitos incertos
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
