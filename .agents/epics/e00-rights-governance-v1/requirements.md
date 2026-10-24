# Requisitos — Direitos, proveniência, licenciamento e decisões de produto

## Requisitos funcionais (RF)
- **RF-01**: Catalogar arquivos, bibliotecas, símbolos e conteúdos protegidos por origem e versão. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Validar direitos de reprodução/modificação/distribuição e termos comerciais por titular. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Desenhar estratégia de implementação independente para componentes sem autorização. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Construir SBOM, política de attribution e gates de PR/build/release. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Aprovar matriz de distribuição, marcas, privacy e revisão periódica. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 registro origem→hash→titular→uso permitido:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 gate de publicação por artefato:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 planos de substituição por componente:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar 100% componentes distribuídos autorizados; zero binários/arquivos originais no Git; documentos de aprovação por release.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de licença de terceiro não comprova titularidade
- Mitigar risco de redistribuição pública de assets
- Mitigar risco de licença incompatível com distribuição comercial
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
