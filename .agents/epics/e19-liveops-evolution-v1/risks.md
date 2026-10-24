# Registro de riscos — Operações contínuas, versionamento, conteúdo e manutenção

## Riscos específicos
| R-1 | migrar dados jogadores de forma errada | Alto / validar gravidade | Expor cenário negativo e observável | Especificar versão semântica cliente/asset/protocolo/realm e migração segura | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | incompatibilidade patch/version | Alto / validar gravidade | Expor cenário negativo e observável | Criar catálogo de incidentes, suporte e telemetria minimizada | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | custos de banda/infra e abuso | Alto / validar gravidade | Expor cenário negativo e observável | Implementar flags de rollout e checagem de compatibilidade em ambiente local | Bloquear trabalho dependente, ADR ou fallback |

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
