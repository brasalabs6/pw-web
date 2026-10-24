# Registro de riscos — Avatares, esqueletos, animações e customização

## Riscos específicos
| R-1 | formato binário incompatível | Alto / validar gravidade | Expor cenário negativo e observável | Especificar bones, transforms, clips, markers e blend trees com fixtures próprias | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | hierarquias de bones divergentes | Alto / validar gravidade | Expor cenário negativo e observável | Implementar amostragem e interpolação determinística de animação | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | diferença GPU/CPU skinning e retargeting | Alto / validar gravidade | Expor cenário negativo e observável | Implementar skinning em shader WebGL2 com fallback CPU e testes | Bloquear trabalho dependente, ADR ou fallback |

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
