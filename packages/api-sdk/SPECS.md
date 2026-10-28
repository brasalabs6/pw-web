# SPECS.md — SDK HTTP tipado para as interfaces web

> **Componente:** `packages/api-sdk` · **Package:** `@pw/api-sdk` · **Status:** implementação inicial em `feat/multi-realm-platform-shell-mvp`.
> **Autoridade:** o código versionado neste package, `docs/PAINEL.md` e `docs/PLATFORM-IMPLEMENTATION.md`. `[ATUAL]` = implementado no repo, `[PROPOSTO]` = objetivo ainda sujeito a código/teste.
> **Objetivo central da plataforma:** shell mostra servidores; o cliente e sua atualização são determinados automaticamente pelo pairing do realm. O package mantém sua própria fronteira sem inventar suporte 1.3.6/1.4.4.

## 1. Propósito e princípios de design

- Concentrar contratos HTTP consumidos por shell, portal e console e unificar erros, autenticação e serialização de requests.
- Evitar componentes React reinventarem fetch/JSON/cookie handling; preservar lógica de versão automática por realm sem cliente selecionável manualmente.
- Fazer mudanças API versionadas com validação de payload/response ao lado de @pw/contracts.

**Stack:** TypeScript + native fetch + cookies credentials include; @pw/api-sdk.

**Situação comprovada [ATUAL]:** PwApi com wrappers para realms/me/auth/launch/overview/createRealm/updateRealm/previews/jobs; API request genérica para funções restantes. Ainda sem cliente OpenAPI gerado, retry/cancel/deadline, observabilidade nem estratégia de múltiplas origens.

### Princípios inegociáveis

- Cada dado/ação tem um *owner*; evitar lógica repetida em apps ou packages.
- Tipos TypeScript não provam dado remoto: validar na borda e em runtime quando necessário.
- Compatibilidade entre cliente, assets, GNET, servidor e DB do jogo deve ser explicitamente homologada.
- Não confundir referências de release ou schema v2 com a versão v1 dos manifests do PR #23.
- `[PROPOSTO]` não constitui serviço/deployment ou aceitação de produção.

## 2. Árvore de arquivos e superfície exportada

```text
packages/api-sdk/
├── package.json
├── src/index.ts
├── SPECS.md
└── TODO.md
```

### Contratos atuais (tipos, endpoints, dados ou funções)

1. PwApi(base=''): request<T>(url,options); usa fetch(base+url, credentials:'include'), Content-Type JSON somente se body, parse JSON e lança ApiError(status,message) em !ok.
2. realms() -> {realms:CatalogRealm[]}; me() -> {user}; login(email,password), register(email,password,displayName), logout().
3. launch(realmId) -> POST /v1/launcher/sessions com channel:'stable'; não recebe clientVersion nem URL de servidor arbitrária.
4. overview(), createRealm(...), updateRealm(id,...), previews({kind,commits,ttlHours}) com Idempotency-Key crypto.randomUUID(), jobs().
5. ApiError estende Error e carrega status; a maioria das respostas é tipada por TS mas não validada com Zod em runtime.
6. Admin release/pairing/deployment endpoints ainda são chamados via request genérica diretamente pelos componentes.

### Modelo conceitual e invariantes

- AuthUser: {id,email,displayName,role}; role é string no SDK, validação forte do RBAC fica no backend.
- CatalogRealm de @pw/contracts; LaunchBootstrap de @pw/contracts usado no shell.
- ApiError {status,message} não padroniza problem+json, trace_id ou causas seguras.
- Cookies HttpOnly não são acessados pelo SDK; apenas enviados automaticamente conforme browser e política do servidor.

## 3. Mapa de relações: produtores, consumidores e ownership

- apps/shell chama realms/me/login/launch; player-portal chama register/login/logout; admin-console usa overview/CRUD/release via request.
- @pw/contracts deveria validar response JSON na borda, não confiar em cast TS; @pw/ui AuthPanel usa SDK, aumentando o acoplamento da UI ao transporte.
- apps/control-api é authority de API; @pw/database não é dependência do SDK.
- @pw/update-kit recebe URLs do bootstrap aprovado e baixa assets separadamente com credentials omit, não usa SDK para bytes grandes.
- Env dev Vite proxy /v1; produção precisa domínio/API base confiável e same-site/cookie policy explícita.

| Direção | Módulo | Responsabilidade de integração |
| --- | --- | --- |
| Produtor | Este package | Exporta tipos/funções/dados com contrato estável e versionado |
| Consumidores | Apps listadas acima | Importam pelo package name; não clonam implementação |
| Infra/persistência | Control API/servidor/CI conforme camada | Não contornar autenticação ou dono do banco |
| Evolução | `SPECS.md` + `TODO.md` + contratos compartilhados | Atualizar código/teste antes de marcar status concluído |

## 4. Regras de segurança, consistência e integridade

- Sem timeout/retries e cancelamento, UI pode ficar travada em rede ruim. Retry só operações idempotentes com safe backoff.
- Generic request não valida URL ou origin; frontend não deve aceitar base URL oriunda do realm ou query sem allowlist.
- Credenciais cross-origin/cookies SameSite Strict exigem mesmo site ou arquitetura de sessão aprovada.
- Falha 204 No Content e erros não-JSON retornam Invalid response; contrato de problem responses precisa ser estável.
- Idempotency-Key foi criado no ato da request; retry automático da mesma ação deve reutilizar a chave original, não gerar outra.
- SDK nunca deve expor `GATEWAY_CONTROL_TOKEN`, senha DB ou informação pessoal em logs.

**Falha segura:** rejeitar formato desconhecido, versão incompatível, hash incorreto, endpoint não confiável, inconsistência de realm/zone/edition ou credencial inválida. Nunca fazer fallback para latest, produção ou binário não homologado.

## 5. Fluxo de implementação e integração

1. Definir contrato local e consumidor/produtor explícitos.
2. Criar schema/fixture ou migration idempotente de dados/artefato e documentar sua versão.
3. Validar erro, replay, concurrency, compatibilidade e limites (tempo/memória/tamanho) antes da promoção.
4. Atualizar primeiro o package; só depois os apps de consumo. Não criar uma segunda autoridade.
5. Homologar com PR #17 e PR #23 quando envolver GNET, WDB, WASM ou asset profile.
6. Publicar evidência do build e testes com commit SHA; informar precisamente o que ainda não foi executado.

## 6. Testes e critérios de aceitação

- Unit com mock fetch: JSON, 401/403/409/422/503, 204, response malformada, AbortSignal e network exception.
- E2E com Node/PostgreSQL do CI; assert de no client selector e stable channel.
- Teste de CSRF/credentials por domínios do shell, portal e admin; headers e cookie semantics.

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
