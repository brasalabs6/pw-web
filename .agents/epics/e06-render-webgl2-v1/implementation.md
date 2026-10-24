# Implementação — Renderer WebGL2 da engine Angelica

DrawView validado → backend C++ GLES3/Emscripten → WebGL2 → readPixels. R0 define unlit/indexed; R1 fecha textura/sampler/targets antes de implementação. Caller mantém dados durante consumo; backend possui recursos GPU e os invalida em context loss.

## Ordem

Seguir o DAG do JSON, não a ordem numérica dos arquivos. IDs antigos são estáveis;
WO-05 continua sendo gate terminal mesmo quando depende de WO-06 ou posteriores.
Implementação espera o contrato da dependência e ferramenta real disponível.

| WO | Entrega | Automação | Dependências |
| --- | --- | --- | --- |
| [WO-01](workorders/wo-01.md) | Inventariar estados D3D e escapes usados pela primeira fatia | assistida | e03-platform-portability-v1/wo-01 |
| [WO-02](workorders/wo-02.md) | Fechar equivalência de vertex formats, matrizes e estados | assistida | e06-render-webgl2-v1/wo-01, e03-platform-portability-v1/wo-06 |
| [WO-03](workorders/wo-03.md) | Implementar P2: primeiro desenho indexado GLES3/WebGL2 | sim, após gates | e03-platform-portability-v1/wo-06, e04-wasm-runtime-v1/wo-01, e01-agent-orchestration-v1/wo-05 |
| [WO-04](workorders/wo-04.md) | Integrar materiais, texturas, transparência e render targets | sim, após gates | e06-render-webgl2-v1/wo-08, e06-render-webgl2-v1/wo-09, e06-render-webgl2-v1/wo-11, e06-render-webgl2-v1/wo-12, e06-render-webgl2-v1/wo-15, e01-agent-orchestration-v1/wo-05 |
| [WO-05](workorders/wo-05.md) | Homologar paridade visual contra baseline Windows autorizada | assistida | e06-render-webgl2-v1/wo-04, e06-render-webgl2-v1/wo-14, e06-render-webgl2-v1/wo-16, e02-native-client-baseline-v1/wo-04, e05-asset-pipeline-v1/wo-05 |
| [WO-06](workorders/wo-06.md) | Definir matriz de capacidades suportadas e erros do backend | sim, após gates | e06-render-webgl2-v1/wo-03, e01-agent-orchestration-v1/wo-05 |
| [WO-07](workorders/wo-07.md) | Especificar R1 de textura RGBA8 e sampler mínimo | sim, após gates | e06-render-webgl2-v1/wo-03, e06-render-webgl2-v1/wo-06, e01-agent-orchestration-v1/wo-05 |
| [WO-08](workorders/wo-08.md) | Implementar upload e desenho texturizado de R1 | sim, após gates | e06-render-webgl2-v1/wo-07, e01-agent-orchestration-v1/wo-05 |
| [WO-09](workorders/wo-09.md) | Implementar blend, alpha test, cull e depth do subconjunto aprovado | sim, após gates | e06-render-webgl2-v1/wo-02, e06-render-webgl2-v1/wo-06, e06-render-webgl2-v1/wo-08, e01-agent-orchestration-v1/wo-05 |
| [WO-10](workorders/wo-10.md) | Especificar render targets e attachments da primeira fatia | sim, após gates | e06-render-webgl2-v1/wo-06, e01-agent-orchestration-v1/wo-05 |
| [WO-11](workorders/wo-11.md) | Implementar variantes GLSL explícitas do material mínimo | sim, após gates | e06-render-webgl2-v1/wo-08, e06-render-webgl2-v1/wo-09, e01-agent-orchestration-v1/wo-05 |
| [WO-12](workorders/wo-12.md) | Implementar framebuffer, resize e readback conforme R1 | sim, após gates | e06-render-webgl2-v1/wo-10, e06-render-webgl2-v1/wo-08, e01-agent-orchestration-v1/wo-05 |
| [WO-13](workorders/wo-13.md) | Fechar dependências do primeiro draw original da Angelica | assistida | e06-render-webgl2-v1/wo-01, e06-render-webgl2-v1/wo-03, e02-native-client-baseline-v1/wo-03 |
| [WO-14](workorders/wo-14.md) | Adaptar o primeiro draw original para o backend browser | assistida | e06-render-webgl2-v1/wo-13, e06-render-webgl2-v1/wo-02, e06-render-webgl2-v1/wo-03, e00-rights-governance-v1/wo-02 |
| [WO-15](workorders/wo-15.md) | Implementar restauração de contexto e descarte de recursos | sim, após gates | e06-render-webgl2-v1/wo-03, e06-render-webgl2-v1/wo-06, e01-agent-orchestration-v1/wo-05 |
| [WO-16](workorders/wo-16.md) | Medir renderer e preparar gate visual com evidência reproduzível | sim, após gates | e06-render-webgl2-v1/wo-04, e01-agent-orchestration-v1/wo-05 |

## Processo

Ler o JSON da tarefa e seus inputs; executar WI-01 a WI-04; escrever teste RED para
comportamento novo; implementar GREEN; revisar diff e anexar evidência. Nunca alterar
o controller/testes para fabricar sucesso. Follow @implementation-loop e @code-review.
Apenas o mantenedor atualiza checkboxes/manifesto após homologação com evidência.
