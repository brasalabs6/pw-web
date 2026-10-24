# Requisitos — Baseline do cliente Windows e seleção validada da Angelica

## Requisitos funcionais (RF)
- **RF-01**: Construir inventário estático de solutions, arquivos e bibliotecas da fonte 1.5.3/1.5.2. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Criar matriz de compatibilidade D3D8/9, headers, ABI, asset e protocolo por versão. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Compilar cliente e engine em ambiente Windows autorizado com logs reproduzíveis. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Verificar inicialização, carregamento da cena, handshake e comportamento visual. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Registrar ADR de seleção final, fallback e bloqueios por componente. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 matriz de compiladores e SDK:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 log/sha de executável:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 matriz client↔assets↔protocol↔server:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar reprodução com toolchain documentada; 0 erros não documentados no build qualificado; versão por screenshot/hash/protocolo.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de repositório incompleto
- Mitigar risco de SDK legado indisponível
- Mitigar risco de cliente não autentica em servidor atual
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
