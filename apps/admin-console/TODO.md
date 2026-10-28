# TODO.md — Console administrativo central de realms e releases

> **Escopo:** `apps/admin-console` · **Fonte de arquitetura:** [SPECS.md](SPECS.md) · **Data base:** 2026-10-28.
> **Convenção:** `[x]` = implementado no código analisado (não significa produção); `[ ]` = pendente. `P0` bloqueia MVP; `P1` operacionalização; `P2` evolução. Não marcar checklist concluída sem evidência de execução.

## Estado atual de referência

- [x] Estrutura/escopo presente: telas Realms, Releases, Compatibility Matrix, Deployments e Previews/Jobs
- [x] Estrutura/escopo presente: fluxos de criação/alteração de configuração declarativa, aprovação e auditoria, não execução direta
- [x] Estrutura/escopo presente: UI e estados de job, evidências, confirmação de riscos e exibição das limitações
- [x] Arquivo SPEC/TODO criado para guiar implementação no diretório deste componente.
- [ ] Funcionalidade plenamente homologada em produção com prova real de gameplay, segurança e restore.

## Backlog rastreável com dependências e critério de aceite

### P0 — Bloqueantes

- [ ] **ADMIN-01 — Implementar autenticação forte, segregação de permissões e MFA em operações críticas**
  - **Depende de:** control-api, player-portal.
  - **Aceite:** Acesso por role/realm e dupla aprovação em produção.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **ADMIN-02 — Completar UX de releases, pairings e canais com diffs e evidência verificável**
  - **Depende de:** ReleaseManager, contracts, tools/release.
  - **Aceite:** Não publicar release sem atestação e assinatura.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **ADMIN-03 — Completar modelo de deployments e readiness, health stale e host provider**
  - **Depende de:** DeploymentManager, realm-agent, control-api.
  - **Aceite:** Telas mostram observed real, não flag arbitrária.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **ADMIN-04 — Criar telas de gestão de mapas/gs/daemon, com plan→apply auditado**
  - **Depende de:** realm-agent, bridge GNET, operations-worker.
  - **Aceite:** Start/stop/drain seguras com recuperação.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P1 — Robustez e operação

- [ ] **ADMIN-05 — Adicionar configuração revisável e rollback de changesets**
  - **Depende de:** database, control-api, adapter servidor.
  - **Aceite:** Diff/policy/rollback antes de aplicar.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **ADMIN-06 — Acompanhar previews reais, TTL, custo, console de jobs e logs**
  - **Depende de:** operations-worker, provider.
  - **Aceite:** Preview com URL isolada e estados reconciliados.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **ADMIN-07 — Criar dashboards CPU/RSS/tick/latência por mapa e realm**
  - **Depende de:** realm-agent, metrics.
  - **Aceite:** Sem capacidade inventada; health semântica.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **ADMIN-08 — Gerenciar backups WDB, restore drills e políticas de retenção**
  - **Depende de:** server adapter, storage.
  - **Aceite:** Restore ensaiado sem apontar preview para prod.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P2 — Evolução

- [ ] **ADMIN-09 — Adicionar GM/moderação/players/economia via APIs tipadas**
  - **Depende de:** gnet-admin-bridge, ledger.
  - **Aceite:** Ações auditadas e privilégio mínimo.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **ADMIN-10 — Adicionar approvals de rollout canary/staging/produção**
  - **Depende de:** operations-worker, SQL schema/version.
  - **Aceite:** Mesmo digest aprovado; plano para migration WDB.
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
