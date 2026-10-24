# Registro de riscos — Orquestração autônoma, isolamento e governança de agentes

## Riscos específicos

| ID | Risco | Gravidade | Como identificar | Primeira mitigação | Plano de contingência |
| --- | --- | --- | --- | --- | --- |
| R-1 | prompt injection via PR e logs | Alto / validar gravidade | Expor cenário negativo e observável | Definir máquina de estados, schemas, escopos, dependências e critérios de evidência | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | chave API exposta a código não confiável | Alto / validar gravidade | Expor cenário negativo e observável | Implementar execução manual isolada, secret boundary e limites de patch | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | loops, race conditions e custo descontrolado | Alto / validar gravidade | Expor cenário negativo e observável | Implementar controlador CI/feedback, limites de retries e parada segura | Bloquear trabalho dependente, ADR ou fallback |

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
