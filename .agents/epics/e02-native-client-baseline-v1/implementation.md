# Implementação — Baseline do cliente Windows e seleção validada da Angelica

Solutions/configuração → inventário de fontes/SDKs → laboratório limpo → bibliotecas → linker → executável por SHA → cena/handshake → ADR. O relatório pertence ao pw-web; o laboratório fornece a evidência, e as referências continuam imutáveis.

## Ordem

Seguir o DAG do JSON, não a ordem numérica dos arquivos. IDs antigos são estáveis;
WO-05 continua sendo gate terminal mesmo quando depende de WO-06 ou posteriores.
Implementação espera o contrato da dependência e ferramenta real disponível.

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

## Processo

Ler o JSON da tarefa e seus inputs; executar WI-01 a WI-04; escrever teste RED para
comportamento novo; implementar GREEN; revisar diff e anexar evidência. Nunca alterar
o controller/testes para fabricar sucesso. Follow @implementation-loop e @code-review.
Apenas o mantenedor atualiza checkboxes/manifesto após homologação com evidência.
