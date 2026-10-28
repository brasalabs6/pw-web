# SPECS.md — Banco de dados do control plane e migrations

> **Componente:** `packages/database` · **Package:** `@pw/database` · **Status:** implementação inicial em `feat/multi-realm-platform-shell-mvp`.
> **Autoridade:** o código versionado neste package, `docs/PAINEL.md` e `docs/PLATFORM-IMPLEMENTATION.md`. `[ATUAL]` = implementado no repo, `[PROPOSTO]` = objetivo ainda sujeito a código/teste.
> **Objetivo central da plataforma:** shell mostra servidores; o cliente e sua atualização são determinados automaticamente pelo pairing do realm. O package mantém sua própria fronteira sem inventar suporte 1.3.6/1.4.4.

## 1. Propósito e princípios de design

- Persistir apenas identidade web e estado/controle da plataforma; a fonte autoritativa de personagens e inventário do PW 1.5.3 permanece WDB/BDB.
- Manter integridade relacional de users, sessions, realms, releases, pairings, canais, deployments, game tickets, operations e auditoria.
- Oferecer migrations reproduzíveis e operações transacionais com isolamento/concorrência seguro para produção multi-realm.

**Stack:** PostgreSQL 16, node-postgres (pg), TypeScript/tsx e SQL versionado; package @pw/database.

**Situação comprovada [ATUAL]:** Pool/`connectDb`, migrations 001–004 com controle de checksum e advisory lock, seed de realm offline, transações em rotas críticas e smoke real PostgreSQL. Ainda faltam RLS, transaction facade centralizada, backup/restore e driver Workers.

### Princípios inegociáveis

- Cada dado/ação tem um *owner*; evitar lógica repetida em apps ou packages.
- Tipos TypeScript não provam dado remoto: validar na borda e em runtime quando necessário.
- Compatibilidade entre cliente, assets, GNET, servidor e DB do jogo deve ser explicitamente homologada.
- Não confundir referências de release ou schema v2 com a versão v1 dos manifests do PR #23.
- `[PROPOSTO]` não constitui serviço/deployment ou aceitação de produção.

## 2. Árvore de arquivos e superfície exportada

```text
packages/database/
├── package.json
├── src/index.ts
├── src/migrate.ts
├── src/seed-demo.ts
├── migrations/001_platform.sql
├── migrations/002_security_operations.sql
├── migrations/003_logout_ticket_revocation.sql
├── migrations/004_contract_state_constraints.sql
├── SPECS.md
└── TODO.md
```

### Contratos atuais (tipos, endpoints, dados ou funções)

1. connectDb(): pg.Pool; DATABASE_URL obrigatório; max connections 10/idle 10s; PG_SSL=require ativa SSL com rejectUnauthorized.
2. migrate.ts: enumera `migrations/*.sql` ordenadas, serializa aplicações por pg_advisory_lock, registra SHA em schema_migrations e recusa checksum alterado; sem rollback geral automático.
3. seed-demo.ts: cria/upsert `pwbr-153-lab` com status offline e stable channel; é opcional para desenvolvimento e não provisiona WDB.
4. PostgreSQL extension pgcrypto fornece gen_random_uuid(); pg/migrations são runtime Node, não driver Cloudflare Workers homologado.

### Modelo conceitual e invariantes

- users(id, email UNIQUE, display_name, password_hash, role player/operator/admin, created_at).
- web_sessions(id, user_id FK, token_sha256 UNIQUE, expires_at): hash SHA-256 do token opaco, cookie HttpOnly controlado pela API.
- realms(id, slug UNIQUE, name, description, status, default_channel, population): identidade pública persistente, estado não prova gameplay.
- client_releases e server_releases(id,version,digest UNIQUE,artifact_url,protocol_profile,data_profile,status,created_at): metadata, digests não validam bytes sozinhos.
- pairings(id, client_release_id FK, server_release_id FK, protocol_profile,data_profile,status,evidence_url): unique por par de releases.
- deployments(id,realm_id FK,server_release_id FK,gateway_url,asset_base_url,state,health_checked_at,updated_at): binding de servidor real e health observado.
- realm_channels(realm_id,name,pairing_id,revision,PK realm_id+name): controle da versão automática por servidor com compare-and-swap.
- game_tickets(token_sha256 PK,session_id,realm_id,deployment_id,pairing_id,expires_at,consumed_at): credencial efêmera, uso único atômico.
- operation_jobs(id,kind,state,payload jsonb,result jsonb,actor_id,idempotency_key UNIQUE,created_at,updated_at): fila SQL de ações.
- audit_events(id bigint,actor_id,action,resource,details jsonb,created_at): histórico append-only em nível de aplicação, sem assinatura imutável atualmente.

## 3. Mapa de relações: produtores, consumidores e ownership

- apps/control-api usa o Pool para query paramétrica e CRUD; deve ser único escritor de regras de negócio.
- apps/operations-worker utiliza `SELECT ... FOR UPDATE SKIP LOCKED` na fila e estados `dispatching/dispatched/blocked`.
- apps/game-gateway jamais acessa PostgreSQL: consome tickets via API privada; apps/realm-agent jamais conecta ao DB do portal.
- apps/shell/player-portal/admin-console consomem API tipada, nunca credenciais/queries DB.
- src/server153 persiste dados em WDB/BDB; planejar mapping account ID/realm/zone por adaptador futuro.
- Infra Cloudflare Workers futura exige proxy/Hyperdrive/driver consistente e revisão de conexões/transações.

| Direção | Módulo | Responsabilidade de integração |
| --- | --- | --- |
| Produtor | Este package | Exporta tipos/funções/dados com contrato estável e versionado |
| Consumidores | Apps listadas acima | Importam pelo package name; não clonam implementação |
| Infra/persistência | Control API/servidor/CI conforme camada | Não contornar autenticação ou dono do banco |
| Evolução | `SPECS.md` + `TODO.md` + contratos compartilhados | Atualizar código/teste antes de marcar status concluído |

## 4. Regras de segurança, consistência e integridade

- Falta unique/constraint em transições de releases, papéis por realm, state enum efetivo de operation_jobs e prevenções de race entre approval/health.
- É obrigatório usar transações multi-write: criação realm+channel atual são queries independentes; audit+update não tem atomicidade.
- `operation_jobs` ON CONFLICT idempotency_key atual pode aceitar payload divergente: abortar 409 em conflito semântico.
- Sessão/token/release podem ficar órfãos e secrets URLs podem ser explorados: TTL cleanup, FK e retenção auditados.
- Migração 001 usa IF NOT EXISTS, mas não dá suporte a ALTER schema com version lock; criar versioned migrations e replay dry-run.
- Se WDB sofrer alteração incompatível, migration/backup WDB não é responsabilidade desta SQL; coordenar por server provider.

**Falha segura:** rejeitar formato desconhecido, versão incompatível, hash incorreto, endpoint não confiável, inconsistência de realm/zone/edition ou credencial inválida. Nunca fazer fallback para latest, produção ou binário não homologado.

## 5. Fluxo de implementação e integração

1. Definir contrato local e consumidor/produtor explícitos.
2. Criar schema/fixture ou migration idempotente de dados/artefato e documentar sua versão.
3. Validar erro, replay, concurrency, compatibilidade e limites (tempo/memória/tamanho) antes da promoção.
4. Atualizar primeiro o package; só depois os apps de consumo. Não criar uma segunda autoridade.
5. Homologar com PR #17 e PR #23 quando envolver GNET, WDB, WASM ou asset profile.
6. Publicar evidência do build e testes com commit SHA; informar precisamente o que ainda não foi executado.

## 6. Testes e critérios de aceitação

- CI PostgreSQL service executa migration e smoke web auth/realm, validando conexões reais.
- Testar CAS de channels sob concorrência, consume de tickets sob duas requests e encerramento de sessão.
- Testar rollback de transações quando segundo insert/audit falhar; cleanup e retention.
- Plan/restore de backup PostgreSQL e semântico de WDB devem ser testados separadamente.

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
