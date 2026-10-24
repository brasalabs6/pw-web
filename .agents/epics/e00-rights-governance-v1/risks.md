# Registro de riscos — Direitos, proveniência, licenciamento e decisões de produto

## Riscos específicos
| R-1 | licença de terceiro não comprova titularidade | Alto / validar gravidade | Expor cenário negativo e observável | Catalogar arquivos, bibliotecas, símbolos e conteúdos protegidos por origem e versão | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | redistribuição pública de assets | Alto / validar gravidade | Expor cenário negativo e observável | Validar direitos de reprodução/modificação/distribuição e termos comerciais por titular | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | licença incompatível com distribuição comercial | Alto / validar gravidade | Expor cenário negativo e observável | Desenhar estratégia de implementação independente para componentes sem autorização | Bloquear trabalho dependente, ADR ou fallback |

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
