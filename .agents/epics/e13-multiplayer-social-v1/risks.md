# Registro de riscos — Sincronização MMO, presença, chat, grupo e guilda

## Riscos específicos
| R-1 | pacotes fora de ordem | Alto / validar gravidade | Expor cenário negativo e observável | Definir snapshots/deltas/clock e idempotência de evento por entidade | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | vazamento de chat ou identidade | Alto / validar gravidade | Expor cenário negativo e observável | Implementar interpolação e reconciliation em peers mockados | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | AOI e tempo real sobrecarregam frame | Alto / validar gravidade | Expor cenário negativo e observável | Implementar AOI, canais e filas limitadas do gateway de testes | Bloquear trabalho dependente, ADR ou fallback |

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
