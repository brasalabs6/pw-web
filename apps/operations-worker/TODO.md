# TODO.md — Operations worker / orquestrador assíncrono

> **Escopo:** `apps/operations-worker` · **Fonte de arquitetura:** [SPECS.md](SPECS.md) · **Data base:** 2026-10-28.
> **Convenção:** `[x]` = implementado no código analisado (não significa produção); `[ ]` = pendente. `P0` bloqueia MVP; `P1` operacionalização; `P2` evolução. Não marcar checklist concluída sem evidência de execução.

## Estado atual de referência

- [x] Estrutura/escopo presente: polling de operation_jobs queued com SELECT FOR UPDATE SKIP LOCKED e reserva dispatching
- [x] Estrutura/escopo presente: workflow_dispatch com kind/clientCommit/serverCommit/operationId/TTL via GitHub API; marca 'dispatched' após HTTP 204
- [x] Estrutura/escopo presente: erro estruturado em result e estado blocked no momento do dispatch; nunca anuncia READY por isso
- [x] Arquivo SPEC/TODO criado para guiar implementação no diretório deste componente.
- [ ] Funcionalidade plenamente homologada em produção com prova real de gameplay, segurança e restore.

## Backlog rastreável com dependências e critério de aceite

### P0 — Bloqueantes

- [ ] **OPS-01 — Completar lease/attempt e reprocessamento de dispatching preso**
  - **Depende de:** database, contract job.
  - **Aceite:** Crash/restart não perde job nem duplica preview.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **OPS-02 — Reconciliação GitHub run_id/artifact_digest e status de conclusão**
  - **Depende de:** GitHub Actions, control-api.
  - **Aceite:** DISPATCHED→RUNNING→READY/FAILED só com prova.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **OPS-03 — Publisher/client preview confiável com manifest+signature+R2 isolado**
  - **Depende de:** tools/release, PR #23.
  - **Aceite:** URL preview hash-pinned com smoke browser real.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **OPS-04 — Server preview com WDB/gs isolado, data fixtures e cleanup**
  - **Depende de:** realm-agent, server153, infra.
  - **Aceite:** Dados de prod inacessíveis e ready real.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **OPS-05 — Preview pareado client/server por SHAs exatos**
  - **Depende de:** OPS-03, OPS-04, gateway.
  - **Aceite:** Compatibilidade aprovada e evidência gravada.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P1 — Robustez e operação

- [ ] **OPS-06 — Aprovação, rollout CAS, rollback e drain conforme migration WDB**
  - **Depende de:** control-api, database, server adapter.
  - **Aceite:** Transições verificadas, sem perda de dados.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **OPS-07 — TTL/quota/GC seguro com leases, retenção e custo**
  - **Depende de:** provider, asset catalog.
  - **Aceite:** Recurso órfão removido sem apagar hashes compartilhados.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **OPS-08 — Security hardening do publisher e GitHub token permission**
  - **Depende de:** CI, infra.
  - **Aceite:** PR externo não acessa secrets/prod.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P2 — Evolução

- [ ] **OPS-09 — Adicionar event/outbox e fila durável Cloudflare Queues**
  - **Depende de:** @pw/contracts, @pw/database.
  - **Aceite:** Sem dupla execução e com DLQ observável.
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
