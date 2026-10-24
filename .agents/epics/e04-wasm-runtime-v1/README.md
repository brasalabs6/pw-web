# e04-wasm-runtime-v1

**Toolchain C++/Emscripten e runtime WebAssembly · revisão 1.1.0 · status: planejado**

Entregar toolchain reproduzível, binding, bootstrap e loop cooperativo, testados no WASM real com falhas observáveis.

## Escopo

- **ADICIONAR:** workorders específicas, fixtures/erros e pré-condições por capacidade.
- **REFACTORIZAR:** detalhar o plano v1 preservando IDs 01–05 e o significado do gate terminal WO-05.
- **REMOVER:** instruções genéricas de stubs/falso sucesso; não remover código nem evidência histórica.

**10 workorders / 40 workitems**. Os workitems estão nos Markdown
correspondentes e em [tasks.md](tasks.md); o dispatcher continua lendo somente JSON
`schema_version: 1`. [Plano técnico](../../../docs/ANGELICA-PORTING-METHOD.md).

[Charter](epic.md) · [Requisitos](requirements.md) · [Arquitetura](architecture.md) ·
[Implementação](implementation.md) · [Testes](testing.md) · [Riscos](risks.md) ·
[Tasks](tasks.md) · [Resumo](task.md).

| WO | Entrega | Automação | Dependências |
| --- | --- | --- | --- |
| [WO-01](workorders/wo-01.md) | Fixar Emscripten e builds reproduzíveis de prova | sim, após gates | e01-agent-orchestration-v1/wo-05 |
| [WO-02](workorders/wo-02.md) | Implementar binding ABI com typed buffers e erros explícitos | sim, após gates | e04-wasm-runtime-v1/wo-01, e04-wasm-runtime-v1/wo-06, e01-agent-orchestration-v1/wo-05 |
| [WO-03](workorders/wo-03.md) | Implementar loop cooperativo com pause, resume e dispose | sim, após gates | e04-wasm-runtime-v1/wo-06, e03-platform-portability-v1/wo-07, e01-agent-orchestration-v1/wo-05 |
| [WO-04](workorders/wo-04.md) | Medir heap, exceções e requisitos de threads sem habilitá-las | sim, após gates | e04-wasm-runtime-v1/wo-02, e04-wasm-runtime-v1/wo-03, e04-wasm-runtime-v1/wo-07, e01-agent-orchestration-v1/wo-05 |
| [WO-05](workorders/wo-05.md) | Homologar testes WASM e paridade nativo/browser | sim, após gates | e04-wasm-runtime-v1/wo-02, e04-wasm-runtime-v1/wo-03, e04-wasm-runtime-v1/wo-04, e04-wasm-runtime-v1/wo-08, e04-wasm-runtime-v1/wo-09, e04-wasm-runtime-v1/wo-10, e01-agent-orchestration-v1/wo-05 |
| [WO-06](workorders/wo-06.md) | Especificar ABI, lifecycle e erros do runtime browser | sim, após gates | e04-wasm-runtime-v1/wo-01, e01-agent-orchestration-v1/wo-05 |
| [WO-07](workorders/wo-07.md) | Recriar views tipadas após crescimento de memória | sim, após gates | e04-wasm-runtime-v1/wo-02, e03-platform-portability-v1/wo-11, e01-agent-orchestration-v1/wo-05 |
| [WO-08](workorders/wo-08.md) | Implementar bootstrap assíncrono com falha visível | sim, após gates | e04-wasm-runtime-v1/wo-06, e01-agent-orchestration-v1/wo-05 |
| [WO-09](workorders/wo-09.md) | Propagar abort e liberar recursos na falha do runtime | sim, após gates | e04-wasm-runtime-v1/wo-02, e04-wasm-runtime-v1/wo-03, e04-wasm-runtime-v1/wo-08, e01-agent-orchestration-v1/wo-05 |
| [WO-10](workorders/wo-10.md) | Automatizar smoke browser com cenários de sucesso e falha | sim, após gates | e04-wasm-runtime-v1/wo-07, e04-wasm-runtime-v1/wo-08, e04-wasm-runtime-v1/wo-09, e01-agent-orchestration-v1/wo-05 |

## Estado e proveniência

Revisão 1.1.0 do plano, 2026-10-24. Nenhuma destas workorders foi concluída por
esta atualização. POC-001 é evidência local restrita; não equivale à homologação
dos gates de cliente/renderer. [provenance: user-input, doc-tree, code]

## Riscos e incertezas

- [HIGH][Confirmed] Build .wasm não prova import, execução ou lifecycle no browser.
- [HIGH][Likely] memory.grow invalida views retidas; dispose com callback tardio pode reativar módulo.
- [MEDIUM][Confirmed] POC rodou em SwiftShader, não comprova desempenho em GPU física.

(HUMAN) Provisionar ferramentas no ambiente de execução se ausentes; mudanças de workflow/provider continuam fora das scopes destes jobs.

## Contratos

- [docs/POC-ANGELICA.md](../../../docs/POC-ANGELICA.md)
- [docs/ANGELICA-PORTING-METHOD.md](../../../docs/ANGELICA-PORTING-METHOD.md)

Contratos futuros em `docs/contracts/` são entregáveis das WOs de especificação.
Sua ausência bloqueia os consumidores; o agente não deve inventar um substituto.
