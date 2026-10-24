# Requisitos — Toolchain C++/Emscripten e runtime WebAssembly

## Requisitos funcionais (RF)
- **RF-01**: Fixar versão/flags da toolchain Emscripten e matriz debug/release. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar binding ABI simples com typed buffers e error codes. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Criar loop cooperativo requestAnimationFrame com pause/resume. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Medir memória, exceções, heap growth, threads e COOP/COEP opt-in. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Criar testes WASM automatizados em browser e comparação nativo↔WASM. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 module factory async:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 API exportada C estável:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 memory lifecycle:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 JS↔WASM typed buffers:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar paridade de unit tests; tempo de inicialização medido; peak linear memory sob orçamento definido.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de builds diferentes por emsdk
- Mitigar risco de OOM e pointers inválidos
- Mitigar risco de browser bloqueia SharedArrayBuffer
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
