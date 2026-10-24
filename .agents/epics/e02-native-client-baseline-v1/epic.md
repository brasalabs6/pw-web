# Epic — Baseline do cliente Windows e seleção validada da Angelica

Obter uma baseline Windows reproduzível e evidência suficiente para selecionar versão, SDKs, assets e realm sem misturar candidatos.

## Resultado esperado

Baseline, inventário, matriz SDK/ABI/recursos e ADR de seleção; não entrega o port browser.

## Limites

Uma workorder por patch/PR. Artefatos de laboratório ficam em artifacts; evidência
sanitizada fica em docs/evidence. Não alterar executor, referências fixadas ou
contratos de outros repositórios. POC e scene sintética não são gameplay real.

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
