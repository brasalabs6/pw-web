# Requisitos — Desacoplamento de Win32, SDK e Direct3D

Cada requisito abaixo tem uma workorder proprietária. Resultado mensurável prevalece
sobre presença de arquivos. Schema ou ambiente ausente mantém a tarefa bloqueada.

- **RF-01 / WO-01:** O inventário identifica acessos D3D fora de A3DDevice e não trata contagem textual como call graph completo.
- **RF-02 / WO-02:** WO-07 a WO-10 recebem assinaturas/fixtures fechadas e falhas explícitas; não existe retorno success sem efeito.
- **RF-03 / WO-03:** Bytes carregados antes do tick produzem o mesmo resultado nativo/WASM e recursos são liberados ao finalizar.
- **RF-04 / WO-04:** Pressionar, perder foco e retornar gera snapshots e deltas definidos, com um único tick por frame.
- **RF-05 / WO-05:** Gate wo-05 libera apenas a plataforma testada e registra capacidades não suportadas com evidência.
- **RF-06 / WO-06:** Todos os valores esperados em R0 coincidem com tolerância 1e-6; índices/cores são exatos e layout tem 16 bytes.
- **RF-07 / WO-07:** A sequência conhecida de timestamps produz os deltas exatos e o reset especificado.
- **RF-08 / WO-08:** Caminho lógico permitido mantém identidade estável; nenhuma entrada consegue escapar do namespace de assets.
- **RF-09 / WO-09:** Uma sequência press/move/release gera snapshot esperado sem duplicação entre frames.
- **RF-10 / WO-10:** Leituras parciais válidas retornam bytes/cursor esperados nativo/WASM, sem fetch dentro do parser.
- **RF-11 / WO-11:** Cada recurso tem um único owner e momento de liberação; nenhuma API depende de estado global oculto.
- **RF-12 / WO-12:** A closure nova compila sem Windows/D3D e as suites não produzem violações de memória/UB.

## Requisitos transversais

- Sem fontes/assets proprietários no build público sem decisão documentada.
- Mesmo caso reproduzível no ambiente declarado; todos os testes obrigatórios contam.
- Input inválido, lifetime e limites tratados antes de efeitos irreversíveis.
- Comandos planejados não são evidência; saída real com versão/SHA é obrigatória.
