# Arquitetura — Toolchain C++/Emscripten e runtime WebAssembly

Loader assíncrono → instância WASM pronta → bridge validada → tick cooperativo → host. Runtime é dono da instância; caller possui inputs e o contrato define quando views são recriadas. Failed/disposed impedem novos ticks; recuperação é inicialização explícita.

## Donos e consumidores

Runtime próprio validado; POC-001 é evidência parcial, não conclusão automática de WO-05. O pw-web é dono dos novos contratos de plataforma; o servidor
é dono do wire format; referências são evidência, não dependências automáticas do
build público. Recursos possuem owner/lifetime explícitos; erro não é sucesso vazio.

## Caminhos de falha

- WO-01: SDK ausente/versão diferente é reportado explicitamente e não resulta em sucesso por skip.
- WO-02: Ponteiro desalinhado, intervalo fora da memória e length com overflow retornam erro antes de acesso.
- WO-03: Callback atrasado após dispose e timestamp regressivo não executam tick inválido.
- WO-04: Medição indisponível aparece como indisponível, não zero; software WebGL não é benchmark de GPU física.
- WO-05: Build bem-sucedido sem import/execução ou suite com zero casos impede PASS.
- WO-06: Nenhum campo any/ponteiro sem limite ou erro não enumerado é aceito na revisão de prontidão.
- WO-07: View antiga/detached e intervalo agora inválido não geram leitura silenciosa de zero.
- WO-08: Resposta tardia após dispose e módulo com export ausente são rejeitados.
- WO-09: Segundo dispose é seguro; erro durante startup não deixa half-ready utilizável.
- WO-10: Página que fica RUNNING para sempre termina em timeout/falha, não em teste pulado.

## Dependências reais

Ver dependencies dos JSONs e o grafo em [port-program.md](../../port-program.md).
Não há requisito global de compilar todo Windows antes de R0 sintético. Integração
original e paridade final exigem a baseline, closure e autorização correspondentes.

## Contratos canônicos

- [docs/POC-ANGELICA.md](../../../docs/POC-ANGELICA.md)
- [docs/ANGELICA-PORTING-METHOD.md](../../../docs/ANGELICA-PORTING-METHOD.md)
