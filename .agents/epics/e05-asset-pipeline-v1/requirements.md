# Requisitos — Pipeline de recursos PCK/PKX, formatos e proveniência

## Requisitos funcionais (RF)
- **RF-01**: Catalogar magic/versionamento e limites de PCK/PKX e famílias de arquivos. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Especificar parsing streaming e GBK→Unicode sem confiar em nomes de caminho. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar parsers defensivos em fixtures sintéticas e fuzzer de entrada. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Gerar manifestos versionados com hashes, tamanhos, offsets e dependências. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Validar casos reais apenas com assets autorizados e decisão de distribuição. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 IArchiveIndex:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 AssetManifest:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 IAssetDecoder:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 resource version map:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar fixtures sintéticas com limites validados; 0 path escapes; 100% arquivos emitidos com hash e versão.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de arquivos >2GB e offsets truncados
- Mitigar risco de bomba de descompressão e path traversal
- Mitigar risco de incompatibilidade de versões e redistribuição
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
