# Registro de riscos — Streaming, CDN, cache OPFS e desempenho do navegador

## Riscos específicos

| ID | Risco | Gravidade | Como identificar | Primeira mitigação | Plano de contingência |
| --- | --- | --- | --- | --- | --- |
| R-1 | pacote monolítico multigigabyte | Alto / validar gravidade | Expor cenário negativo e observável | Definir budgets alvo por browser/dispositivo e manifesto de assets permitidos | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | cache corrompido | Alto / validar gravidade | Expor cenário negativo e observável | Implementar fetch Range/retry/sha/abort com mock HTTP server | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | queda de FPS em streaming e memória estourada | Alto / validar gravidade | Expor cenário negativo e observável | Implementar OPFS/IndexedDB cache versionado e LRU com cotas | Bloquear trabalho dependente, ADR ou fallback |

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
