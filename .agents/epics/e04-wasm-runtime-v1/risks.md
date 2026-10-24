# Riscos — Toolchain C++/Emscripten e runtime WebAssembly

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


## Condições de parada

Não inferir schema, autorização ou paridade. Manter as tarefas independentes
executáveis enquanto a parte dependente fica BLOCKED. Retomar com evidência da
pré-condição; não remover gates apenas para alimentar o scheduler.
