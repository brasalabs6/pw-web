# e06-render-webgl2-v1

**Renderer WebGL2 da engine Angelica · revisão 1.1.0 · status: planejado**

Construir renderer WebGL2 por subconjuntos e adaptar um primeiro draw Angelica somente após closure, baseline e autorização.

## Escopo

- **ADICIONAR:** workorders específicas, fixtures/erros e pré-condições por capacidade.
- **REFACTORIZAR:** detalhar o plano v1 preservando IDs 01–05 e o significado do gate terminal WO-05.
- **REMOVER:** instruções genéricas de stubs/falso sucesso; não remover código nem evidência histórica.

**16 workorders / 64 workitems**. Os workitems estão nos Markdown
correspondentes e em [tasks.md](tasks.md); o dispatcher continua lendo somente JSON
`schema_version: 1`. [Plano técnico](../../../docs/ANGELICA-PORTING-METHOD.md).

[Charter](epic.md) · [Requisitos](requirements.md) · [Arquitetura](architecture.md) ·
[Implementação](implementation.md) · [Testes](testing.md) · [Riscos](risks.md) ·
[Tasks](tasks.md) · [Resumo](task.md).

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

## Estado e proveniência

Revisão 1.1.0 do plano, 2026-10-24. Nenhuma destas workorders foi concluída por
esta atualização. POC-001 é evidência local restrita; não equivale à homologação
dos gates de cliente/renderer. [provenance: user-input, doc-tree, code]

## Riscos e incertezas

- [HIGH][Confirmed] Há chamadas D3D fora de A3DDevice; closure precisa ser fechada por fatia.
- [HIGH][Likely] Transposição, ARGB, profundidade e winding podem compilar e desenhar errado.
- [HIGH][Possible] IDs antigos após context loss ou shader fallback silencioso criam falso sucesso visual.

(HUMAN) Liberar integração do legado e recursos da baseline (WO-14/05). R0 sintético avança sem assets comerciais; paridade original não.

## Contratos

- [.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md](../../../.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md)
- [docs/DECISIONS/ADR-0004-incremental-port-method.md](../../../docs/DECISIONS/ADR-0004-incremental-port-method.md)

Contratos futuros em `docs/contracts/` são entregáveis das WOs de especificação.
Sua ausência bloqueia os consumidores; o agente não deve inventar um substituto.
