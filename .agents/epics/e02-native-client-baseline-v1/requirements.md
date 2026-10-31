# Requisitos — Baseline do cliente Windows e seleção validada da Angelica

Cada requisito abaixo tem uma workorder proprietária. Resultado mensurável prevalece
sobre presença de arquivos. Schema ou ambiente ausente mantém a tarefa bloqueada.

- **RF-01 / WO-01:** Inventário identifica os 21 projetos das soluções principais 1.5.3 e explica dependências por configuração, sem inferir sucesso do linker.
- **RF-02 / WO-02:** A matriz distingue presença de .lib, compatibilidade ABI, build e execução; nenhum campo pendente aparece como compatível.
- **RF-03 / WO-03:** Um executável reconstruído de fontes e dependências identificadas tem SHA e build reproduzível em diretório vazio.
- **RF-04 / WO-04:** Cada capacidade possui resultado observado e artefato; somente entrada real no mundo satisfaz a capacidade mundo.
- **RF-05 / WO-05:** ADR aceita ou rejeitada contém evidências e decisão identificada; o gate terminal continua significando seleção validada.
- **RF-06 / WO-06:** Preflight distingue compilador, headers e linker disponíveis e passa no laboratório usado em WO-03.
- **RF-07 / WO-07:** Mapa de linker permite identificar produtor e arquitetura de cada biblioteca consumida pelo cliente.
- **RF-08 / WO-08:** Todos os caminhos e símbolos de Release têm resolução demonstrada ou bloqueio nomeado antes de WO-03.
- **RF-09 / WO-09:** WO-04 recebe manifesto sanitizado de recursos e destino de laboratório cuja compatibilidade será testada.
- **RF-10 / WO-10:** Relatório explica custo e evidência de cada alternativa sem declarar 1.5.2 funcional por README.

## Requisitos transversais

- Sem fontes/assets proprietários no build público sem decisão documentada.
- Mesmo caso reproduzível no ambiente declarado; todos os testes obrigatórios contam.
- Input inválido, lifetime e limites tratados antes de efeitos irreversíveis.
- Comandos planejados não são evidência; saída real com versão/SHA é obrigatória.
