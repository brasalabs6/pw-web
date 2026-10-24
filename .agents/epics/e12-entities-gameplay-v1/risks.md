# Registro de riscos — Movimento, combate, skills, NPCs, quests e interação

## Riscos específicos
| R-1 | cliente altera estado servidor-autoritativo | Alto / validar gravidade | Expor cenário negativo e observável | Definir máquina de entidades e componentes com fixtures sem IP proprietário | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | desync movimento/skill | Alto / validar gravidade | Expor cenário negativo e observável | Implementar comando de movimento, colisão e predição/correção em simulação | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | protocol opcode incompleto | Alto / validar gravidade | Expor cenário negativo e observável | Implementar targeting, cast timeline e feedback visual de skills mockadas | Bloquear trabalho dependente, ADR ou fallback |

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
