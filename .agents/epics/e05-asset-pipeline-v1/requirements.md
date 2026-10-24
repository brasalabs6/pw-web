# Requisitos — Pipeline de recursos PCK/PKX, formatos e proveniência

Cada requisito abaixo tem uma workorder proprietária. Resultado mensurável prevalece
sobre presença de arquivos. Schema ou ambiente ausente mantém a tarefa bloqueada.

- **RF-01 / WO-01:** Cada campo catalogado tem origem verificável; formatos ainda não compreendidos permanecem bloqueados.
- **RF-02 / WO-02:** Cada parser futuro tem formato acordado, erro e exemplo válido/inválido; algoritmo de compressão não é adivinhado.
- **RF-03 / WO-03:** Arquivo válido produz o índice e conteúdo esperado; corpus inválido retorna erros sem OOB, hang ou alocação ilimitada.
- **RF-04 / WO-04:** Mesmas entradas produzem manifesto idêntico e cada hash corresponde aos bytes da fixture.
- **RF-05 / WO-05:** Ao menos o conjunto mínimo acordado passa com bytes/estruturas comparáveis ao baseline autorizado.
- **RF-06 / WO-06:** Limites e invariantes podem ser testados antes de conhecer PCK; conversão uint64→size_t é checada.
- **RF-07 / WO-07:** Fixtures little-endian têm mesmo resultado nativo/WASM e limites de 32/64 bits são respeitados.
- **RF-08 / WO-08:** Índice retornado coincide com fixture e preserva offsets/tamanhos dentro dos limites declarados.
- **RF-09 / WO-09:** Payload conhecido é recuperado exatamente com contagem de bytes e estado final válidos.
- **RF-10 / WO-10:** Nome válido possui mapeamento determinístico e único para o manifesto.
- **RF-11 / WO-11:** Um recurso só fica pronto quando seus bytes e dependências estão íntegros.
- **RF-12 / WO-12:** Cena passa a READY apenas com todas as dependências verificadas e respeita o budget.

## Requisitos transversais

- Sem fontes/assets proprietários no build público sem decisão documentada.
- Mesmo caso reproduzível no ambiente declarado; todos os testes obrigatórios contam.
- Input inválido, lifetime e limites tratados antes de efeitos irreversíveis.
- Comandos planejados não são evidência; saída real com versão/SHA é obrigatória.
