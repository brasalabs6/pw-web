# TODO.md — SDK HTTP tipado para as interfaces web

> **Componente:** `packages/api-sdk` · **Especificação canônica:** [SPECS.md](SPECS.md) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Legenda:** `[x]` código visto no repositório; `[ ]` requisito pendente. Prioridade P0 = bloqueante; P1 = hardening/integracão; P2 = evolução.

## Avanço verificado em 2026-10-28

**Escopo:** alterações na branch `feat/multi-realm-platform-shell-mvp` com jobs de TypeScript, PostgreSQL, update-kit, SDK e Rust. `[x]` significa aceite local de workorder específico, não certificação de produção.

- **Parcial** — SDK-02: validação runtime de catálogo, usuário e bootstrap; falta demais responses/admin.
- **Parcial** — SDK-03: AbortSignal + timeout de 15 s; sem retry automático perigoso, falta polling e backoff idempotente.
- **Parcial** — SDK-04: configuração VITE_API_BASE_URL e CORS restrito; falta infraestrutura real de domínios compartilhados e testes CSRF completos.

**Bloqueio transversal:** gameplay real depende do cliente Angelica WASM PR #23 e do servidor com `gs`, WDB e login→mundo comprovados; previews de servidor continuam apenas diagnósticos.

## Baseline existente

- [x] PwApi(base=''): request<T>(url,options); usa fetch(base+url, credentials:'include'), Content-Type JSON somente se body, parse JSON e lança ApiError(status,message) em !ok.
- [x] realms() -> {realms:CatalogRealm[]}; me() -> {user}; login(email,password), register(email,password,displayName), logout().
- [x] launch(realmId) -> POST /v1/launcher/sessions com channel:'stable'; não recebe clientVersion nem URL de servidor arbitrária.
- [x] overview(), createRealm(...), updateRealm(id,...), previews({kind,commits,ttlHours}) com Idempotency-Key crypto.randomUUID(), jobs().
- [x] ApiError estende Error e carrega status; a maioria das respostas é tipada por TS mas não validada com Zod em runtime.
- [x] Documentação local SPEC + TODO criada, com ownership compartilhado e dependências.
- [ ] Aceite integral do package em ambiente de produção com prova cruzada, sem falhas de segurança.

## Backlog detalhado (cada item é workorder independentemente verificável)

### P0 — Fundação/bloqueantes

- [ ] **SDK-01 — Criar métodos tipados para releases, pairings, channels, deployments e jobs**
  - **Depende de:** admin-console, contracts.
  - **Aceite verificável:** Eliminar request raw e duplicação de paths.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **SDK-02 — Validar todas as respostas Zod em runtime + errors padronizados**
  - **Depende de:** contracts, control-api.
  - **Aceite verificável:** Respostas malformadas rejeitadas com código claro.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **SDK-03 — Adicionar AbortSignal/deadlines e retry seguro/idempotente**
  - **Depende de:** shell, admin.
  - **Aceite verificável:** Logout/switch realm cancela requests e não duplica jobs.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **SDK-04 — Definir URL API e cookie policy por ambiente com testes CORS/CSRF**
  - **Depende de:** infra, control-api.
  - **Aceite verificável:** HTTPS e sessão corretos em staging/prod.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

### P1 — Integração/robustez

- [ ] **SDK-05 — Unificar idempotency keys por operação e tracing correlation**
  - **Depende de:** operations-worker, admin.
  - **Aceite verificável:** Retry mantém mesmo job ID e audit trace.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **SDK-06 — Gerar SDK de OpenAPI e contrato de erros versionado**
  - **Depende de:** contracts, api.
  - **Aceite verificável:** No drift entre contratos e backend.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **SDK-07 — Padronizar paginação e polling de status de preview/rollout**
  - **Depende de:** admin, worker.
  - **Aceite verificável:** Sem loops infinitos nem loading enganoso.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

### P2 — Evolução

- [ ] **SDK-08 — Adicionar telemetria frontend com PII redigido**
  - **Depende de:** observability futuro.
  - **Aceite verificável:** Diagnóstico p95 sem vazamento.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

## Cadeia de dependências e sequência de execução

1. Fechar cada P0 que cria contrato compartilhado usado por apps: teste cross-module antes de alteração de consumidor.
2. Executar P0 de segurança/compatibilidade com cenário negativo; falta de schema/arquivo não pode causar fallback silencioso.
3. Executar P1 de durabilidade, monitoramento, testes e ergonomia após end-to-end mínimo.
4. P2 somente depois de validar o port 1.5.3 no navegador e o servidor com `gs` jogável.
5. Atualizar as evidências e marcar `[x]` por workorder somente após execução real.

## Gate do package

- [ ] Código e schema consistentes com `SPECS.md`; nenhum documento afirma feature ausente.
- [ ] Dependências de outras apps/packages integradas sem duplicação.
- [ ] Testes de regressão e adversariais atualizados e verdes no commit do PR.
- [ ] Segurança, concorrência e segregação de dados por realm/env verificadas quando aplicável.
- [ ] Documentação revisada após code review e migração preserva releases/sessões antigas.
- [ ] Qualquer ação de produção tem aprovação explícita, evidência e rollback.

**Contrato obrigatório:** o jogador escolhe o servidor; o backend resolve a versão do cliente automaticamente; o cliente verifica release/assetset/assinatura antes de baixar ou iniciar o jogo.
