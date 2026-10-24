# Registro de riscos — UI do jogo, HUD, entrada, câmera e áudio

## Riscos específicos
| R-1 | web browser consome teclas/scroll | Alto / validar gravidade | Expor cenário negativo e observável | Documentar inventário de telas, fluxos UI e inputs com baseline autorizado | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | latência de UI | Alto / validar gravidade | Expor cenário negativo e observável | Implementar mapeamento de ações e câmera sem prender foco do browser | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | recurso sonoro com direitos incertos | Alto / validar gravidade | Expor cenário negativo e observável | Criar shell responsivo HUD/inventário/chat/configurações com dados mockados | Bloquear trabalho dependente, ADR ou fallback |

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
