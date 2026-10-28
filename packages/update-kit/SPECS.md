# SPECS.md — Atualizador seguro de cliente WASM e assets

> **Componente:** `packages/update-kit` · **Package:** `@pw/update-kit` · **Status:** implementação inicial em `feat/multi-realm-platform-shell-mvp`.
> **Autoridade:** o código versionado neste package, `docs/PAINEL.md` e `docs/PLATFORM-IMPLEMENTATION.md`. `[ATUAL]` = implementado no repo, `[PROPOSTO]` = objetivo ainda sujeito a código/teste.
> **Objetivo central da plataforma:** shell mostra servidores; o cliente e sua atualização são determinados automaticamente pelo pairing do realm. O package mantém sua própria fronteira sem inventar suporte 1.3.6/1.4.4.

## 1. Propósito e princípios de design

- Instalar/atualizar automaticamente os arquivos da **release escolhida pelo servidor**, nunca por usuário; conferir manifest assinado e todos os bytes antes de uso.
- Proteger contra manipulação de CDN/asset, mistura de versões e update parcial; aproveitar hashes iguais com cache verificado.
- Evoluir para streaming de entry bundles e mapas do cliente 1.5.3 PR #23, mantendo integridade e sem bloquear login por bytes opcionais.

**Stack:** TypeScript browser, WebCrypto Ed25519/SHA-256, CacheStorage, Zod v2; @pw/update-kit.

**Situação comprovada [ATUAL]:** verifyManifest, canonicalJson e prepareClient executam verificação criptográfica e cache de downloads. CI test:updater cobre assinatura válida e adulteração. Não inicia Angelica (canLaunchOriginalClient retorna false), não possui asset DAG, leases, quotas ou progress byte streaming.

### Princípios inegociáveis

- Cada dado/ação tem um *owner*; evitar lógica repetida em apps ou packages.
- Tipos TypeScript não provam dado remoto: validar na borda e em runtime quando necessário.
- Compatibilidade entre cliente, assets, GNET, servidor e DB do jogo deve ser explicitamente homologada.
- Não confundir referências de release ou schema v2 com a versão v1 dos manifests do PR #23.
- `[PROPOSTO]` não constitui serviço/deployment ou aceitação de produção.

## 2. Árvore de arquivos e superfície exportada

```text
packages/update-kit/
├── package.json
├── src/index.ts
├── SPECS.md
└── TODO.md
```

### Contratos atuais (tipos, endpoints, dados ou funções)

1. digest(Uint8Array)->SHA-256 hex; canonicalJson(value) ordena keys lexicograficamente, proíbe floats não seguros e serializa arrays/objetos.
2. verifyManifest(raw,expectedDigest,keys): Zod schema2, recalcula digest de campos sem releaseDigest/signature, compara com expectedDigest e verifica assinatura Ed25519 de `pw-client-release-v2\n${digest}` usando public key allowlisted.
3. prepareClient({manifestUrl,expectedDigest,trustedKeys,onProgress,signal}): GET manifesto no-store, limite texto 2MiB, verifica e busca entry.js, entry.wasm e files; progresso completed/total/downloaded/phase.
4. verifiedBytes: HTTPS (localhost permite HTTP), bytes max 256 MiB por objeto, CacheStorage `pw-assets-sha256-v1` por SHA-256, rehash do cache, fetch com credentials omitted, verificação de tamanho/digest e put após prova.
5. canLaunchOriginalClient(): boolean retorna false; não simula um WASM funcional; integração com PR #23 pendente.

### Modelo conceitual e invariantes

- UpdateManifest schema2: clientVersion, releaseDigest, assetSetDigest, entry.js/wasm, files[].path/url/sha256/bytes, assinatura keyId/value.
- Release digest é SHA-256 de JSON canônico sem releaseDigest/signature; signature Ed25519 sobre prefixo fixo+hash; ferramentas tools/release/sign-client-v2.mjs e test:updater compartilham esta regra.
- Cache key absoluto `location.origin/__pw_hash_cache__/${sha}`; conteúdo não se confunde por path lógico; hash semelhante pode ser reusado entre releases.
- Asset set é referência de digest, mas DAG de manifests de conteúdo do PR #23 ainda não é percorrido; files[] é lista plana.
- Sem data-profile binding verificado no client-side além do bootstrap da API; pairing entregue depende de control-api.

## 3. Mapa de relações: produtores, consumidores e ownership

- apps/shell chama prepareClient após POST launcher/sessions, usando versão/digest retornados, não configurados pelo jogador.
- @pw/contracts.updateManifest e sha256Hex validam formato; tools/release/sign-client-v2.mjs assina release com chave privada CI, public key entra no shell.
- apps/control-api registra digest release, pairing/realm; está pendente verificar automaticamente se release publicada tem assinatura real.
- src/client153/PR #23: Emscripten JS glue + WASM/VFS/Angelica; tools/assets PR #23: AssetSetIndex→BundleCatalog→BundleManifest→objects/chunks.
- @pw/ui apresenta progress/cache/erros e futuramente status por tipo de download; API SDK serve auth/catalog, mas asset fetch deve omitir cookies.

| Direção | Módulo | Responsabilidade de integração |
| --- | --- | --- |
| Produtor | Este package | Exporta tipos/funções/dados com contrato estável e versionado |
| Consumidores | Apps listadas acima | Importam pelo package name; não clonam implementação |
| Infra/persistência | Control API/servidor/CI conforme camada | Não contornar autenticação ou dono do banco |
| Evolução | `SPECS.md` + `TODO.md` + contratos compartilhados | Atualizar código/teste antes de marcar status concluído |

## 4. Regras de segurança, consistência e integridade

- Chave pública confiável é configurada no build, não no manifesto da rede; unknown key rejeita.
- Publisher emite JSON canônico e loader compara texto canônico exatamente, rejeitando chaves duplicadas/representação divergente. A API verifyManifest de objeto puro não examina bytes; só prepareClient garante canonical bytes.
- Manifest restringe path relativo, sem traversal/backslash/percent e com detecção de colisão case-insensitive; download só da mesma origem HTTPS do manifesto. Ainda faltam normalização Unicode NFC, MIME/SRI e limite total de bundle.
- Fetch faz leitura streaming limitada a tamanho declarado e máximo 256 MiB/objeto, mas ainda junta partes em memória/CacheStorage; migrar para OPFS/chunks e orçamentos menores no mobile.
- Não há eviction, leases multi-tab, atomic install de release completa, crash recovery, quota fallback ou cancel cleanup abrangente; implementar.
- O endpoint de preflight agora prepara arquivos SEM ticket; /launcher/sessions exige digest e pairing após update, porém a chamada final do shell permanece pendente enquanto o WASM não inicia.
- Sem service worker agora; não criar segunda autoridade de atualização sem protocolo/garantia.
- Chave de produção não pode ser usada em previews não confiáveis; Direito de distribuição de assets é gate humano obrigatório.

**Falha segura:** rejeitar formato desconhecido, versão incompatível, hash incorreto, endpoint não confiável, inconsistência de realm/zone/edition ou credencial inválida. Nunca fazer fallback para latest, produção ou binário não homologado.

## 5. Fluxo de implementação e integração

1. Definir contrato local e consumidor/produtor explícitos.
2. Criar schema/fixture ou migration idempotente de dados/artefato e documentar sua versão.
3. Validar erro, replay, concurrency, compatibilidade e limites (tempo/memória/tamanho) antes da promoção.
4. Atualizar primeiro o package; só depois os apps de consumo. Não criar uma segunda autoridade.
5. Homologar com PR #17 e PR #23 quando envolver GNET, WDB, WASM ou asset profile.
6. Publicar evidência do build e testes com commit SHA; informar precisamente o que ainda não foi executado.

## 6. Testes e critérios de aceitação

- CI test:updater: Ed25519 válido, digest adulterado, chave desconhecida e assinatura por chave errada.
- Negativos propostos: JSON duplicado, size bomb, path ../, URL maliciosa, archive bomb, 404, HTML no lugar de WASM, truncated asset.
- Testes browser de cache cold/warm, duas abas, interrupção, quota, perda de rede, servidor/versão diferentes.
- Testes reais do release v145 em desktop Android/iOS simulados/physical com latency/FPS/heap/asset bytes medidos.

### Comandos de CI e reprodução

- TypeScript: `npm run typecheck`; três SPAs: `npm run build:web`.
- Database e integração HTTP: `npm run db:migrate` e `node tests/platform/api-smoke.mjs`, requerem PostgreSQL e control-api.
- Contratos/updater: `npm run test:platform` e `npm run test:updater`.
- Rust: `cargo test --workspace` para integração gateway/realm-agent (quando aplicável).
- Para gameplay verdadeiro: test harness Chrome desktop/mobile, servidor `gs` com dados autorizados, login→CreateRole→EnterWorld. **Ainda não está comprovado nesta branch.**

## 7. Riscos técnicos e decisões abertas

1. A política de versionamento de manifests e OpenAPI precisa evitar ambiguidades entre schemas v1/v2.
2. Envolver backend real exige isolation por realm/env/zone e prevenção contra vazamento de WDB/secret.
3. Mudanças que dependem de runtime/client original precisam ser executadas em branch/PR compatíveis; não considerar fixture diagnóstico como prova de jogo.
4. A estabilidade do package depende de interfaces negativas testadas (hash, protocolo, schema, assinatura, replay, storage, concorrência e sessão).
5. Review humano e verificação jurídica são gates para publicar assets ou fontes de terceiros.

## 8. Definition of Done e governança do package

- [ ] API e contratos versionados; compatibilidade aprovada entre produtor/consumidor.
- [ ] Cobertura positiva/negativa em CI e testes de runtime onde aplicável.
- [ ] Security review, PII/secrets e operação multi-realm revisados.
- [ ] Sem lógica duplicada em outras apps/packages, imports consistentes.
- [ ] Documentos de arquitetura, schemas e TODO alinhados ao código.
- [ ] Evidência no PR com commit, resultado e limites, e aprovação antes de produção.

**Backlog executável:** [TODO.md](TODO.md), com responsabilidades, dependências e critérios de aceite individuais.

> **Nota de manutenção (2026-10-28):** status verificado, dependências e itens parciais desta área são acompanhados em [P0-IMPLEMENTATION-REPORT.md](../../docs/P0-IMPLEMENTATION-REPORT.md). Fluxos de gameplay dependentes do PR #23/gs continuam pendentes.
