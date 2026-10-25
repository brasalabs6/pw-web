# Release imutável — contrato v1

**Fonte de verdade.** Produtor: `tools/release/` do pw-web; consumidores previstos:
Worker app/bootstrap, publisher, testes e shell browser. Owner de conteúdo:
[contrato e15](../../e15-assets-streaming-perf-v1/contracts/content-manifest.md).
Implementação/schemas executáveis ainda pendentes; não são APIs de servidor.

## Documento e identidade

`release.json` é JSON canônico conforme e15, <=1 MiB, unknown/duplicate fields
rejeitados. Seu SHA-256 define a identidade da release e a chave
`releases/sha256/<hash>/release.json`. Nunca sobrescrever essa chave.

```ts
interface ReleaseManifest {
  schema: 1;
  mode: "diagnostic" | "game";
  sourceClient: "1.5.3"; // alvo; diagnostic não afirma compilar cliente completo
  sourceCommit: string; // git SHA-1 completo neste repo; não só branch/tag
  upstreamCommit: "e3d6d0075d0a745c9f9859f2cb25b01d71dec045";
  sourceManifestSha256: string;
  toolchainLockSha256: string;
  buildProfileSha256: string;
  assetSetSha256: string; // AssetSetIndex de e15
  assetsSchema: 1;
  protocolProfile: string | null; // ID homologado por e10, não inferido da versão
  requiredBrowserFeatures: string[]; // enum do schema: webgl2, wasm, webaudio
  runtime: {
    entryModule: string; // chave lógica existente em files
    wasmModule: string; // idem
    files: { id: string; sha256: string; bytes: number;
             mediaType: "text/javascript" | "application/wasm" }[];
  };
  entryBundles: string[]; // UI/login; mapa escolhido chega depois via índice
}
```

Hashes 64 hex; sourceCommit 40 hex; IDs/canonicalização e inteiros conforme e15.
IDs de runtime únicos; refs precisam existir; `game` exige protocolProfile não
nulo e matriz client↔assets↔realm homologada. `diagnostic` exige null e proíbe
conexão ao realm. Memória/capacidades opcionais não são requisitos mínimos ocultos:
OPFS indisponível tem fallback, pthreads/SIMD não entram antes de contrato e prova.
Não incluir URLs de produção, segredo, timestamp atual ou caminho absoluto no
artefato: o mesmo build precisa poder ser promovido entre ambientes.

Bootstrap servido pelo app: `{schema:1, releaseSha256, assetBaseUrl,
mode, realmId:null|string, gatewayUrl:null|string}`. App é seu produtor; paths
relativos derivam apenas de hashes validados. `assetBaseUrl` é HTTPS allowlisted,
`gatewayUrl` WSS aprovado no ambiente. Diagnostic retorna realm/gateway nulos.
Erros: UnsupportedSchema, InvalidRelease, IncompatibleRuntime,
IncompatibleRealm, MissingObject, InvalidSignature; não escolher versão mais
nova ou degradar para modo game por conta do cliente.

## Assinatura e confiança

Produção requer assinatura detached Ed25519: `{schema:1,algorithm:"Ed25519",
keyId,manifestSha256,signature}`; signature base64 de 64 bytes. Input assinado é
UTF-8 literal `pw-release-v1\n` seguido dos 64 hex ASCII do hash do manifest.
Documento em `signatures/<hash>/<keyId>.json`. Shell tem chaves públicas confiáveis
versionadas, nunca chave privada; conferir assinatura e hash antes de inicializar
cliente. Sintéticos locais podem usar chave de teste explicitamente identificada,
sem aceitá-la na política de produção. Rotação: introduzir chave nova no shell
antes de assinar somente com ela; preservar verificação da release antiga no
intervalo de suporte. Algoritmo alternativo exige revisão do contrato.

TLS protege origem; hashes protegem conteúdo/pins; assinatura protege a promoção
contra publicação não autorizada quando chave/release estão separados. Um PR não
recebe a chave de assinatura. CI com signing comprometido continua um risco,
mitigado por environments protegidos, aprovação e attestation do build.

## Publicação, cache e rollback

Publisher valida schema, assinatura e grafo fechado; sobe objetos por hash e faz
readback dos novos objetos/tamanhos/hashes. HEAD/ETag sozinho não prova SHA-256.
Sobe bundles, catálogos, índice, release e assinatura; verifica a closure de entry/runtime e
existência da closure completa do índice antes de habilitar o app. Reexecução com
mesmo hash é idempotente; conflito de bytes aborta. Estados do job:
Built → Uploaded → Verified → Previewed → Approved → Active; Failed não promove.

Worker app contém pin `RELEASE_SHA256` e base CDN do ambiente, servindo
`/bootstrap.json` com `Cache-Control:no-store`; HTML revalida. Runtime e manifests
imutáveis podem ter TTL longo; bootstrap nunca é objeto immutable. Deploy app
preserva a versão anterior. Sessão pinou release: não troca em background nem
mistura assets após novo deploy. Reconnect só é liberado se o realm ainda suporta
esse protocolProfile; senão orientar reload na fronteira segura da sessão.

Rollback: redeploy/rollback da versão app com o pin anterior e verificação do
manifest disponível. Reverter Worker não reverte R2, cache, banco ou schema do
realm. Migração de servidor incompatível exige plano e aprovação em e10/e17.
Conservar releases Active/rollback e todas as referências de seus objetos.

GC é mark-and-sweep: raízes são pins ativos, rollback, previews vivos e leases de
sessões, mais grace period. Idade de objeto sozinha não autoriza apagar chunk
compartilhado. Até haver lease de sessão e janela máxima definidas, **GC remoto
automático fica desligado**; PR fechado remove preview, não objetos por hash.

## Aceite adversarial

Release com arquivo faltante, assinatura inválida ou protocolo não homologado não
promove. Queda depois do upload e antes do deploy mantém release anterior ativa.
Dois publishers não atualizam o mesmo ambiente em paralelo: GitHub concurrency +
checagem de pin/versão esperada imediatamente antes da promoção; timeout consulta
deployments antes de repetir. Duas abas com releases diferentes continuam lendo
seus hashes. CDN com 404 antigo não transforma objeto ausente em sucesso; subir
antes de anunciar, revalidar leitura e limitar negative caching nos prefixes.

## Carregamento de código e integridade

V1 publica um entry JS único do runtime, sem imports JS externos não cobertos.
O shell confiável verifica release/assinatura e injeta `<script type="module">`
com src por hash, `integrity="sha256-<digest base64>"` e crossorigin anonymous.
Bundle inclui o glue necessário; qualquer import transitivo externo bloqueia
a build até existir cadeia de integridade equivalente. CDN deve permitir CORS.
O contrato de entrada e04 deve receber o pin da release explicitamente, validar
a correspondência e impedir inicialização duplicada; fechar essa API em CF-06
antes de usar código original. Isso não é uma ABI JS→C++ já implementada.

O runtime usa bytes WASM verificados por SHA/tamanho antes de instanciar, via
entrada wasmBinary equivalente validada na versão Emscripten. Sem duplicar
download. compileStreaming/hash incremental só substitui esse caminho após prova
de memória/integridade: não instanciar código recebido antes de validar. SRI é
para o entry JS; não fingir que ele também valida o WASM baixado pelo glue.
