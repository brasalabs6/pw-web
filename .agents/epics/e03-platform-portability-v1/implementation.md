# Implementação — Desacoplamento de Win32, SDK e Direct3D

Caller → tipos/validação R0 → adapters de relógio, path e bytes disponíveis → core portátil. DOM e scheduler ficam no host. Os adapters não conhecem Direct3D, registry ou protocolo de rede; erros atravessam a fronteira explicitamente.

## Ordem

Seguir o DAG do JSON, não a ordem numérica dos arquivos. IDs antigos são estáveis;
WO-05 continua sendo gate terminal mesmo quando depende de WO-06 ou posteriores.
Implementação espera o contrato da dependência e ferramenta real disponível.

| WO | Entrega | Automação | Dependências |
| --- | --- | --- | --- |
| [WO-01](workorders/wo-01.md) | Inventariar includes, APIs e escapes de plataforma por módulo | assistida | e02-native-client-baseline-v1/wo-01 |
| [WO-02](workorders/wo-02.md) | Fechar contratos de plataforma e erros sem stubs de sucesso | sim, após gates | e03-platform-portability-v1/wo-01, e01-agent-orchestration-v1/wo-05 |
| [WO-03](workorders/wo-03.md) | Integrar adapters portáteis de tempo, caminhos e leitura em memória | sim, após gates | e03-platform-portability-v1/wo-07, e03-platform-portability-v1/wo-08, e03-platform-portability-v1/wo-10, e03-platform-portability-v1/wo-11, e01-agent-orchestration-v1/wo-05 |
| [WO-04](workorders/wo-04.md) | Integrar input e scheduler sem dependências Win32 no core | sim, após gates | e03-platform-portability-v1/wo-03, e03-platform-portability-v1/wo-09, e04-wasm-runtime-v1/wo-03, e01-agent-orchestration-v1/wo-05 |
| [WO-05](workorders/wo-05.md) | Homologar portabilidade e limites Win32 remanescentes | sim, após gates | e03-platform-portability-v1/wo-04, e03-platform-portability-v1/wo-06, e03-platform-portability-v1/wo-12, e01-agent-orchestration-v1/wo-05 |
| [WO-06](workorders/wo-06.md) | Implementar P1: ABI fixa, matriz clip e cor do contrato R0 | sim, após gates | e04-wasm-runtime-v1/wo-01, e01-agent-orchestration-v1/wo-05 |
| [WO-07](workorders/wo-07.md) | Implementar adapter de relógio monotônico injetável | sim, após gates | e03-platform-portability-v1/wo-02, e01-agent-orchestration-v1/wo-05 |
| [WO-08](workorders/wo-08.md) | Implementar validação de caminhos lógicos e fronteira de locale | sim, após gates | e03-platform-portability-v1/wo-02, e01-agent-orchestration-v1/wo-05 |
| [WO-09](workorders/wo-09.md) | Implementar snapshots de input e reset de foco | sim, após gates | e03-platform-portability-v1/wo-02, e01-agent-orchestration-v1/wo-05 |
| [WO-10](workorders/wo-10.md) | Implementar leitura limitada de buffers já carregados | sim, após gates | e03-platform-portability-v1/wo-02, e01-agent-orchestration-v1/wo-05 |
| [WO-11](workorders/wo-11.md) | Definir ownership e política de alocação para a fatia | sim, após gates | e03-platform-portability-v1/wo-02, e01-agent-orchestration-v1/wo-05 |
| [WO-12](workorders/wo-12.md) | Auditar closure portátil com compilação e sanitizers | sim, após gates | e03-platform-portability-v1/wo-03, e03-platform-portability-v1/wo-06, e01-agent-orchestration-v1/wo-05 |

## Processo

Ler o JSON da tarefa e seus inputs; executar WI-01 a WI-04; escrever teste RED para
comportamento novo; implementar GREEN; revisar diff e anexar evidência. Nunca alterar
o controller/testes para fabricar sucesso. Follow @implementation-loop e @code-review.
Apenas o mantenedor atualiza checkboxes/manifesto após homologação com evidência.
