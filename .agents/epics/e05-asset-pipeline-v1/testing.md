# Testes — Pipeline de recursos PCK/PKX, formatos e proveniência

Positivo e negativo por workorder são obrigatórios. Ferramenta ausente significa
BLOCKED; teste executado com falha significa FAIL. Build sozinho não é prova runtime.

## WO-01

- Positivo: Cada campo catalogado tem origem verificável; formatos ainda não compreendidos permanecem bloqueados.
- Negativo: Nome de extensão ou versão aproximada não determina layout binário.
- Comando/ação: `Revisão de fonte:linha/hash no inventário; ./scripts/verify.sh`.

## WO-02

- Positivo: Cada parser futuro tem formato acordado, erro e exemplo válido/inválido; algoritmo de compressão não é adivinhado.
- Negativo: Schema incompleto impede aprovação; não se cria contrato no consumidor para preencher ausência de evidência.
- Comando/ação: `Revisão semântica campo a campo + ./scripts/verify.sh`.

## WO-03

- Positivo: Arquivo válido produz o índice e conteúdo esperado; corpus inválido retorna erros sem OOB, hang ou alocação ilimitada.
- Negativo: Bombas de descompressão e offsets além do arquivo são rejeitados antes de exceder os limites.
- Comando/ação: `./scripts/test-asset-parsers.sh (criar; incluir sanitizer nativo e WASM)`.

## WO-04

- Positivo: Mesmas entradas produzem manifesto idêntico e cada hash corresponde aos bytes da fixture.
- Negativo: Dependência ausente/cíclica ou mesmo ID com conteúdos diferentes falha sem manifesto parcialmente válido.
- Comando/ação: `python3 -m unittest discover -s tests -p 'test_asset_manifest.py' -v (criar)`.

## WO-05

- Positivo: Ao menos o conjunto mínimo acordado passa com bytes/estruturas comparáveis ao baseline autorizado.
- Negativo: Asset de outra versão ou sem autorização não integra o corpus válido.
- Comando/ação: `Runner de WO-03 com fixtures autorizadas locais e hashes; ./scripts/verify.sh`.

## WO-06

- Positivo: Limites e invariantes podem ser testados antes de conhecer PCK; conversão uint64→size_t é checada.
- Negativo: offset+length que transborda ou tamanho descomprimido além do budget não permite alocação.
- Comando/ação: `Revisão de casos uint64_max/wasm32 + ./scripts/verify.sh`.

## WO-07

- Positivo: Fixtures little-endian têm mesmo resultado nativo/WASM e limites de 32/64 bits são respeitados.
- Negativo: Arquivo de 3 bytes não permite read_u32; erro não avança cursor.
- Comando/ação: `./scripts/test-byte-reader.sh (criar nesta WO)`.

## WO-08

- Positivo: Índice retornado coincide com fixture e preserva offsets/tamanhos dentro dos limites declarados.
- Negativo: Magic desconhecido, registro truncado e intervalo cruzando arquivo sem regra definida são rejeitados.
- Comando/ação: `./scripts/test-archive-index.sh (criar nesta WO)`.

## WO-09

- Positivo: Payload conhecido é recuperado exatamente com contagem de bytes e estado final válidos.
- Negativo: Stream curto e saída anunciada acima do budget retornam erro controlado.
- Comando/ação: `./scripts/test-decompress.sh (criar nesta WO)`.

## WO-10

- Positivo: Nome válido possui mapeamento determinístico e único para o manifesto.
- Negativo: Duas entradas que convergem para o mesmo nome e encoding inválido falham explicitamente.
- Comando/ação: `python3 -m unittest discover -s tests -p 'test_asset_names.py' -v (criar)`.

## WO-11

- Positivo: Um recurso só fica pronto quando seus bytes e dependências estão íntegros.
- Negativo: Hash diferente ou dependência ausente impede READY mesmo se HTTP retornou 200.
- Comando/ação: `node --test tests/assets/manifest.test.mjs (criar)`.

## WO-12

- Positivo: Cena passa a READY apenas com todas as dependências verificadas e respeita o budget.
- Negativo: Cancelamento não deixa recursos half-ready nem conserva referências a buffers descartados.
- Comando/ação: `node --test tests/assets/loader.test.mjs (criar) + cenário browser de cancelamento`.

## Evidência

Registrar comando, versão, SHA, fixture, resultado e limites em docs/evidence;
logs/artifacts locais não substituem um resultado verificável. Comandos “criar” são
entregáveis futuros; registrar zero casos ou screenshot isolada não satisfaz aceite.
`./scripts/verify.sh` continua obrigatório e não valida o cliente completo.
