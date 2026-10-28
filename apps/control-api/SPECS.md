# SPECS.md — Control Plane API / domínio e autoridade do PW Web

> **Componente:** `apps/control-api` · **Status:** implementação inicial (MVP em evolução) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Fonte de verdade do código:** caminhos abaixo. **Referências integradas:** `docs/PAINEL.md`, `docs/PLATFORM-IMPLEMENTATION.md`, `packages/contracts/src/index.ts`, `packages/database/migrations/001_platform.sql`.
> **Regra:** `[ATUAL]` descreve o commit inspecionado; `[ALVO]` descreve contrato/feature ainda não implementados. Não confundir especificação com comportamento testado.

## 1. Missão, problema e decisões de produto

- Ser única autoridade de identidade web, realms, versions, releases, pairings, canais, deployments, tickets, permissões e filas de operação.
- Separar definitivamente control DB PostgreSQL do WDB/BDB autoritativo do jogo; impedir launcher/gateway/agent de acessar SQL livremente.

**Stack e deploy:** Node.js 22, TypeScript, Hono 4, @hono/node-server, pg/PostgreSQL 16, Zod/scrypt.

**Situação comprovada [ATUAL]:** Implementação inicial Node local (não Worker Cloudflare); APIs de identidade, registry e jobs parciais; validação CI HTTP/PostgreSQL passou, sem hardening de produção.

**Regra transversal:** jogador escolhe **servidor** e autentica; o control plane fixa **automaticamente** a versão de cliente e os assets homologados daquele servidor. Nenhuma release não comprovada é lançada no navegador.

## 2. Localização e árvore do componente

```text
apps/control-api/
├── src/app.ts
├── src/server.ts
├── src/security.ts
├── package.json
├── SPECS.md  # contrato deste componente
└── TODO.md   # backlog executável com dependências e aceite
```

Dependências de build e runtime devem ser lidas no `package.json` ou `Cargo.toml` local; `Cargo.toml` na raiz do repo inclui as aplicações Rust.

## 3. Responsabilidades e fronteiras

### 3.1 Domínio exclusivo / ownership

- transações e políticas de negócio para CRUD de realms, releases, deployments, channels e pairings
- hash scrypt assíncrono das senhas web, limitação SQL de login/registro, cookie pw_session HttpOnly/Strict, web_sessions persistidas e tickets de gateway one-shot
- readiness semântico e audit_events/operation_jobs por actor e idempotency_key

### 3.2 O que é expressamente proibido neste componente

- renderizar WASM, assinar releases ou servir assets binários originais
- fazer systemctl, chamar GNET admin diretamente ou gerenciar WDB sem adapter/agent
- marcar preview READY no dispatch do GitHub; emitir certificado de conteúdo sem gate publisher

A UI/admin **nunca** executa scripts arbitrários; dados persistentes do jogo permanecem no WDB/BDB gerenciado pela stack do servidor, enquanto versões, deploys, contas web e jobs pertencem ao PostgreSQL do control plane.

## 4. Interfaces, operações e contratos públicos

1. GET /healthz {status:'alive'} e GET /readyz: teste de PostgreSQL, não prova de jogo.
2. POST /v1/auth/register|login|logout e GET /v1/auth/me; COOKIE pw_session, token guardado como SHA-256 em web_sessions.
3. GET /v1/catalog/realms e /v1/catalog/realms/:id; estado online público exige health recente do deployment e pairing supported.
4. POST /v1/launcher/releases {realmId,channel:'stable'} -> release pinada SEM ticket, para atualização. POST /v1/launcher/sessions {realmId,channel:'stable',expectedPairingId,expectedClientDigest} -> bootstrap com ticket de 90s APÓS atualização, 409 para mudança de versão/realm e não READY.
5. POST /v1/internal/tickets/consume {ticket,deploymentId}; bearer GATEWAY_CONTROL_TOKEN; UPDATE consumed_at atômico com expires_at e deployment.
6. POST/PATCH /v1/admin/realms, GET /v1/admin/overview, GET/POST /v1/admin/pairings, GET /v1/admin/channels e pin CAS por revision.
7. POST /v1/admin/releases/clients|servers, POST /v1/admin/releases/:kind/:id/status, GET/POST /v1/admin/deployments, POST /v1/internal/deployments/:id/health com bearer+prova Ed25519 temporal por deployment.
8. POST /v1/admin/previews com Idempotency-Key e createPreviewInput; GET /v1/admin/jobs; 202 queued, não significa preview criado.

**Norma de evolução de contratos:** mudanças em wire JSON/HTTP seguem `packages/contracts` + migracões compatíveis; mudanças C++ GNET precisam evidência de framing e compatibilidade do port PR #23 e do servidor PR #17. Não criar contratos duplicados por aplicação.

## 5. Modelo operacional, fluxo e máquina de estados

1. REGISTER → hash scrypt unique email → LOGIN → web_sessions SHA-256 → SESSION RESOLVE; 401 vs 403 tipados.
2. REALM OFFLINE → releases draft→validated→published → pairing supported com evidence → channel stable pin CAS → deployment pending→ready por probe → LOGIN/LAUNCH.
3. LAUNCHER SESSION só quando realm online, pairing supported, releases published, deployment ready e health_checked_at <30s; emite game_ticket 90s.
4. PREVIEW REQUEST -> operation_jobs queued → operations-worker dispatch; controle API não realiza operações externas no request path.

**Princípios:** identificadores estáveis `realm_id` e `deployment_id`; releases pinadas pelo SHA-256; channel pin protegido por `expectedRevision`; session ticket curto e de uso único. Diferenciar `liveness`, `transport_ready` e `gameplay_ready`.

## 6. Mapa de dependências e colaboração

- @pw/contracts valida todos os inputs Zod; @pw/database centraliza Pool e migrations/tx; @pw/api-sdk é consumidor remoto.
- apps/shell/player-portal/admin-console usam HTTP; gateway consome ticket privado; realm-agent/health probe informa status; operations-worker lê job queue.
- src/server153: limite de confiança nativo por gateway/adapter, não por query direta; tools/release: digest e aprovação das releases.
- Infra futura Cloudflare/Hyperdrive: o código atual usa Node e pg; portar para Worker requer driver/transações e segredo seguros.

**Regra anti-duplicação:** interfaces de HTTP, dados, assets, segurança e versionamento pertencem aos packages indicados. Este app deve consumir essas APIs; lógica comum não deve ser colada em várias aplicações.

## 7. Segurança, consistência e tolerância a falhas

- CRÍTICO: registro/login público ainda sem rate limit, email verification, MFA, CSRF robusto e revogação multi-device.
- CRÍTICO: Bearer dos endpoints internos é simples shared-secret; necessitar mTLS/service auth, replay protection e autorização de origem.
- CRÍTICO: status 'supported' e release 'published' podem ser marcados por admin com evidence URL sem verificador CI; não são prova cripto automática.
- CRÍTICO: bootstrap não tem assinatura própria; clients recebem ticket de 90s antes do download, que pode expirar; redesenhar.
- Garantir query parametrizada, transações multi-write, FK, idempotência sem substituir payload de request conflitante, read-your-write e CAS.
- Isolamento por realm/env, session binding a deployment e gateway profile, TTL de health, anti-collision de dados WDB/zone e autorização granular.

Falhas devem resultar em código/estado explícito e **fail-closed**: não trocar silenciosamente para outra versão, realm, endpoint, backend, release de produção, dados WDB ou chaves desconhecidas. Log sem PII, password, token ou payload sensível.

## 8. Plano de validação e evidências exigidas

- CI HTTP+PostgreSQL cobre register/login, RBAC player, create realm, 409 jogo inexistente, logout e idempotency header obrigatório.
- Cobrir transações sob concorrência de pin, ticket consume paralelos e use-once, expiração, role downgrade e channel drift.
- Testar origins/CSRF/replay, brute force, payload tamper, redirects/SSRF em URLs de releases e mensagens de erro sem segredos.
- Teste end-to-end do ramo READY requer servidor real, prova de gs+login→mundo e cliente portado.

**Comandos gerais:** `npm run typecheck`, `npm run build:web`, `npm run test:platform`, `npm run test:updater`, `cargo test --workspace` e `npm run db:migrate` conforme camada. **Não** marcar teste como aprovado sem observar logs do GitHub Actions ou execução independente.

## 9. Critérios de Definition of Done

- Entradas e respostas validadas pelo contrato compartilhado e integradas de ponta a ponta.
- Segurança e isolamento por usuário/realm/env/protocolo com testes negativos.
- Erros tipados, logs e idempotência de operações verificáveis.
- Testes automatizados com evidência/commit SHA e cenários de falha.
- Documentação atualizada do status factual e das limitações.
- Sem secrets, source de terceiros não autorizada, assets originais ou WDB de produção no Git/artefatos públicos.
- Acompanhamento de progressão detalhada em [TODO.md](TODO.md). Cada item inclui seu próprio aceite.

## 10. Plano de integração incremental

| Etapa | Foco | Porta de saída |
| --- | --- | --- |
| Fundação | Conformidade com contracts, database e permissões | CI tipos/schema, migração e segurança |
| Integração | Coordenação entre as sete apps e os packages | API/serviço real, sem mocks enganosos |
| Gameplay | Cliente Emscripten + gateway GNET + servidor/gs/WDB | Login→role→world comprovado |
| Operação | Previews isolados, health, observabilidade, rollback | Testes adversariais, audit e runbook |
| Produção | Infra, TLS, segredo, direitos e capacidade | Gate humano com evidência e rollback |

**Decisões abertas** devem virar ADR linkado no documento mestre; não assumir design final em backlog. Consulte os itens `P0` antes de promover qualquer deployment.

> **Nota de manutenção (2026-10-28):** status verificado, dependências e itens parciais desta área são acompanhados em [P0-IMPLEMENTATION-REPORT.md](../../docs/P0-IMPLEMENTATION-REPORT.md). Fluxos de gameplay dependentes do PR #23/gs continuam pendentes.
