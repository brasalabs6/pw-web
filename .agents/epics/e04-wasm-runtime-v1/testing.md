# Testes — Toolchain C++/Emscripten e runtime WebAssembly

Positivo e negativo por workorder são obrigatórios. Ferramenta ausente significa
BLOCKED; teste executado com falha significa FAIL. Build sozinho não é prova runtime.

## WO-01

- Positivo: Com toolchain declarada, build e smoke são reproduzíveis; não exige baixar referências.
- Negativo: SDK ausente/versão diferente é reportado explicitamente e não resulta em sucesso por skip.
- Comando/ação: `./scripts/check-wasm-toolchain.sh (criar); ./scripts/build-wasm.sh; ./scripts/verify.sh`.

## WO-02

- Positivo: Typed buffer é interpretado com layout de 32 bits acordado; o round-trip real preserva os bytes.
- Negativo: Ponteiro desalinhado, intervalo fora da memória e length com overflow retornam erro antes de acesso.
- Comando/ação: `node --test tests/runtime/bridge.test.mjs (criar; carregar WASM real)`.

## WO-03

- Positivo: Após pausa longa, resume não simula o tempo oculto inteiro nem agenda dois loops.
- Negativo: Callback atrasado após dispose e timestamp regressivo não executam tick inválido.
- Comando/ação: `node --test tests/runtime/frame-loop.test.mjs (criar) + autoteste browser de visibilidade`.

## WO-04

- Positivo: Relatório contém cenário, dispositivo, limites observados e lacunas; nenhuma capacidade de thread é presumida.
- Negativo: Medição indisponível aparece como indisponível, não zero; software WebGL não é benchmark de GPU física.
- Comando/ação: `node tests/runtime/memory-benchmark.mjs (criar) + medição no browser identificado`.

## WO-05

- Positivo: Gate documenta execução real WASM e rejeição de baseline errada; POC-001 sozinha não conclui a WO.
- Negativo: Build bem-sucedido sem import/execução ou suite com zero casos impede PASS.
- Comando/ação: `./scripts/build-wasm.sh; node --test tests/probes/compare.test.mjs; runner browser de WO-10; ./scripts/verify.sh`.

## WO-06

- Positivo: Bridge/loop/bootstrap podem ser implementados sem decidir schemas ou lifetime durante o código.
- Negativo: Nenhum campo any/ponteiro sem limite ou erro não enumerado é aceito na revisão de prontidão.
- Comando/ação: `Revisão de exemplos/transições e ./scripts/verify.sh`.

## WO-07

- Positivo: Dados sobrevivem ao crescimento e nenhuma view obsoleta é usada pelo consumidor.
- Negativo: View antiga/detached e intervalo agora inválido não geram leitura silenciosa de zero.
- Comando/ação: `node --test tests/runtime/memory-growth.test.mjs (criar)`.

## WO-08

- Positivo: Consumidor só recebe instância pronta e uma inicialização ativa; erro não vira rótulo genérico de sucesso.
- Negativo: Resposta tardia após dispose e módulo com export ausente são rejeitados.
- Comando/ação: `node --test tests/runtime/loader.test.mjs (criar) + requests do autoteste browser`.

## WO-09

- Positivo: Após falha/dispose não há ticks, listeners ativos nem referência retida a buffers liberados.
- Negativo: Segundo dispose é seguro; erro durante startup não deixa half-ready utilizável.
- Comando/ação: `node --test tests/runtime/dispose.test.mjs (criar)`.

## WO-10

- Positivo: O runner falha automaticamente nos casos negativos e passa no browser com ambiente identificado.
- Negativo: Página que fica RUNNING para sempre termina em timeout/falha, não em teste pulado.
- Comando/ação: `./scripts/test-wasm-browser.sh (criar nesta WO); ./scripts/verify.sh`.

## Evidência

Registrar comando, versão, SHA, fixture, resultado e limites em docs/evidence;
logs/artifacts locais não substituem um resultado verificável. Comandos “criar” são
entregáveis futuros; registrar zero casos ou screenshot isolada não satisfaz aceite.
`./scripts/verify.sh` continua obrigatório e não valida o cliente completo.
