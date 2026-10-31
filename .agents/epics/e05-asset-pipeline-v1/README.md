# e05-asset-pipeline-v1

**Pipeline de recursos PCK/PKX, formatos e proveniência · revisão 1.1.0 · status: planejado**

Preparar uma variante de asset confirmada, com parsing limitado, manifesto íntegro e preload assíncrono antes do uso.

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

## Estado e proveniência

Revisão 1.1.0 do plano, 2026-10-24. Nenhuma destas workorders foi concluída por
esta atualização. POC-001 é evidência local restrita; não equivale à homologação
dos gates de cliente/renderer. [provenance: user-input, doc-tree, code]

## Riscos e incertezas

- [HIGH][Confirmed] Layouts de todas as famílias ainda não estão materializados em contratos implementáveis.
- [HIGH][Possible] Offset >2GiB, overflow e bomba de descompressão podem quebrar wasm32.
- [HIGH][Possible] Conversão de encoding/case pode gerar colisão de identidade ou traversal.

(HUMAN) Fornecer assets autorizados e decidir uso/distribuição em WO-05. Testes sintéticos não aprovam direitos nem compatibilidade de versão.

## Contratos

- [docs/LEGAL.md](../../../docs/LEGAL.md)
- [docs/ANGELICA-PORTING-METHOD.md](../../../docs/ANGELICA-PORTING-METHOD.md)

Contratos futuros em `docs/contracts/` são entregáveis das WOs de especificação.
Sua ausência bloqueia os consumidores; o agente não deve inventar um substituto.
