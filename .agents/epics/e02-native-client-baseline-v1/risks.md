# Riscos — Baseline do cliente Windows e seleção validada da Angelica

## Estado e proveniência

Revisão 1.1.0 do plano, 2026-10-24. Nenhuma destas workorders foi concluída por
esta atualização. POC-001 é evidência local restrita; não equivale à homologação
dos gates de cliente/renderer. [provenance: user-input, doc-tree, code]

## Riscos e incertezas

- [HIGH][Confirmed] VS2005/SDKs não disponíveis no host da POC; build Windows ainda não demonstrado.
- [HIGH][Confirmed] Configurações misturam caminhos Angelica2/Angelica2.2; escolher Release Win32 coerente.
- [HIGH][Possible] Bibliotecas encontradas pelo nome podem ter ABI/CRT incompatíveis.

(HUMAN) Provisionar Windows/SDKs e recursos autorizados (WO-06/09); decidir versão/direitos no gate WO-05. Bloqueia laboratório e seleção, não matemática própria.

## Contratos

- [docs/VALIDATION.md](../../../docs/VALIDATION.md)
- [docs/DECISIONS/ADR-0003-version-selection.md](../../../docs/DECISIONS/ADR-0003-version-selection.md)

Contratos futuros em `docs/contracts/` são entregáveis das WOs de especificação.
Sua ausência bloqueia os consumidores; o agente não deve inventar um substituto.


## Condições de parada

Não inferir schema, autorização ou paridade. Manter as tarefas independentes
executáveis enquanto a parte dependente fica BLOCKED. Retomar com evidência da
pré-condição; não remover gates apenas para alimentar o scheduler.
