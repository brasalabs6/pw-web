# Registro de riscos — Compatibilidade de navegadores, acessibilidade e dispositivos

## Riscos específicos
| R-1 | WebGL2 não suportado ou driver instável | Alto / validar gravidade | Expor cenário negativo e observável | Definir navegadores alvo e matriz API/feature flags com critérios de fallback | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | limites iOS/OPFS | Alto / validar gravidade | Expor cenário negativo e observável | Implementar capacidade/diagnóstico e mensagens específicas ao hardware | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | acessibilidade insuficiente e ações presas no browser | Alto / validar gravidade | Expor cenário negativo e observável | Implementar UX responsivo, foco, legendas, remapeamento de entradas e i18n | Bloquear trabalho dependente, ADR ou fallback |

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
