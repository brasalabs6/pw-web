# TODO.md — Control Plane API / domínio e autoridade do PW Web

> **Escopo:** `apps/control-api` · **Fonte de arquitetura:** [SPECS.md](SPECS.md) · **Data base:** 2026-10-28.
> **Convenção:** `[x]` = implementado no código analisado (não significa produção); `[ ]` = pendente. `P0` bloqueia MVP; `P1` operacionalização; `P2` evolução. Não marcar checklist concluída sem evidência de execução.

## Estado atual de referência

- [x] Estrutura/escopo presente: transações e políticas de negócio para CRUD de realms, releases, deployments, channels e pairings
- [x] Estrutura/escopo presente: hash scrypt das senhas web, cookie pw_session HttpOnly/Strict, web_sessions persistidas e tickets de gateway one-shot
- [x] Estrutura/escopo presente: readiness semântico e audit_events/operation_jobs por actor e idempotency_key
- [x] Arquivo SPEC/TODO criado para guiar implementação no diretório deste componente.
- [ ] Funcionalidade plenamente homologada em produção com prova real de gameplay, segurança e restore.

## Backlog rastreável com dependências e critério de aceite

### P0 — Bloqueantes

- [ ] **API-01 — Definir módulos internos por domínio e serviços de aplicação/testes unitários**
  - **Depende de:** contracts,database.
  - **Aceite:** Rotas finas, políticas e transações em serviços testáveis.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **API-02 — Proteger login/register com rate-limit, email verification, CSRF e MFA admin**
  - **Depende de:** security, database.
  - **Aceite:** Evidência adversarial de brute force e CSRF.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **API-03 — Fazer release validation/pairing approval depender de CI assinado e prova GNET**
  - **Depende de:** tools/release, gateway.
  - **Aceite:** Não aprova por URL de evidência apenas.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **API-04 — Resolver bootstrap assinado com tickets emitidos após download e policy por realm**
  - **Depende de:** shell, update-kit, gateway.
  - **Aceite:** Sem ticket expirado no loading e sem troca de pairing.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **API-05 — Endurecer endpoint health (attestation da probe e identidade da implantação)**
  - **Depende de:** realm-agent, deploy provider.
  - **Aceite:** Probe falsa não publica realm ONLINE.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **API-06 — Estabelecer transações SQL/CAS e RLS/scoping para dados multi-realm**
  - **Depende de:** database.
  - **Aceite:** Sem channel race ou leak cross-realm.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P1 — Robustez e operação

- [ ] **API-07 — Criar domínio config revisions, maps, operations plan/apply**
  - **Depende de:** realm-agent, operations-worker.
  - **Aceite:** Operações idempotentes, auditadas e reversíveis.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **API-08 — Gateway IAM service-to-service com mTLS e secret rotation**
  - **Depende de:** gateway, agent, infra.
  - **Aceite:** Tokens rotacionam sem indisponibilidade.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **API-09 — Implementar account bridge GNET, game-account ownership e recovery**
  - **Depende de:** server153 adapter, player-portal.
  - **Aceite:** Criar/login/character com servidor real.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **API-10 — Implementar provider de preview/rollout completo, referências de artefatos e retention**
  - **Depende de:** operations-worker, tools/release.
  - **Aceite:** Preview isolado com recursos reconciliáveis.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P2 — Evolução

- [ ] **API-11 — Portar para Workers/Hono após prova de pg/Hyperdrive e bindings**
  - **Depende de:** infra/cloudflare.
  - **Aceite:** Transações e dados com paridade CI antes da migração.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

## Plano de execução por dependência

1. Resolver os `P0` de contracts/database/API/service auth exigidos pelo componente; não usar fixture como produção.
2. Integrar ambiente de teste e happy path com dados/artefatos legalmente disponíveis.
3. Executar cenários negativos, concorrência, isolamento, rollback e regressão.
4. Revisar custos/segurança/monitoramento e concluir P1 sob autorização; P2 não deve interromper entrega do port WASM.
5. Atualizar `SPECS.md`, `docs/PAINEL.md` e o próprio checklist com evidências concretas antes de marcar `[x]`.

## Gates que bloqueiam marcação de pronto

- [ ] Contratos/API/schema versionados e aceitos pelos produtores/consumidores.
- [ ] Permissões, secrets, escopo por realm/env e erros de usuário testados.
- [ ] Teste de integração real (e, se aplicável, login GNET/gs/EnterWorld).
- [ ] Resultado CI/benchmark atual anexado ao PR sem alegar métricas inexistentes.
- [ ] Infra isolada e rollback/backup documentados para mutações persistentes.
- [ ] Operador/reviewer aprova promoção para produção; feature flag/rollback definidos.

**Nota:** as entregas das outras apps/packages estão vinculadas às dependências acima, mas cada dono valida seu próprio aceite. Tarefas de gameplay só podem ser marcadas depois que o PR #23 finalizar a migração WebAssembly e o PR #17 comprovar a stack `gs` em runtime homologado.
