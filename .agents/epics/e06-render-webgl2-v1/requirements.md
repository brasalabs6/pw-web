# Requisitos — Renderer WebGL2 da engine Angelica

## Requisitos funcionais (RF)
- **RF-01**: Inventariar render states realmente usados na Angelica2 e dependências D3D8. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Definir tabelas de equivalência de vertex formats e estados blend/cull/depth. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar backend WebGL2 incremental com shaders do subconjunto utilizado. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Integrar materiais, texturas, render targets, transparência e context restoration. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Validar paridade visual com screenshot Windows e GPU capture autorizados. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 IRenderDevice:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 ITexture:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 IMeshBuffer:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 IPipelineState:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 IMaterial:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar golden images com tolerância registrada; frametime e draw calls; recursos GPU liberados sem vazamento.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de semântica D3D↔GL divergente
- Mitigar risco de shaders D3DX indisponíveis
- Mitigar risco de perda do contexto GPU e leaking buffers
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
