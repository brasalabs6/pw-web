# Registro de riscos — Segurança, infraestrutura, CI/CD e entrega

## Riscos específicos
| R-1 | expor pw-gs/postgres à internet | Alto / validar gravidade | Expor cenário negativo e observável | Desenhar topologia por ambiente e segurança das portas/rede | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | supply chain de actions | Alto / validar gravidade | Expor cenário negativo e observável | Criar manifests IaC para sandbox sem produção e secrets externos | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | deploy de conteúdo sem direitos | Alto / validar gravidade | Expor cenário negativo e observável | Implementar TLS/CSP/CORS/cabeçalhos e limites de ataque do gateway mock | Bloquear trabalho dependente, ADR ou fallback |

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
