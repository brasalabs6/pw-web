# Registro de riscos — Pipeline de recursos PCK/PKX, formatos e proveniência

## Riscos específicos

| ID | Risco | Gravidade | Como identificar | Primeira mitigação | Plano de contingência |
| --- | --- | --- | --- | --- | --- |
| R-1 | arquivos >2GB e offsets truncados | Alto / validar gravidade | Expor cenário negativo e observável | Catalogar magic/versionamento e limites de PCK/PKX e famílias de arquivos | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | bomba de descompressão e path traversal | Alto / validar gravidade | Expor cenário negativo e observável | Especificar parsing streaming e GBK→Unicode sem confiar em nomes de caminho | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | incompatibilidade de versões e redistribuição | Alto / validar gravidade | Expor cenário negativo e observável | Implementar parsers defensivos em fixtures sintéticas e fuzzer de entrada | Bloquear trabalho dependente, ADR ou fallback |

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
