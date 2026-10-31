# Requisitos — Toolchain C++/Emscripten e runtime WebAssembly

Cada requisito abaixo tem uma workorder proprietária. Resultado mensurável prevalece
sobre presença de arquivos. Schema ou ambiente ausente mantém a tarefa bloqueada.

- **RF-01 / WO-01:** Com toolchain declarada, build e smoke são reproduzíveis; não exige baixar referências.
- **RF-02 / WO-02:** Typed buffer é interpretado com layout de 32 bits acordado; o round-trip real preserva os bytes.
- **RF-03 / WO-03:** Após pausa longa, resume não simula o tempo oculto inteiro nem agenda dois loops.
- **RF-04 / WO-04:** Relatório contém cenário, dispositivo, limites observados e lacunas; nenhuma capacidade de thread é presumida.
- **RF-05 / WO-05:** Gate documenta execução real WASM e rejeição de baseline errada; POC-001 sozinha não conclui a WO.
- **RF-06 / WO-06:** Bridge/loop/bootstrap podem ser implementados sem decidir schemas ou lifetime durante o código.
- **RF-07 / WO-07:** Dados sobrevivem ao crescimento e nenhuma view obsoleta é usada pelo consumidor.
- **RF-08 / WO-08:** Consumidor só recebe instância pronta e uma inicialização ativa; erro não vira rótulo genérico de sucesso.
- **RF-09 / WO-09:** Após falha/dispose não há ticks, listeners ativos nem referência retida a buffers liberados.
- **RF-10 / WO-10:** O runner falha automaticamente nos casos negativos e passa no browser com ambiente identificado.

## Requisitos transversais

- Sem fontes/assets proprietários no build público sem decisão documentada.
- Mesmo caso reproduzível no ambiente declarado; todos os testes obrigatórios contam.
- Input inválido, lifetime e limites tratados antes de efeitos irreversíveis.
- Comandos planejados não são evidência; saída real com versão/SHA é obrigatória.
