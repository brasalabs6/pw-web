# SPECS.md — Console administrativo central de realms e releases

> **Componente:** `apps/admin-console` · **Status:** implementação inicial (MVP em evolução) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Fonte de verdade do código:** caminhos abaixo. **Referências integradas:** `docs/PAINEL.md`, `docs/PLATFORM-IMPLEMENTATION.md`, `packages/contracts/src/index.ts`, `packages/database/migrations/001_platform.sql`.
> **Regra:** `[ATUAL]` descreve o commit inspecionado; `[ALVO]` descreve contrato/feature ainda não implementados. Não confundir especificação com comportamento testado.

## 1. Missão, problema e decisões de produto

- Fornecer uma visão administrativa unificada de vários servidores/clientes sem oferecer shell arbitrário nem editar WDB diretamente.
- Gerenciar catálogo de versions/builds, pairing homologado, canal stable, ambiente preview, saúde semântica e, após adaptação, mapas/GM/backups.

**Stack e deploy:** React/Vite; @pw/ui, @pw/api-sdk, control-api com roles operator/admin.

**Situação comprovada [ATUAL]:** CRUD parcial de realms, overview, releases, pairings, canais, deployments e pedidos de preview; falta orquestração real de hosts/mapas/volumes e UX avançada.

**Regra transversal:** jogador escolhe **servidor** e autentica; o control plane fixa **automaticamente** a versão de cliente e os assets homologados daquele servidor. Nenhuma release não comprovada é lançada no navegador.

## 2. Localização e árvore do componente

```text
apps/admin-console/
├── src/App.tsx
├── src/ReleaseManager.tsx
├── src/DeploymentManager.tsx
├── src/main.tsx
├── vite.config.ts
├── package.json
├── SPECS.md  # contrato deste componente
└── TODO.md   # backlog executável com dependências e aceite
```

Dependências de build e runtime devem ser lidas no `package.json` ou `Cargo.toml` local; `Cargo.toml` na raiz do repo inclui as aplicações Rust.

## 3. Responsabilidades e fronteiras

### 3.1 Domínio exclusivo / ownership

- telas Realms, Releases, Compatibility Matrix, Deployments e Previews/Jobs
- fluxos de criação/alteração de configuração declarativa, aprovação e auditoria, não execução direta
- UI e estados de job, evidências, confirmação de riscos e exibição das limitações

### 3.2 O que é expressamente proibido neste componente

- executar systemctl diretamente pelo browser, armazenar segredo em bundle, assinar release no frontend
- escolher versão client pelo jogador; alterar pairing automaticamente por comparação apenas de strings
- marcar deployment ready por HTTP ou autorizar GM sem revisão específica

A UI/admin **nunca** executa scripts arbitrários; dados persistentes do jogo permanecem no WDB/BDB gerenciado pela stack do servidor, enquanto versões, deploys, contas web e jobs pertencem ao PostgreSQL do control plane.

## 4. Interfaces, operações e contratos públicos

1. GET /v1/admin/overview -> realms/clientReleases/serverReleases/jobs; roles operator/admin.
2. POST /v1/admin/realms e PATCH /v1/admin/realms/:id; criação realm offline; manutenção sem stop real automático.
3. GET/POST /v1/admin/releases/{clients,servers}; POST /v1/admin/releases/:kind/:id/status (admin; evidência URL).
4. GET/POST /v1/admin/pairings; GET /v1/admin/channels; POST /v1/admin/realms/:id/channels/:name/pin {pairingId,expectedRevision} CAS.
5. GET/POST /v1/admin/deployments; preview POST /v1/admin/previews exige Idempotency-Key; GET /v1/admin/jobs.
6. Futuro: /v1/admin/maps, /v1/admin/config-revisions, /v1/admin/backups, /v1/admin/player-moderation, /v1/admin/rollouts, /v1/admin/audit.

**Norma de evolução de contratos:** mudanças em wire JSON/HTTP seguem `packages/contracts` + migracões compatíveis; mudanças C++ GNET precisam evidência de framing e compatibilidade do port PR #23 e do servidor PR #17. Não criar contratos duplicados por aplicação.

## 5. Modelo operacional, fluxo e máquina de estados

1. AUTH_WEB -> PERMISSION_CHECK -> OVERVIEW -> select realm/release/deployment/job -> typed mutation -> wait operation status.
2. Relatório de release assume draft→validated→published ou revoked, com evidence URL; dashboard não é autoridade criptográfica.
3. Preview: informa kind client/server/paired, SHA(s) e TTL; API registra job queued; worker faz dispatch; somente smoke real poderia marcar READY.
4. Troca stable: apenas pairing supported, revision CAS; sessões existentes não devem receber WASM misturado.

**Princípios:** identificadores estáveis `realm_id` e `deployment_id`; releases pinadas pelo SHA-256; channel pin protegido por `expectedRevision`; session ticket curto e de uso único. Diferenciar `liveness`, `transport_ready` e `gameplay_ready`.

## 6. Mapa de dependências e colaboração

- @pw/api-sdk: HTTP e sessão; @pw/ui: componentes; @pw/contracts: forms e estados, futuramente gerados do OpenAPI.
- control-api: RBAC/validação/transação/audit; operations-worker: execução de job; realm-agent: processo e métricas via backend; gateway: saúde de transporte.
- @pw/database: transições e histórico somente via control-api; update-kit: metadata/assinatura de release, não chave privada; tools/release: publisher CI.

**Regra anti-duplicação:** interfaces de HTTP, dados, assets, segurança e versionamento pertencem aos packages indicados. Este app deve consumir essas APIs; lógica comum não deve ser colada em várias aplicações.

## 7. Segurança, consistência e tolerância a falhas

- Toda ação destrutiva exige plano, escopo, confirmação com reason, MFA e permissão granular; hoje ainda faltam esses gates.
- Criar realm ou pin deve preservar isolation por realm/env/zone e proteger production/preview.
- Evitar XSS em URLs/evidências, não renderizar JSON/HTML não confiável; sanitizar logs e secrets.
- Exibir distinção desired/observed e daemon health vs gameplay ready; métricas históricas não são provas de uptime atual.
- Operator pode cadastrar objetos, mas publicação/paired status supported exige admin; evoluir para dual approval.

Falhas devem resultar em código/estado explícito e **fail-closed**: não trocar silenciosamente para outra versão, realm, endpoint, backend, release de produção, dados WDB ou chaves desconhecidas. Log sem PII, password, token ou payload sensível.

## 8. Plano de validação e evidências exigidas

- testes de rota/role 401/403, operador vs admin, CAS de channel, duplicate preview idempotent.
- Playwright com os formulários, falhas, tab navegação, estados, acessibilidade, confirmação de ações irreversíveis.
- preview não reporta ready só por dispatch; cross-realm negativo; dados sensíveis redigidos no UI.

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
