# Riscos — Renderer WebGL2 da engine Angelica

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


## Condições de parada

Não inferir schema, autorização ou paridade. Manter as tarefas independentes
executáveis enquanto a parte dependente fica BLOCKED. Retomar com evidência da
pré-condição; não remover gates apenas para alimentar o scheduler.
