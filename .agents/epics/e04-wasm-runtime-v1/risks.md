# Registro de riscos — Toolchain C++/Emscripten e runtime WebAssembly

## Riscos específicos
| R-1 | builds diferentes por emsdk | Alto / validar gravidade | Expor cenário negativo e observável | Fixar versão/flags da toolchain Emscripten e matriz debug/release | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | OOM e pointers inválidos | Alto / validar gravidade | Expor cenário negativo e observável | Implementar binding ABI simples com typed buffers e error codes | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | browser bloqueia SharedArrayBuffer | Alto / validar gravidade | Expor cenário negativo e observável | Criar loop cooperativo requestAnimationFrame com pause/resume | Bloquear trabalho dependente, ADR ou fallback |

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
