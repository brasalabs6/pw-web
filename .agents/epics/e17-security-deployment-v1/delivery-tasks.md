# Entrega Cloudflare — backlog incremental v1

**Estado: execução incremental.** CF-03 tem uma primeira implementação no PR #16
em revisão local; permanece incompleta até produtores/consumidores reais importarem
o validador comum. As demais tasks continuam pendentes.
Escopo manual complementar a e03/e04/e05/e15/e17, sem alterar scheduler, write_scopes
ou completed-workorders. Follow @implementation-loop e @execute-plan na execução;
contratos novos/alterados seguem @architecture-spec-authoring. Um incremento por PR.
Os comandos futuros indicados aqui são contratos de ferramentas **a criar**, não
scripts existentes. Testes comportamento/contrato começam RED antes da implementação.

## Grafo e cortes de entrega

- Corte A: CF-02/03/05/06/08/09/15 → build diagnóstico sintético reproduzível.
- Corte B: CF-01 + CF-16/17/18/19 → upload sandbox + Preview + prova de CDN.
- Corte C: CF-10/11/12/13/14 → streaming/cache/transição com falhas e benchmark.
- Corte D: CF-04/07 + e05/e06/e07 homologados → conteúdo/cliente reais, sem salto de gate.
- Corte E: CF-20/21/22/23/24 → staging, promoção, rollback e operação.
- Exposição pública MVP é decisão do operador já recebida. Conta/domínios/tokens
  ainda não foram confirmados; isso bloqueia apenas execução remota, não os contratos.

## Tarefas por camada e dono

### Inputs, source e build

- [ ] **CF-01 (HUMAN, external-account/credential, bloqueia remoto)** — Operador/Codex,
  inventário de conta/zone/domínios, plano de acesso e GitHub Environments. Definir
  nomes reais de buckets inputs privados/content preview/staging/prod, domínios
  CDN e app, quem aprova produção e escopos dos tokens. Não gravar valores secretos.
  Aceite: tabela sem placeholders + token preview negado em produção. Validação:
  sandbox remoto e negativo por recurso, não mera leitura da configuração.
- [ ] **CF-02** — Build owner, `toolchains/lock.json`, package.json/lockfile de tooling;
  sem dependências. Pin emsdk/Node/CMake/Ninja/cooker/Wrangler conforme build-pipeline,
  verificar digest real de imagem e CLI `preview` instalada. Aceite: checkout limpo
  sem binários locais e versões divergentes rejeitadas. Validação: `npm ci`,
  `npx --no-install wrangler --version`, verificador de lock a criar.
- [ ] **CF-03** — Produtores `tools/assets/schemas`, `tools/release/schemas`, testes
  consumidores; sem dependências. Materializar schemas canônicos dos dois contratos,
  canonicalização/hashes e fixtures independentes. Aceite: reject unknown/duplicate
  keys, path traversal, span overflow, ciclo e release game sem protocolProfile.
  Validação: suíte de contrato a criar; cada produtor/consumidor usa o mesmo schema.
- [ ] **CF-04** — Source owner, `src/client153/source-manifest.json`, ferramenta de
  import/proveniência; depende CF-03 e closure autorizada A1/e06. Importar arquivos
  mínimos do SHA fixado com headers e hashes, preservar caminhos; rejeitar libs/
  objetos/SDKs e símbolos duplicados. Aceite: adulteração upstream e arquivo extra
  não rastreado falham; references intacto. Validação: provenance checker a criar.
- [ ] **CF-05** — C++ owner, CMake root e `src/client153/CMakeLists.txt`; depende CF-02.
  Criar targets explícitos dos módulos hoje portáteis; fonte legacy só após CF-04.
  Aceite: testes nativos existentes passam; fonte Windows fora do alvo falha em
  vez de obter no-op. Validação: configure/build/ctest em diretório vazio a criar,
  mais `./scripts/verify.sh`; baseline Windows continua gate separado e02.
- [ ] **CF-06** — WASM owner, perfis `wasm-debug/release` e runner de fixture; depende
  CF-02/05. Compilar core/R0 sintético sem assets, manifest modo diagnostic, JS glue
  modularizado e WASM, símbolos só em private-debug. Fixar API de entrada do runtime
  com e04 e entry JS único protegido por SRI (sem imports externos descobertos) antes de adicionar o cliente real. Aceite: sem exports desconhecidos,
  sem .data monolítico, release informa honestamente capacidades. Validação:
  `./scripts/test-render-clip.sh`, build R0 + browser existentes e teste de outputs.
- [ ] **CF-07** — Asset/source owner, catálogo de inputs privados e mapeamento e05;
  depende CF-04 e fontes/fixtures reais. Fechar versões PCK/PKX/mesh/texture antes
  do extractor. Aceite: entrada truncada/path colliding bloqueia; versão errada não
  é aceita como 1.5.3. Validação: testes golden autorizados e referência nativa.
- [ ] **CF-08** — Release owner, diretório candidato/allowlist e relatório de tamanho;
  depende CF-03/06. Separar shell/runtime/content/provenance/private-debug; scanner
  nega segredo, source map e assets fora do catálogo. Aceite: `.env`/`.obj`/source
  injetados impedem publish, sem só confiar em gitignore. Validação: fixture de
  vazamento no packager e checksum do output aprovado.
- [ ] **CF-09** — Build owner, job de reprodutibilidade; depende CF-08. Duas builds
  limpas com filesystem order/timezone distintos; cache desabilitado na comparação.
  Aceite: hashes runtime/chunks iguais; report separa metadados de attestation.
  Validação: comparador de artefatos a criar, falhando com mudança de byte real.

### Conteúdo e browser

- [ ] **CF-10** — `tools/assets` cooker sintético; depende CF-03. Implementar regras
  de blocos/microbundles/dedupe, DAG e proprietário comum de asset. A versão real
  depende CF-07. Aceite: >4 MiB dividido, colisão rejeitada, alteração em pequeno
  invalida só microbundle pertinente; rebuild determinístico. Teste do cooker.
- [ ] **CF-11** — `web/streaming` loader; depende CF-03/10. Resolver closure lazy,
  verificar SHA/bytes, dedupe por hash, fila P0–P3, aborto refcount e retries finitos.
  Aceite: 200 HTML de login/hash ruim não chegam ao decoder; 401 não vira retry
  infinito; nenhuma busca `latest`. HTTP fixture real + testes de erro a criar.
- [ ] **CF-12** — `web/storage` cache; depende CF-11. Commit em duas fases, OPFS/IDB
  fallback, leases entre abas e LRU protegido. Aceite: quota negada funciona em
  memória; crash/temp órfão e duas abas não servem bytes parciais nem removem pin.
  Validação: testes browser com storage negado, corrupção e concorrência.
- [ ] **CF-13** — `engine/resources` + streaming de cena; depende CF-11/12 e contratos
  de cena e07. Generation/cancel/transição entry, ownership RAM/GPU e retirada do
  mundo antigo no limite permitido. Aceite sintético: A→B→C não ativa B atrasado,
  baixa memória usa loading explícito. Aceite real depende e10/e11 e realm; não
  inventar ready packet. Validação: trace de estados + handle counts + cenários.
- [ ] **CF-14** — Performance owner, `tools/benchmarks` e budgets versionados; depende
  CF-13. Medir cold/warm/rede restrita/context loss e separar HTTP, WASM, decode,
  heap e GPU. Aceite: relatório com p50/p95/bytes/requests e nenhum número inventado;
  thresholds propostos ficam aprovados antes de virar gate bloqueante.

### Publicação e CDN

- [ ] **CF-15** — `tools/release` builder/verifier; depende CF-03/08/10. Release
  pinada, canonicalização, assinatura detached com chave de teste e verificação
  pública, closure completa validada. Aceite: assinatura/chave/input faltante e
  protocolProfile incompatível impedem promoção. Validação: suite de contrato real,
  sem chave de produção no teste.
- [ ] **CF-16** — Release owner, uploader R2 S3; depende CF-15; remoto depende CF-01.
  Upload idempotente por hash, conteúdo→bundles→índice→release/assinatura, readback
  e estado de job. Aceite: timeout reconciliado e objeto conflitante bloqueia; falha
  parcial preserva app anterior. Validação local S3 compatível + sandbox R2 real;
  mock não prova headers/cache Cloudflare.
- [ ] **CF-17** — Infra owner, configuração revisável CDN/CORS/metadata e buckets;
  depende CF-01/16. Domínios próprios, r2.dev desligado, públicos apenas objetos
  permitidos, identidade de chunks e MIME/runtime correto. Aceite: HIT/MISS medido,
  CORS sem credentials e origem de inputs privada negada. GET real, hash de corpo,
  negative cache e tentativa de acesso ao input/debug.
- [ ] **CF-18** — `infra/cloudflare/app` Worker/bootstrap/Wrangler; depende CF-02/15.
  Implementar pin e campos por ambiente; registrar URLs de teste, nunca realm prod
  por default. Aceite: pin/domínio ausente => 503, diagnóstico sem WSS e raiz de
  deploy contém só public-shell. Unit/contract + wrangler dev/dry-run; remoto CF-01.
- [ ] **CF-19** — Publisher preview e browser smoke; depende CF-16/17/18. `wrangler
  preview` com nome PR validado e config explícita, usando artifact digest aprovado;
  verificar URL/SHA antes de comentar no PR. Aceite: preview não usa produção e
  round-trip de fixture cold/warm passa; erro/404/hash ruim dá FAIL. Criar runner
  com Chrome desktop/mobile e coleta sanitizada; sem browser é NOT RUN.

### CI/CD e operação

- [ ] **CF-20** — CI owner, `game-build.yml`; depende CF-09/15. PR read-only, build
  sintético, schemas, WASM e browser, outputs/digests; preservar workflows existentes.
  Aceite: fork sem secrets passa CI e não publica nada. Rodar workflow de fork/teste
  controlado e revisar permissions/pins SHA de actions realmente existentes.
- [ ] **CF-21** — CI owner, `game-preview.yml` + cleanup; depende CF-19/20. Separar
  build não confiável de publisher confiável, aprovar head/run_id/digest e negar
  scripts do artefato. Cleanup valida PR e remove só Preview. Aceite: artifact
  adulterado, stale SHA e PR externo não obtêm tokens; shared chunks permanecem.
- [ ] **CF-22** — CI/operador, staging + `game-release.yml`; depende CF-21 e CF-01.
  Promover os mesmos hashes, Environment production aprovado, assinatura com chave
  autorizada, concurrency e precondition da versão remota. Aceite: deployment
  concorrente ou smoke falho não substitui pin; sem auto-merge ou rebuild oculto.
- [ ] **CF-23** — Infra/browser owner, rollback e mixed-version; depende CF-22.
  Reverter pin app, manter objetos/release anterior e duas abas antigas/novas.
  Aceite: login/cena usam cada closure correta; mudança incompatível de realm
  bloqueia e requer forward recovery de e10. Simulação de queda em cada fronteira.
- [ ] **CF-24** — Operação, retenção/custo/GC; depende CF-14/23. Raízes ativas,
  rollback/previews/leases e janela de suporte; sem lease GC automático permanece
  desativado. Aceite: dry-run mark/sweep jamais inclui objeto compartilhado vivo;
  plano de custo usa workload medido, não promessa de CDN gratuito ilimitado.

## Gate de implementação e limites

Prontos para código offline: schemas/testes, manifests/fixtures, CMake de módulos
já portados, packager diagnóstico, scheduler/cache sintéticos e validadores.
Remoto: CF-01 antes de tokens/upload/domínios. Assets reais: CF-07 antes de cooker
real. Game mode: baseline e02 + renderer/formatos/cena + protocolo homologado;
nenhuma task desta árvore elimina esses gates. WO-02 de e15 continua exigindo seus
próprios critérios de Range quando executado; v1 por objetos inteiros é um corte
anterior, não marca essa workorder concluída por aproximação.

## Evidência de execução parcial — CF-03 (2026-10-25)

- Criados JSON Schemas de conteúdo e release, canonicalização/rejeição de chaves
  duplicadas, validação estrita e checagem da closure de índice→catálogo→bundle→release.
- Criados fixtures canônicos independentes com hashes fixos e 11 testes Node.
- `./scripts/verify.sh`: 31 testes Python, 11 contratos Node e 14 casos C++ PASS.
- `./scripts/probe-manifest-browser.sh` com `CHROME_BIN` e
  `PW_BROWSER_MODULE_DIR` apontando para instalações locais:
  PASS em Chrome 152: browser importou os mesmos JSON Schemas, percorreu
  index→catalog→bundle por hashes e validou o release e o path traversal negativo.
  Manifest local fica em `artifacts/`.
- Escopo ainda aberto: não existem cooker/release publisher ou `web/streaming`
  consumers integrados; não valida bytes de objetos ou assets/source/game reais.
  Esses consumidores precisam importar esta mesma implementação/schema, não
  replicar as regras. Nenhum upload, conta Cloudflare ou deploy foi executado.
