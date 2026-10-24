# Registro de riscos — Baseline do cliente Windows e seleção validada da Angelica

## Riscos específicos
| R-1 | repositório incompleto | Alto / validar gravidade | Expor cenário negativo e observável | Construir inventário estático de solutions, arquivos e bibliotecas da fonte 1.5.3/1.5.2 | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | SDK legado indisponível | Alto / validar gravidade | Expor cenário negativo e observável | Criar matriz de compatibilidade D3D8/9, headers, ABI, asset e protocolo por versão | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | cliente não autentica em servidor atual | Alto / validar gravidade | Expor cenário negativo e observável | Compilar cliente e engine em ambiente Windows autorizado com logs reproduzíveis | Bloquear trabalho dependente, ADR ou fallback |

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
