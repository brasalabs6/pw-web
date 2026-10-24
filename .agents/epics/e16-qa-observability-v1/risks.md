# Registro de riscos — Estratégia de testes, tracing, replay e QA automatizado

## Riscos específicos

| ID | Risco | Gravidade | Como identificar | Primeira mitigação | Plano de contingência |
| --- | --- | --- | --- | --- | --- |
| R-1 | falso verde do scaffold | Alto / validar gravidade | Expor cenário negativo e observável | Criar matriz de testes por camada, severidade e definição de done | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | fixtures com direitos incertos | Alto / validar gravidade | Expor cenário negativo e observável | Automatizar fixtures sintéticas, fuzz, ASAN/UBSAN e testes de ABI | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | testes flaky e builds não determinísticas | Alto / validar gravidade | Expor cenário negativo e observável | Criar harness browser E2E WebGL/WASM/viewport e screenshot diffs | Bloquear trabalho dependente, ADR ou fallback |

## Legenda da tabela
Formato: ID | risco | gravidade inicial | sinal de falha | primeira mitigação | contingency.

## Riscos transversais
| Categoria | Sinal | Tratamento |
| --- | --- | --- |
| Direitos | licença titular desconhecido | manter fora de artefatos distribuídos |
| Integridade | checks verdes sem funcionalidade real | evidência E2E específica e gate humano |
| Segurança do agente | mudança em workflows/segredos | rejeitar patch antes de write-token |
| Compatibilidade | assets/protocolos de versões diferentes | matriz de versão e validação por commit |
| Performance | stutter/memória e download excessivo | perf budgets, tracing e opções de fallback |
| Operação | falta de credenciais/servidor/SDK | bloquear honestamente, não simular sucesso |

## Registro de incidente e decisão
Cada risco precisa de dono, impacto, likelihood revisada, evidência, mitigação, prazo de revisão e critério de encerramento. Erros críticos exigem suspensão de release e workorder de correção.
