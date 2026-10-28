# TODO.md — Banco de dados do control plane e migrations

> **Componente:** `packages/database` · **Especificação canônica:** [SPECS.md](SPECS.md) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Legenda:** `[x]` código visto no repositório; `[ ]` requisito pendente. Prioridade P0 = bloqueante; P1 = hardening/integracão; P2 = evolução.

## Avanço verificado em 2026-10-28

**Escopo:** alterações na branch `feat/multi-realm-platform-shell-mvp` com jobs de TypeScript, PostgreSQL, update-kit, SDK e Rust. `[x]` significa aceite local de workorder específico, não certificação de produção.

- **Parcial** — DB-01: migrations por checksum e advisory lock + CI re-apply; falta rollback automatizado e migrações de WDB.
- **Parcial** — DB-02: realm+channel+audit e preview+audit em transações; outras operações ainda não são atômicas.
- **Parcial** — DB-06: operação periódica remove buckets/session/tickets expirados; falta política completa de retenção/auditoria.

**Itens concluídos neste corte:** DB-03, DB-04 (validação pelo pipeline automatizado; ver docs/P0-IMPLEMENTATION-REPORT.md).

**Bloqueio transversal:** gameplay real depende do cliente Angelica WASM PR #23 e do servidor com `gs`, WDB e login→mundo comprovados; previews de servidor continuam apenas diagnósticos.

## Baseline existente

- [x] connectDb(): pg.Pool; DATABASE_URL obrigatório; max connections 10/idle 10s; PG_SSL=require ativa SSL com rejectUnauthorized.
- [x] migrate.ts: lê `migrations/001_platform.sql` e executa `db.query(sql)`; não mantém tabela de versão do schema nem rollback.
- [x] seed-demo.ts: cria/upsert `pwbr-153-lab` com status offline e stable channel; é opcional para desenvolvimento e não provisiona WDB.
- [x] PostgreSQL extension pgcrypto fornece gen_random_uuid(); pg/migrations são runtime Node, não driver Cloudflare Workers homologado.
- [x] Documentação local SPEC + TODO criada, com ownership compartilhado e dependências.
- [ ] Aceite integral do package em ambiente de produção com prova cruzada, sem falhas de segurança.

## Backlog detalhado (cada item é workorder independentemente verificável)

### P0 — Fundação/bloqueantes

- [ ] **DB-01 — Implementar migration tracker, locking e rollback strategy**
  - **Depende de:** control-api, CI.
  - **Aceite verificável:** Migration repeatable com histórico e estado inequívoco.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **DB-02 — Transações atômicas de realm+channel, approvals+audit, job+audit**
  - **Depende de:** control-api.
  - **Aceite verificável:** Falha parcial faz rollback integral.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [x] **DB-03 — Fortalecer operações de idempotência com payload hash e conflitos**
  - **Depende de:** operations-worker.
  - **Aceite verificável:** Mesmo key+payload devolve mesmo job; payload distinto 409.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [x] **DB-04 — Formalizar state machines CHECK e indexes/constraints de uso**
  - **Depende de:** contracts, worker.
  - **Aceite verificável:** Dispatched/dispatching sem valores fora do contrato.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **DB-05 — Escopo por realm e auth granular de operadores/tenants**
  - **Depende de:** admin-console, control-api.
  - **Aceite verificável:** Sem acesso cross-realm não autorizado.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

### P1 — Integração/robustez

- [ ] **DB-06 — Implementar limpeza TTL de sessions/tickets/jobs e retenção de auditoria**
  - **Depende de:** operations-worker.
  - **Aceite verificável:** GC não afeta sessões/releases ativas.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **DB-07 — Adicionar revisions/versionamento de config e rollout CAS**
  - **Depende de:** admin-console, realm-agent.
  - **Aceite verificável:** Diff auditado e conflict detection.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **DB-08 — Adicionar modelos game_accounts/characters mapping/economy ledger**
  - **Depende de:** player-portal, gauthd.
  - **Aceite verificável:** Ownership e idempotência financeira.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **DB-09 — Backup/restore drills + migrações CI com Postgres real**
  - **Depende de:** infra, operations.
  - **Aceite verificável:** RPO/RTO medidos e restore provado.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

### P2 — Evolução

- [ ] **DB-10 — Provar drivers de PostgreSQL em Workers/Hyperdrive se migrar runtime**
  - **Depende de:** control-api, cloudflare.
  - **Aceite verificável:** Paridade de transações e compatibilidade de runtime.
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
