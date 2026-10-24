# Registro de riscos — Renderer WebGL2 da engine Angelica

## Riscos específicos

| ID | Risco | Gravidade | Como identificar | Primeira mitigação | Plano de contingência |
| --- | --- | --- | --- | --- | --- |
| R-1 | semântica D3D↔GL divergente | Alto / validar gravidade | Expor cenário negativo e observável | Inventariar render states realmente usados na Angelica2 e dependências D3D8 | Bloquear trabalho dependente, ADR ou fallback |
| R-2 | shaders D3DX indisponíveis | Alto / validar gravidade | Expor cenário negativo e observável | Definir tabelas de equivalência de vertex formats e estados blend/cull/depth | Bloquear trabalho dependente, ADR ou fallback |
| R-3 | perda do contexto GPU e leaking buffers | Alto / validar gravidade | Expor cenário negativo e observável | Implementar backend WebGL2 incremental com shaders do subconjunto utilizado | Bloquear trabalho dependente, ADR ou fallback |

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
