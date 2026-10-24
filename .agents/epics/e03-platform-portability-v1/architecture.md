# Arquitetura — Desacoplamento de Win32, SDK e Direct3D

Caller → tipos/validação R0 → adapters de relógio, path e bytes disponíveis → core portátil. DOM e scheduler ficam no host. Os adapters não conhecem Direct3D, registry ou protocolo de rede; erros atravessam a fronteira explicitamente.

## Donos e consumidores

Tipos e adapters próprios com erros definidos; inventário da closure e gate de portabilidade limitado. O pw-web é dono dos novos contratos de plataforma; o servidor
é dono do wire format; referências são evidência, não dependências automáticas do
build público. Recursos possuem owner/lifetime explícitos; erro não é sucesso vazio.

## Caminhos de falha

- WO-01: Arquivo duplicado ou #if desativado não recebe classificação de código ativo sem evidência.
- WO-02: Pedido de socket raw, registry ou ActiveX é marcado unsupported/fora da fatia, não simulado.
- WO-03: Leitura fora do intervalo ou caminho rejeitado interrompe a operação sem dados parciais tratados como sucesso.
- WO-04: Eventos atrasados após dispose e tecla solta fora da janela não mantêm input preso.
- WO-05: Um único caso obrigatório pulado impede PASS agregado.
- WO-06: NaN, overflow da transformação, count inválido, viewport zero ou índice fora do limite retornam InvalidInput sem escrita parcial.
- WO-07: Relógio regressivo/NaN é rejeitado; não se corrige usando Date.now sem autorização do contrato.
- WO-08: A/../B, C:\x, UNC e NUL retornam erro explícito, sem normalização permissiva.
- WO-09: Blur com botão pressionado limpa estado e eventos pós-dispose são ignorados.
- WO-10: Offset+length que estoura inteiro ou ultrapassa o buffer falha antes de acessar memória.
- WO-11: Borrow sobrevivendo ao dispose ou ao memory.grow é explicitamente inválido e tem teste especificado.
- WO-12: Caso sentinela em teste isolado demonstra que o harness detecta erro; sentinela não entra em produção.

## Dependências reais

Ver dependencies dos JSONs e o grafo em [port-program.md](../../port-program.md).
Não há requisito global de compilar todo Windows antes de R0 sintético. Integração
original e paridade final exigem a baseline, closure e autorização correspondentes.

## Contratos canônicos

- [.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md](../../../.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md)
- [docs/ANGELICA-PORTING-METHOD.md](../../../docs/ANGELICA-PORTING-METHOD.md)
