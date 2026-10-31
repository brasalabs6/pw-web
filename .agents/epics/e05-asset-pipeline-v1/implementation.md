# Implementação — Pipeline de recursos PCK/PKX, formatos e proveniência

Bytes não confiáveis → reader limitado → índice de variante confirmada → descompressão com budget → nomes lógicos → manifesto/hash → preload → bytes prontos para cena. Contrato do formato vem da evidência do produtor; consumidor não inventa layout.

## Ordem

Seguir o DAG do JSON, não a ordem numérica dos arquivos. IDs antigos são estáveis;
WO-05 continua sendo gate terminal mesmo quando depende de WO-06 ou posteriores.
Implementação espera o contrato da dependência e ferramenta real disponível.

| WO | Entrega | Automação | Dependências |
| --- | --- | --- | --- |
| [WO-01](workorders/wo-01.md) | Catalogar versões e layouts observados dos formatos de assets | assistida | e02-native-client-baseline-v1/wo-01 |
| [WO-02](workorders/wo-02.md) | Fechar contratos de parsing, encoding e manifesto da fatia | assistida | e05-asset-pipeline-v1/wo-01, e05-asset-pipeline-v1/wo-06 |
| [WO-03](workorders/wo-03.md) | Integrar parsers defensivos e fuzzing da variante escolhida | sim, após gates | e05-asset-pipeline-v1/wo-07, e05-asset-pipeline-v1/wo-08, e05-asset-pipeline-v1/wo-09, e05-asset-pipeline-v1/wo-10, e01-agent-orchestration-v1/wo-05 |
| [WO-04](workorders/wo-04.md) | Gerar manifestos determinísticos com hashes e dependências | sim, após gates | e05-asset-pipeline-v1/wo-02, e05-asset-pipeline-v1/wo-03, e01-agent-orchestration-v1/wo-05 |
| [WO-05](workorders/wo-05.md) | Homologar pipeline com recursos reais autorizados | assistida | e05-asset-pipeline-v1/wo-04, e05-asset-pipeline-v1/wo-11, e05-asset-pipeline-v1/wo-12, e00-rights-governance-v1/wo-02 |
| [WO-06](workorders/wo-06.md) | Definir limites de leitura e budgets para entradas binárias | sim, após gates | e01-agent-orchestration-v1/wo-05 |
| [WO-07](workorders/wo-07.md) | Implementar byte reader com limites e endian explícitos | sim, após gates | e05-asset-pipeline-v1/wo-06, e01-agent-orchestration-v1/wo-05 |
| [WO-08](workorders/wo-08.md) | Implementar índice de uma variante PCK/PKX confirmada | sim, após gates | e05-asset-pipeline-v1/wo-02, e05-asset-pipeline-v1/wo-07, e01-agent-orchestration-v1/wo-05 |
| [WO-09](workorders/wo-09.md) | Implementar descompressão com tamanho de saída limitado | sim, após gates | e05-asset-pipeline-v1/wo-02, e05-asset-pipeline-v1/wo-07, e01-agent-orchestration-v1/wo-05 |
| [WO-10](workorders/wo-10.md) | Implementar conversão de nomes e detecção de colisões | sim, após gates | e05-asset-pipeline-v1/wo-02, e03-platform-portability-v1/wo-08, e01-agent-orchestration-v1/wo-05 |
| [WO-11](workorders/wo-11.md) | Validar manifesto e grafo de dependências no consumidor | sim, após gates | e05-asset-pipeline-v1/wo-04, e01-agent-orchestration-v1/wo-05 |
| [WO-12](workorders/wo-12.md) | Implementar pré-carregamento assíncrono limitado da primeira cena | sim, após gates | e05-asset-pipeline-v1/wo-11, e04-wasm-runtime-v1/wo-08, e03-platform-portability-v1/wo-10, e01-agent-orchestration-v1/wo-05 |

## Processo

Ler o JSON da tarefa e seus inputs; executar WI-01 a WI-04; escrever teste RED para
comportamento novo; implementar GREEN; revisar diff e anexar evidência. Nunca alterar
o controller/testes para fabricar sucesso. Follow @implementation-loop e @code-review.
Apenas o mantenedor atualiza checkboxes/manifesto após homologação com evidência.
