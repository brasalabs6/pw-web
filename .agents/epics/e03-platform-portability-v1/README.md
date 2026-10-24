# e03-platform-portability-v1

**Desacoplamento de Win32, SDK e Direct3D · revisão 1.1.0 · status: planejado**

Isolar as dependências de plataforma da primeira fatia e provar tipos, matemática, leitura e input com testes nativo/WASM.

## Escopo

- **ADICIONAR:** workorders específicas, fixtures/erros e pré-condições por capacidade.
- **REFACTORIZAR:** detalhar o plano v1 preservando IDs 01–05 e o significado do gate terminal WO-05.
- **REMOVER:** instruções genéricas de stubs/falso sucesso; não remover código nem evidência histórica.

**12 workorders / 48 workitems**. Os workitems estão nos Markdown
correspondentes e em [tasks.md](tasks.md); o dispatcher continua lendo somente JSON
`schema_version: 1`. [Plano técnico](../../../docs/ANGELICA-PORTING-METHOD.md).

[Charter](epic.md) · [Requisitos](requirements.md) · [Arquitetura](architecture.md) ·
[Implementação](implementation.md) · [Testes](testing.md) · [Riscos](risks.md) ·
[Tasks](tasks.md) · [Resumo](task.md).

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

## Estado e proveniência

Revisão 1.1.0 do plano, 2026-10-24. Nenhuma destas workorders foi concluída por
esta atualização. POC-001 é evidência local restrita; não equivale à homologação
dos gates de cliente/renderer. [provenance: user-input, doc-tree, code]

## Riscos e incertezas

- [HIGH][Confirmed] DWORD é unsigned long: 8 bytes neste Linux versus 4 em wasm32.
- [HIGH][Confirmed] GetD3DDevice escapa da fachada; port não se resolve substituindo só A3DDevice.
- [HIGH][Likely] Views/ponteiros retidos além do lifetime causam falhas após growth/dispose.

(HUMAN) Aprovar autorização antes de integrar código legado; P1 e adapters próprios não precisam copiar referências. Threads/áudio sem contrato permanecem fora da fatia.

## Contratos

- [.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md](../../../.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md)
- [docs/ANGELICA-PORTING-METHOD.md](../../../docs/ANGELICA-PORTING-METHOD.md)

Contratos futuros em `docs/contracts/` são entregáveis das WOs de especificação.
Sua ausência bloqueia os consumidores; o agente não deve inventar um substituto.
