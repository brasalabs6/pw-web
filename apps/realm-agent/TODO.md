# TODO.md — Realm Agent / gerenciamento local dos serviços do PW

> **Escopo:** `apps/realm-agent` · **Fonte de arquitetura:** [SPECS.md](SPECS.md) · **Data base:** 2026-10-28.
> **Convenção:** `[x]` = implementado no código analisado (não significa produção); `[ ]` = pendente. `P0` bloqueia MVP; `P1` operacionalização; `P2` evolução. Não marcar checklist concluída sem evidência de execução.

## Avanço verificado em 2026-10-28

**Escopo:** alterações na branch `feat/multi-realm-platform-shell-mvp` com jobs de TypeScript, PostgreSQL, update-kit, SDK e Rust. `[x]` significa aceite local de workorder específico, não certificação de produção.

- **Parcial** — AGENT-03: unit allowlist e bearer autorizados, agora com testes Rust; plan/apply transacional ainda ausente.
- **Parcial** — AGENT-04: validação de unidade/param e comparador auth testados; investigação real de fork/watchdog ainda bloqueada.

**Bloqueio transversal:** gameplay real depende do cliente Angelica WASM PR #23 e do servidor com `gs`, WDB e login→mundo comprovados; previews de servidor continuam apenas diagnósticos.

## Estado atual de referência

- [x] Estrutura/escopo presente: HTTP GET /healthz (liveness, gameplayReady false), GET /v1/services (is-active das unidades), POST /v1/services/:name/:action {reason}
- [x] Estrutura/escopo presente: AGENT_ALLOWED_UNITS explicitamente restrito a pw-*.service; AGENT_LISTEN padrão 127.0.0.1:9010, proíbe bind não-loopback
- [x] Estrutura/escopo presente: Authorization: Bearer AGENT_SHARED_TOKEN >=32 e comparação em tempo constante; logs reason e ação, timeout 6s por systemctl
- [x] Arquivo SPEC/TODO criado para guiar implementação no diretório deste componente.
- [ ] Funcionalidade plenamente homologada em produção com prova real de gameplay, segurança e restore.

## Backlog rastreável com dependências e critério de aceite

### P0 — Bloqueantes

- [ ] **AGENT-01 — Provisionar unidades pw-*.service com scripts reais sem `--fresh`**
  - **Depende de:** server153, infra host.
  - **Aceite:** Startup/stop idempotente e logs por unidade.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **AGENT-02 — Implementar health de gs/mapa e sonda login→mundo**
  - **Depende de:** gateway, server153.
  - **Aceite:** Emite prova real assinada para control-api.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **AGENT-03 — Implementar plan/apply separado e autorizações por realm/zone**
  - **Depende de:** control-api, operations-worker.
  - **Aceite:** Nenhum restart perigoso sem aprovação.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **AGENT-04 — Auditar `gs-ours` watchdog, forks e cleanup para não deixar processos órfãos**
  - **Depende de:** scripts servidor.
  - **Aceite:** Stop/remove all grandchildren sem perda de WDB.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P1 — Robustez e operação

- [ ] **AGENT-05 — Controlar mapas/instâncias com drain/checkpoint GNET bridge**
  - **Depende de:** native adapter, server153.
  - **Aceite:** P0 restore e jogadores não perdem dados.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **AGENT-06 — Config schema/revisions e restart dependency graph**
  - **Depende de:** control-api, database.
  - **Aceite:** Dry-run diff, backup e rollback por config.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **AGENT-07 — Backups/restores WDB e integridade de zone/uniquenamed**
  - **Depende de:** storage provider.
  - **Aceite:** Recovery ensaiado e snapshot consistente.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **AGENT-08 — Exportar CPU/RSS/ticks/FDs/restarts/players por mapa**
  - **Depende de:** observability.
  - **Aceite:** Separar medido de estimado.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P2 — Evolução

- [ ] **AGENT-09 — Suportar múltiplos providers via interface e deploy versionado**
  - **Depende de:** operations-worker, infra.
  - **Aceite:** Provision/deprovision sem acesso root da UI.
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
