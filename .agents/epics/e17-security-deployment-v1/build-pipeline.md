# Build e árvore portada — spec v1

Estado: **planejado**, complementa e03/e04/e05/e17; esta entrega não contém CMake,
cooker, workflows de deploy ou build completo do cliente. Regras de fonte:
[ADR-0005](../../../docs/DECISIONS/ADR-0005-cloudflare-delivery.md) e
[src/client153](../../../src/client153/README.md).

## Módulos e pacotes de desenvolvimento

| Área proposta | Responsabilidade | Forma de build |
| --- | --- | --- |
| src/client153/Angelica2 | arquivos originais portados, por closure | bibliotecas C++ estáticas explícitas |
| src/client153/Element | lógica do cliente autorizada | biblioteca/entry C++, quando dependências fechadas |
| engine/core, render, formats, resources | contratos e sistemas portáteis novos | testes nativos + WASM |
| engine/platform/web | GLES3, loop/input, arquivo em memória, áudio/rede adapters | ligação Emscripten |
| web/ | shell, bootstrap, glue, streaming/storage | ESM; bundler só quando necessário |
| tools/assets | inventário/cooker/chunks/manifests determinísticos | ferramenta offline, fora do browser/Worker |
| tools/release | schema, assinatura, grafo, upload e promoção | ferramenta CI, sem lógica de jogo |
| infra/cloudflare/app | bootstrap e hosting Static Assets | Worker/Wrangler |
| gateway/ | transporte WSS e fronteira do realm | alvo próprio de e10; hospedagem não homologada |

Módulos C++ não implicam vários WASMs. Primeiro um runtime principal modularizado
em targets estáticos, com exports mínimos. Código de mapas/instâncias é normalmente
o mesmo; o que muda são dados. Side modules Emscripten só após benchmark de ABI,
memória compartilhada entre módulos e tamanho; não fabricar DLL load sob demanda.

## Importação da source 1.5.3

`src/client153/source-manifest.json` será fonte rastreada de proveniência, schema
com `schema:1`, `upstreamUrl`, `upstreamCommit` fixo, `authorizationRef`, `files[]`
com `upstreamPath`, `upstreamSha256`, `portedPath`, `copyrightPreserved:true`.
Paths relativos normalizados; cada arquivo upstream aparece uma vez. Arquivo novo
nosso não recebe origem inventada; pertence ao módulo novo apropriado.
Hash da versão portada vem de um relatório de build gerado, não do próprio arquivo
que o contém. Novos imports conferem bytes upstream e header antes do commit.

CMake usa lista explícita de fontes/headers e targets, sem `GLOB_RECURSE` pegando
todo Win32. Build comum não inicializa references. O job de import/proveniência
é opt-in autorizado e usa SHA fixo; nunca `submodule update --remote`. Após import,
CI compila os arquivos rastreados de src sem depender de checkout externo.
Não ligar duas cópias do mesmo símbolo de engine/src. Não importar `.obj`, `.pch`,
`.lib`, `.dll`, SDKs, assets ou configurações de rede. Dependência ausente é erro,
não typedef/stub/no-op. Headers de plataforma permanecem fora do núcleo portátil.

## Toolchains e perfis

Criar `toolchains/lock.json` com emsdk **6.0.13** (já observado localmente), commit
upstream do SDK, digest de imagem Linux a verificar, versões exatas CMake/Ninja,
Node e ferramentas cooker, Wrangler **4.152.0** (versão consultada no registry em
2026-10-25). Esses pins são proposta; arquivo/lockfile ainda não existem. Instalar
Wrangler local como devDependency com versão exata e lockfile gerado por npm.
Ações GitHub são fixadas por SHA de commit verificado, não SHA fictício nem tag
`latest`. CI registra versões reais e falha por divergência do lock.

Perfis propostos: native-tests com sanitizers; wasm-debug com assertions/mapas
privados; wasm-release com otimização de tamanho medida (`-Oz` versus `-O2`), LTO,
dead stripping e exports explícitos. Compilar e ligar com flags coerentes.
Proibir `-ffast-math`, remoção indiscriminada de exceções/RTTI ou suppressions
só para reduzir bytes. Exceções, RTTI, filesystem, threads e ALLOW_MEMORY_GROWTH
precisam de inventário do código real; não copiar flags do scaffold para o cliente.
Um perfil baseline WebGL2/sem pthreads evita exigir COOP/COEP antes de precisar.
SIMD/pthreads são variantes futuras com feature detection e fallback comprovado.

## Outputs e reprodutibilidade

```text
build/<profile>/             # intermediários, nunca CDN
artifacts/release-candidate/
  public-shell/              # HTML/CSS/JS do launcher, somente allowlist
  runtime/                   # .mjs + .wasm; sem assets .data monolíticos
  content/                   # índice, manifests e objetos por hash
  release.json               # contrato e17
  provenance/                # source/build reports, SBOM, tamanhos
  private-debug/             # símbolos/source maps; excluídos do publisher
```

Nunca apontar `assets.directory`/uploader à raiz do repo. Publisher tem allowlist
explícita, nega `.env`, `.dev.vars`, source/maps, referências, credenciais e debug.
Dois checkouts limpos com o mesmo input devem gerar mesmos hashes de runtime e
conteúdo; normalizar caminhos (`-ffile-prefix-map`), timezone/locale/ordem, arquivos
gerados e timestamps. Attestation/assinatura são metadados detached; não alteram
bytes promovidos. Cache CI é otimização, não autoridade: key inclui toolchain,
perfil, source manifest, lockfiles e dependências; release validada também sem cache.

Build de conteúdo é independente do código. PR que só muda C++ não recooka todo
o jogo; usa AssetSetIndex já homologado. PR de assets referencia input catalog
por hash; cache do cooker usa hash do input + versão/config cooker. Dados de jogo
ficam em armazenamento de inputs com acesso restrito, não em checkout público.

## Sequência CI/CD a implementar

1. `pull_request`: sem secrets, submodules=false, testes existentes + schema,
   compile nativo e WASM para módulos já portados, fixtures sintéticas, browser
   R0/streaming. Verificação de source/licenças, SBOM, size report e artefatos.
   Não chamar prova de cliente completo um job que só compila core/probe.
2. PR trusted aprovado: workflow de preview separado com Environment protegido.
   Usa run_id/head SHA e artifact digest exatos do job aprovado. O publisher não
   executa código/scripts do PR com token de produção. Fonte/workflow/config/lockfile de publicação
   vêm de commit confiável; artefatos são dados validados e não escolhem comandos.
   Conferir identidade/revisão do workflow de build permitido, não confiar em
   report.json do PR afirmando que passou. Mudança de workflow requer revisão. WASM não roda no publisher.
3. Build autorizado de conteúdo: job isolado/opt-in; não recebe assets em forks.
   A prova sintética pública continua separada do build com conteúdo original.
4. Upload R2 sandbox → verificar hashes/grafo → assinar por ambiente autorizado →
   deploy Preview com Wrangler → smoke positivo/negativo → comentário com URL,
   SHA/build/assetSet e resultado. Sem credenciais, validar pacote local e marcar
   etapa remota BLOCKED. Não criar conta temporária para contornar o bloqueio.
5. Merge main gera candidato de staging dos mesmos outputs verificados, sem
   rebuild diferente na promoção. Produção usa workflow_dispatch + GitHub
   Environment production com aprovação humana, pin do artefato e concurrency.
6. Smoke público/protegido conforme política, headers/cache/CORS, hash de runtime,
   feature detection, world entry somente quando existir realm de staging real.
   Falha de promoção mantém pin anterior; rollback usa artefato anterior preservado.
7. PR closed remove Preview pelo identificador validado e token de preview. Não
   apagar bucket compartilhado ou fazer GC remoto automático por idade.

Nomes propostos: `game-build.yml`, `game-preview.yml`, `game-release.yml` e
`game-preview-cleanup.yml`. **Ainda não existem**. Preservar CI/Grok existentes;
primeiro ampliar testes, só depois habilitar upload. Nunca `pull_request_target`
com checkout/código não confiável, nem disparar produção por comentário de PR.

## Comandos previstos e gate

Hoje executáveis: `./scripts/verify.sh`, `./scripts/test-render-clip.sh`,
`./scripts/build-render-probe.sh`, probes opt-in existentes. Futuros comandos
contratuais (não executáveis ainda): `build-client`, `cook-assets`,
`validate-release`, `publish-release`, `promote-release`, `benchmark-streaming`,
com CLI/paths fechados na tarefa dona antes da implementação. Não publicar exemplos
com aparência de pipeline já implementada. O primeiro corte deve usar só uma
fixture sintética e comprovar o fluxo inteiro antes de conteúdo comercial.
