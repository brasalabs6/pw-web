# Registro de riscos — Desacoplamento de Win32, SDK e Direct3D

## Riscos específicos

| ID | Risco | Gravidade | Como identificar | Primeira mitigação | Plano de contingência |
| --- | --- | --- | --- | --- | --- |
| R-1 | dependências ocultas em headers transversais | Alto / validar gravidade | Expor cenário negativo e observável | Automatizar inventário de includes/API macros e grafo de módulos originais | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | ABI packing e UTF/GBK | Alto / validar gravidade | Expor cenário negativo e observável | Especificar contratos de plataforma e stubs sem comportamento de jogo | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | semântica de threads/cancelamento divergente | Alto / validar gravidade | Expor cenário negativo e observável | Implementar adapters portáteis de tempo, memória, caminhos e arquivos virtuais | Bloquear trabalho dependente, ADR ou fallback |

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
