# Registro de riscos — GNET, protocolo de mundo e gateway WSS seguro

## Riscos específicos

| ID | Risco | Gravidade | Como identificar | Primeira mitigação | Plano de contingência |
| --- | --- | --- | --- | --- | --- |
| R-1 | endianness e framing | Alto / validar gravidade | Expor cenário negativo e observável | Registrar opcode/length/endian por versão e declarar fonte de evidência | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | compatibilidade 1.5.3↔realm v155 | Alto / validar gravidade | Expor cenário negativo e observável | Implementar leitor/escritor bounds-checked e protocolo mock em memória | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | flood/malformed packets e auth bypass | Alto / validar gravidade | Expor cenário negativo e observável | Implementar WSS gateway de testes com limites, origem, sessão e backpressure | Bloquear trabalho dependente, ADR ou fallback |

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
