# SPECS.md — Contratos e schemas compartilhados da plataforma

> **Componente:** `packages/contracts` · **Package:** `@pw/contracts` · **Status:** implementação inicial em `feat/multi-realm-platform-shell-mvp`.
> **Autoridade:** o código versionado neste package, `docs/PAINEL.md` e `docs/PLATFORM-IMPLEMENTATION.md`. `[ATUAL]` = implementado no repo, `[PROPOSTO]` = objetivo ainda sujeito a código/teste.
> **Objetivo central da plataforma:** shell mostra servidores; o cliente e sua atualização são determinados automaticamente pelo pairing do realm. O package mantém sua própria fronteira sem inventar suporte 1.3.6/1.4.4.

## 1. Propósito e princípios de design

- Tornar o package a fonte única de formatos serializáveis HTTP, IDs, schemas de entrada, máquinas de estado e compatibilidade de versões.
- Separar validação de objetos JSON do protocolo nativo GNET: C++ wire framing e estruturas do PW 1.5.3 exigem specs/testes próprios, não podem ser inferidos da forma de uma API Zod.
- Padronizar names e discriminantes sem duplicar lógica nas apps; tornar evoluções compatíveis e explicitamente versionadas.

**Stack:** TypeScript, Zod 4, ES modules; package @pw/contracts.

**Situação comprovada [ATUAL]:** Schemas JSON de API e updater v2 em `src/index.ts`; validados por TS no CI. Ainda não há OpenAPI gerado, golden fixtures de todas as rotas, schema GNET nem sincronização automática com Rust.

### Princípios inegociáveis

- Cada dado/ação tem um *owner*; evitar lógica repetida em apps ou packages.
- Tipos TypeScript não provam dado remoto: validar na borda e em runtime quando necessário.
- Compatibilidade entre cliente, assets, GNET, servidor e DB do jogo deve ser explicitamente homologada.
- Não confundir referências de release ou schema v2 com a versão v1 dos manifests do PR #23.
- `[PROPOSTO]` não constitui serviço/deployment ou aceitação de produção.

## 2. Árvore de arquivos e superfície exportada

```text
packages/contracts/
├── package.json
├── src/index.ts
├── SPECS.md
└── TODO.md
```

### Contratos atuais (tipos, endpoints, dados ou funções)

1. realmStatus = offline | maintenance | starting | online; releaseStatus = draft | validated | published | revoked.
2. operationStatus = queued | dispatched | running | ready | failed | cancelled | blocked; atenção: operations-worker usa também `dispatching`, discrepância atual do contrato.
3. previewKind = client | server | paired; createPreviewInput exige commits 40 hex pelo kind, TTL 1..72, realmId opcional.
4. catalogRealm: id uuid, slug, name, description, status, population int|null, defaultChannel; apresentação pública exclui segredos do host.
5. registerInput: email normalizado, password com mínimo 12 e max 128, displayName 2..60; loginInput email+password.
6. launcherSessionInput {realmId uuid,channel:stable|beta|preview}; seleção do cliente é automática no backend, não campo de usuário.
7. createRealmInput slug 3..48, name 3..100, description default '', defaultChannel stable|beta; updateRealmInput exige pelo menos um campo.
8. releaseCreateInput: version, digest SHA256, HTTPS artifactUrl, protocolProfile, dataProfile, status; API de import força draft mesmo se input indicar status superior.
9. pairingCreateInput clientReleaseId/serverReleaseId, protocolProfile, dataProfile, status untested/preview_only/supported/incompatible, evidenceUrl opcional.
10. patchChannelInput pairingId uuid + expectedRevision inteiro CAS.
11. LaunchBootstrap type: schema2, realmId, channel, pairingId, client{version,releaseDigest,manifestUrl}, server{version,releaseDigest,deploymentId}, gateway{url,ticket,ticketExpiresAt}, assetBaseUrl.
12. updateManifest Zod schema2: clientVersion, releaseDigest, assetSetDigest, entry{js,wasm} {url,sha256,bytes}, files[]{path,url,sha256,bytes}, signature{keyId,value}; campos desconhecidos recusados no top-level.
13. compatiblePair(client,server): compara igualdade exata de protocol_profile/data_profile; é pré-filtro, NÃO prova real de wire/GNET/gs.

### Modelo conceitual e invariantes

- IDs public UUID v4 / digest SHA-256 lowercase de 64 hex; commit SHA-1 git de 40 hex para candidatos PR.
- Realm.status não é inferência de liveness; observação de gs e login→world deve ser independente.
- Release channel aponta para pairing e revision CAS; release digest é identidade imutável; assets possuem assetSetDigest independente.
- Estado 'supported' depende de evidência humana/CI; falta verificação automatizada; não deve permitir bypass por admin no produto final.
- Control API e JSON Schema v1 do PR #23 são versões distintas: evolução v2 não altera arquivos v1 in-place.

## 3. Mapa de relações: produtores, consumidores e ownership

- apps/control-api: producer/consumer de Zod para input, registry e session; deve parar de duplicar validators inline.
- apps/shell + @pw/api-sdk: tipos CatalogRealm, LaunchBootstrap; o launcher só pode iniciar release homologada.
- apps/admin-console: formulário de realms/releases/preview/pairing, estado de jobs e rollout.
- apps/operations-worker: valida createPreviewInput e opera estados; alinhar `dispatching` no enum.
- @pw/update-kit + tools/release/sign-client-v2.mjs: mesma canonicalização, IDs e manifesto; não misturar manifest v2 com schema v1 do PR #23.
- apps/game-gateway e apps/realm-agent: Rust consome wire HTTP com JSON manual no momento; gerar fixtures/OpenAPI e testes cross-language.

| Direção | Módulo | Responsabilidade de integração |
| --- | --- | --- |
| Produtor | Este package | Exporta tipos/funções/dados com contrato estável e versionado |
| Consumidores | Apps listadas acima | Importam pelo package name; não clonam implementação |
| Infra/persistência | Control API/servidor/CI conforme camada | Não contornar autenticação ou dono do banco |
| Evolução | `SPECS.md` + `TODO.md` + contratos compartilhados | Atualizar código/teste antes de marcar status concluído |

## 4. Regras de segurança, consistência e integridade

- Zod valida estrutura mas não prova assinatura, compatibilidade GNET, rights, storage/schema ou status operacional; esses são gates adicionais.
- Versionamento explícito major nos manifestos; mudanças incompatíveis requerem novo schema e migration/adapters.
- JSON canonical: keys lexicográficas, inteiros seguros, nenhuma duplicate key; parse/browser deve rejeitar conflitos e import externo.
- Paths de files no updateManifest hoje são apenas string: criar validação de normalização, traversal, limite de profundidade/nome e collision.
- Requerem-se limites de payload, listas, tempo, length de URL, source allowlist e schema de evento/version; TypeScript não é autorização runtime.

**Falha segura:** rejeitar formato desconhecido, versão incompatível, hash incorreto, endpoint não confiável, inconsistência de realm/zone/edition ou credencial inválida. Nunca fazer fallback para latest, produção ou binário não homologado.

## 5. Fluxo de implementação e integração

1. Definir contrato local e consumidor/produtor explícitos.
2. Criar schema/fixture ou migration idempotente de dados/artefato e documentar sua versão.
3. Validar erro, replay, concurrency, compatibilidade e limites (tempo/memória/tamanho) antes da promoção.
4. Atualizar primeiro o package; só depois os apps de consumo. Não criar uma segunda autoridade.
5. Homologar com PR #17 e PR #23 quando envolver GNET, WDB, WASM ou asset profile.
6. Publicar evidência do build e testes com commit SHA; informar precisamente o que ainda não foi executado.

## 6. Testes e critérios de aceitação

- Unit Zod positivo/negativo para UUID, sha, ttl, URL, status e rejeição de campos não reconhecidos.
- Golden fixtures da serialização de manifesto e assinatura Node ↔ navegador; já existe `test:updater` para Ed25519.
- Conformidade Rust↔TS para TicketConsume, status codes, idempotency key, bearer e expires_at.
- Compatibilidade cross-version v1/v2 e known wire v145, sem misturar assetSet de versões diferentes.

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
