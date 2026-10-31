# Arquitetura — Baseline do cliente Windows e seleção validada da Angelica

Solutions/configuração → inventário de fontes/SDKs → laboratório limpo → bibliotecas → linker → executável por SHA → cena/handshake → ADR. O relatório pertence ao pw-web; o laboratório fornece a evidência, e as referências continuam imutáveis.

## Donos e consumidores

Baseline, inventário, matriz SDK/ABI/recursos e ADR de seleção; não entrega o port browser. O pw-web é dono dos novos contratos de plataforma; o servidor
é dono do wire format; referências são evidência, não dependências automáticas do
build público. Recursos possuem owner/lifetime explícitos; erro não é sucesso vazio.

## Caminhos de falha

- WO-01: Projeto com path ausente, macro e case diferente produz três classificações distintas; nenhum é silenciosamente descartado.
- WO-02: A diferença DWORD Linux=8/wasm32=4 da POC permanece explícita; nome da pasta não é evidência de versão.
- WO-03: Biblioteca ausente ou máquina incompatível produz FAIL/BLOCKED; artefato antigo não conta como build novo.
- WO-04: Cliente que abre janela mas falha login permanece parcial e não libera seleção definitiva por suposição.
- WO-05: Sem build ou direitos, manter BLOCKED/proposto; não marcar wo-05 concluída para liberar outros épicos.
- WO-06: VS de arquitetura errada ou d3dx8 ausente bloqueia antes do build do cliente.
- WO-07: Uma lib encontrada pelo nome com CRT/arquitetura incompatível é rejeitada, não reutilizada silenciosamente.
- WO-08: Mistura acidental com Angelica2.2 ou SDK x64 é identificada na revisão.
- WO-09: Arquivo com versão ou hash diferente bloqueia o cenário; presença de um .pck qualquer não é suficiente.
- WO-10: Headers/libs entre versões nunca são combinados como solução para missing symbols.

## Dependências reais

Ver dependencies dos JSONs e o grafo em [port-program.md](../../port-program.md).
Não há requisito global de compilar todo Windows antes de R0 sintético. Integração
original e paridade final exigem a baseline, closure e autorização correspondentes.

## Contratos canônicos

- [docs/VALIDATION.md](../../../docs/VALIDATION.md)
- [docs/DECISIONS/ADR-0003-version-selection.md](../../../docs/DECISIONS/ADR-0003-version-selection.md)
