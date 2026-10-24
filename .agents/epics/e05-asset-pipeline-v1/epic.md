# Epic — Pipeline de recursos PCK/PKX, formatos e proveniência

Preparar uma variante de asset confirmada, com parsing limitado, manifesto íntegro e preload assíncrono antes do uso.

## Resultado esperado

Parser de uma variante, manifesto e preload limitados; OPFS/CDN completo continua em e15.

## Limites

Uma workorder por patch/PR. Artefatos de laboratório ficam em artifacts; evidência
sanitizada fica em docs/evidence. Não alterar executor, referências fixadas ou
contratos de outros repositórios. POC e scene sintética não são gameplay real.

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
