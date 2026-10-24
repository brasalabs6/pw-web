# Requisitos — Terreno, cidades, vegetação, água e visibilidade do mundo

## Requisitos funcionais (RF)
- **RF-01**: Definir matrizes de coordenadas, units/axis e modelo de chunks/scenegraph. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar heightfield e malha procedural com LOD e stitching. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar quadtree/frustum culling e carregamento de objetos estáticos. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Adicionar estados para água, vegetação, skybox, iluminação e clima. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Comparar cenário autorizado e testar teleporte/streaming sem corruptelas. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 WorldChunk:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 HeightField:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 VisibilitySet:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 SceneGraph:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 IChunkLoader:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar alinhamento de posição e altura; p95 frame time ao trocar setor; memória limitada por janela de mundo.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de coordenadas, escala e eixos divergentes
- Mitigar risco de costuras de terreno
- Mitigar risco de carregamento de chunks gera stutter
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
