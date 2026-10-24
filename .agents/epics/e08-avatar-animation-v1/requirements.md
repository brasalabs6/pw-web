# Requisitos — Avatares, esqueletos, animações e customização

## Requisitos funcionais (RF)
- **RF-01**: Especificar bones, transforms, clips, markers e blend trees com fixtures próprias. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar amostragem e interpolação determinística de animação. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar skinning em shader WebGL2 com fallback CPU e testes. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Implementar montagem de equipamento, aparência e máquina de estados. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Validar avatar/autorização, caminhada, ataque, montaria e voo em comparação. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 Skeleton:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 SkinMesh:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 AnimationClip:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 AvatarAppearance:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 AnimationStateMachine:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar erro de pose dentro de tolerância; custo ms/avatar; 100 personagens simulados no teste de carga.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de formato binário incompatível
- Mitigar risco de hierarquias de bones divergentes
- Mitigar risco de diferença GPU/CPU skinning e retargeting
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
