# Requisitos — Streaming, CDN, cache OPFS e desempenho do navegador

## Requisitos funcionais (RF)
- **RF-01**: Definir budgets alvo por browser/dispositivo e manifesto de assets permitidos. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar fetch Range/retry/sha/abort com mock HTTP server. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar OPFS/IndexedDB cache versionado e LRU com cotas. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Priorizar visibilidade, pré-busca, decode em worker e limites VRAM. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Rodar perfil real com pacotes autorizados e rede lenta/perda de conexão. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 ManifestVersion:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 ChunkRequest:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 AssetPriorityQueue:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 CacheEntry:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 PerfSample:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar bytes até primeiro frame; p95 frame time durante streaming; taxa hit cache, espaço consumido.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de pacote monolítico multigigabyte
- Mitigar risco de cache corrompido
- Mitigar risco de queda de FPS em streaming e memória estourada
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
