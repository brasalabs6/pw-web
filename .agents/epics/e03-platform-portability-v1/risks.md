# Riscos — Desacoplamento de Win32, SDK e Direct3D

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


## Condições de parada

Não inferir schema, autorização ou paridade. Manter as tarefas independentes
executáveis enquanto a parte dependente fica BLOCKED. Retomar com evidência da
pré-condição; não remover gates apenas para alimentar o scheduler.
