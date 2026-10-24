# e02-native-client-baseline-v1

**Baseline do cliente Windows e seleção validada da Angelica · revisão 1.1.0 · status: planejado**

Obter uma baseline Windows reproduzível e evidência suficiente para selecionar versão, SDKs, assets e realm sem misturar candidatos.

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
| [WO-01](workorders/wo-01.md) | Construir inventário estático de solutions, arquivos e bibliotecas 1.5.3/1.5.2 | assistida |  |
| [WO-02](workorders/wo-02.md) | Criar matriz cliente, SDK, ABI, assets e protocolo por versão | assistida | e02-native-client-baseline-v1/wo-01 |
| [WO-03](workorders/wo-03.md) | Compilar cliente Windows limpo contra engine e bibliotecas verificadas | assistida | e02-native-client-baseline-v1/wo-07, e02-native-client-baseline-v1/wo-08 |
| [WO-04](workorders/wo-04.md) | Validar inicialização, cena e handshake da baseline Windows | assistida | e02-native-client-baseline-v1/wo-03, e02-native-client-baseline-v1/wo-09 |
| [WO-05](workorders/wo-05.md) | Decidir seleção de versão com evidência, fallback e bloqueios | assistida | e02-native-client-baseline-v1/wo-04, e02-native-client-baseline-v1/wo-10, e00-rights-governance-v1/wo-02 |
| [WO-06](workorders/wo-06.md) | Provisionar matriz reproduzível de VS2005 e SDKs no laboratório | assistida | e02-native-client-baseline-v1/wo-02 |
| [WO-07](workorders/wo-07.md) | Reconstruir bibliotecas Angelica/Common na ordem de dependência | assistida | e02-native-client-baseline-v1/wo-06 |
| [WO-08](workorders/wo-08.md) | Resolver paths de build, exports e closure de link de Release Win32 | assistida | e02-native-client-baseline-v1/wo-07 |
| [WO-09](workorders/wo-09.md) | Identificar recursos autorizados e realm da baseline | assistida | e02-native-client-baseline-v1/wo-02, e00-rights-governance-v1/wo-02 |
| [WO-10](workorders/wo-10.md) | Comparar alternativa 1.5.2 moderna sem misturar árvores | assistida | e02-native-client-baseline-v1/wo-01 |

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
