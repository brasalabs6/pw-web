# Registro de riscos — Identidade, login, personagem e sessão

## Riscos específicos
| R-1 | sessão roubada | Alto / validar gravidade | Expor cenário negativo e observável | Especificar sequência login/realm/lista/personagem/mundo e estados de erro | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | ticket em URL ou log | Alto / validar gravidade | Expor cenário negativo e observável | Implementar máquina de estados com mocks e sanitização de segredos | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | duplicidade de login e replay | Alto / validar gravidade | Expor cenário negativo e observável | Criar frontend login/seleção com estados loading/error/logout e cancelamento | Bloquear trabalho dependente, ADR ou fallback |

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
