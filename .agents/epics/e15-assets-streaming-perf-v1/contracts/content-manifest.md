# Manifest de conteúdo — contrato v1

**Fonte de verdade.** Produtor: `tools/assets/` no pw-web. Consumidores previstos:
`web/streaming`, `web/storage`, `engine/resources`, publisher de e17. Contrato
novo interno, especificado; schemas/implementação ainda pendentes. Não é wire
format PCK/PKX nem protocolo do servidor. Integra e15 WO-01/02/03/04 e e05.

## Grafo e identidade

Documentos JSON UTF-8: AssetSetIndex → BundleCatalog → BundleManifest → objetos.
JSON canônico do produtor: chaves ordenadas lexicograficamente, sem whitespace,
sem floats, sem duplicate keys, bytes UTF-8 sem BOM. Após parse limitado a 32
níveis, reserializar e exigir bytes idênticos ao canônico recebido: isso também
rejeita chaves duplicadas que JSON.parse normalmente descartaria. Hash é SHA-256 dos bytes
canônicos; fixtures de canonicalização fazem parte do contrato. O documento não
contém seu próprio hash. Limites propostos v1: cada JSON <=2 MiB; índice raiz <=256 shards, <=16384 bundles por
shard e <=16384 assets por bundle. Catálogo/asset group que exceder limite falha
com SizeLimit até reparticionamento determinístico do cooker; nunca truncar. Todos os inteiros são não negativos <= Number.MAX_SAFE_INTEGER.

AssetSetIndex:

```ts
interface AssetSetIndex {
  schema: 1;
  sourceClient: "1.5.3";
  cookerVersion: string; // versão exata e não vazia
  catalogs: { prefix: string; sha256: string }[]; // prefix: dois hex minúsculos
}
interface BundleCatalog {
  schema: 1;
  prefix: string;
  bundles: { id: string; manifestSha256: string }[];
}
interface BundleManifest {
  schema: 1;
  id: string;
  requires: string[]; // IDs deste AssetSetIndex; DAG, sem dependências ausentes
  assets: {
    path: string;
    bytes: number;
    sha256: string; // arquivo lógico completo, após extração, antes de decode GPU
    requiredForEntry: boolean;
    spans: { objectSha256: string; objectBytes: number;
             offset: number; bytes: number }[];
  }[];
}
```

Shard de um bundle = primeiros dois hex de SHA-256(UTF-8(id)). O índice lista
apenas shards não vazios; o browser busca lazy o shard necessário. Catálogo contém
só IDs do próprio prefix, ordenados e únicos. Produtor valida todo o DAG offline,
mas o browser não precisa baixar todos os catálogos para abrir login.

IDs: ASCII `[a-z0-9][a-z0-9._/-]{0,127}`, sem segmentos `.`/`..`, barras duplas,
backslash ou slash inicial. Hash: exatamente 64 caracteres hex minúsculos.
Paths: UTF-8 NFC, slash relativo, sem NUL/controle, `.`/`..`, drive, URL ou barras
vazias; <=1024 bytes. O cooker mantém tabela explícita de nome original→path.
Colisão por normalização, casefold ou codificação legado é erro com diagnóstico;
não escolher um arquivo silenciosamente. Nenhum decoder infere versão pelo nome.

`objectBytes` está em 1..4194304; spans têm bytes positivos e contagem <=16384.
`requires` e IDs não admitem duplicatas. `spans` ordenados reconstroem o arquivo concatenando suas fatias. Requer
`offset+bytes<=objectBytes`, soma=asset.bytes e hash final correto. Um asset vazio
admite spans=[] e SHA-256 do conteúdo vazio. Hash de cada objeto é validado antes
de consumir qualquer span. A lista de dependências deve ser acíclica e seus
arquivos obrigatórios precisam estar íntegros antes do bundle ficar Ready.

## Empacotamento determinístico

V1 mantém os formatos autorizados extraídos; transformação de texturas/malhas é
etapa separada, versionada e testada contra a referência. Nunca servir PCK inteiro
nem usar `--preload-file` de todo o jogo no WASM. Sem perda visual silenciosa.

Limite proposto: objetos binários identity de **4 MiB**, não meta de download
inicial. Cada asset >=256 KiB é segmentado sozinho em blocos de até 4 MiB; assets
menores são ordenados por path e agrupados deterministicamente por bundle em
microbundles de até 1 MiB (sem padding; flush antes de exceder). Um asset não é
partido entre microbundles; arquivo maior usa a regra de blocos grandes. Arquivos
referenciados por vários bundles têm um único bundle proprietário comum e uma
aresta `requires`; IDs duplicados de path entre proprietários são erro.

Dedupe por hash do objeto, sem estado local da execução anterior. Isso reduz
redownload de blocos grandes; alteração em um arquivo pequeno pode invalidar seu
microbundle, custo deliberadamente limitado a 1 MiB. Remover timestamps, caminhos
absolutos e ordem de filesystem da saída. Hash igual deve produzir bytes iguais;
objeto existente com bytes/hash diferentes bloqueia publicação.

## Transporte e erros

Chave `objects/sha256/<hash>`. Binários de chunks: octet-stream, Content-Encoding
ausente, Cache-Control público `public,max-age=31536000,immutable,no-transform`
quando exposição pública for liberada. Desabilitar transformação/compressão HTTP
para esse prefixo. V1 busca objetos inteiros pequenos: não precisa de Range.
Range futuro serve apenas objetos identity com teste 206/Content-Range/If-Range;
um ETag de R2 não substitui SHA-256 e o Worker não herda If-Range automaticamente.

Erros estáveis: InvalidSchema, UnsupportedSchema, InvalidPath, DuplicatePath,
MissingDependency, DependencyCycle, SizeLimit, HashMismatch, TruncatedObject,
ObjectMissing. Schema desconhecido é erro; campos desconhecidos são rejeitados
em v1. Nenhum erro de conteúdo permite fallback de versão ou asset em branco.

## Exemplo mínimo válido e negativos

Bundle `bootstrap/ui`, sem dependências, único arquivo vazio `ui/empty.bin`:
`bytes:0`, `spans:[]`, `sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.
O catálogo do shard correspondente lista o hash real desse bundle; o índice
lista o hash do catálogo. Todos são gerados a partir dos bytes canônicos.
Não usar o hash do arquivo vazio como hash do bundle JSON.

Negativos obrigatórios: hash com 63 caracteres; path `../x`; colisão `A`/`a`;
span overflow; total diferente; ciclo; objeto faltante; JSON duplicando chave;
resposta 200 de login HTML; bytes truncados; mesmo conjunto gerado em duas
máquinas com hashes diferentes. A fixture sintética deve incluir arquivo >4 MiB,
vários pequenos, asset compartilhado e dependência que falha, com esperados
independentes do cooker. Esquemas executáveis pertencem a `tools/assets/schemas/`
e são consumidos, não copiados, pelo publisher/browser.
