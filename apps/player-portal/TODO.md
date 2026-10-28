# TODO.md — Portal do jogador e identidade web

> **Escopo:** `apps/player-portal` · **Fonte de arquitetura:** [SPECS.md](SPECS.md) · **Data base:** 2026-10-28.
> **Convenção:** `[x]` = implementado no código analisado (não significa produção); `[ ]` = pendente. `P0` bloqueia MVP; `P1` operacionalização; `P2` evolução. Não marcar checklist concluída sem evidência de execução.

## Avanço verificado em 2026-10-28

**Escopo:** alterações na branch `feat/multi-realm-platform-shell-mvp` com jobs de TypeScript, PostgreSQL, update-kit, SDK e Rust. `[x]` significa aceite local de workorder específico, não certificação de produção.

- **Parcial** — PLAYER-02: throttle de login/registro PostgreSQL; faltam confirmação email, recuperação, MFA e gestão de contas GNET.

**Bloqueio transversal:** gameplay real depende do cliente Angelica WASM PR #23 e do servidor com `gs`, WDB e login→mundo comprovados; previews de servidor continuam apenas diagnósticos.

## Estado atual de referência

- [x] Estrutura/escopo presente: UI de registro, login/logout e visualização de email/nome/role fornecidos pela API
- [x] Estrutura/escopo presente: fluxos futuros de MFA, recuperação, consentimento, privacidade, vínculo realm-account e self-service
- [x] Estrutura/escopo presente: avisos explícitos quando uma operação só existe na identidade web
- [x] Arquivo SPEC/TODO criado para guiar implementação no diretório deste componente.
- [ ] Funcionalidade plenamente homologada em produção com prova real de gameplay, segurança e restore.

## Backlog rastreável com dependências e critério de aceite

### P0 — Bloqueantes

- [ ] **PLAYER-01 — Implementar endpoint game-account e ponte segura com gauthd real**
  - **Depende de:** control-api, adapter do server153, WDB.
  - **Aceite:** Conta real persiste e entra no jogo sem senha previsível.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **PLAYER-02 — Aplicar anti-brute-force, email verification e recovery segura**
  - **Depende de:** auth/control-api, infraestrutura de email.
  - **Aceite:** Sem brute force nem takeover por token/token reuse.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **PLAYER-03 — Criar modelo game_accounts por realm/zone, autorização de ownership**
  - **Depende de:** database, control-api, server adapter.
  - **Aceite:** Não cruza contas/personagens entre realms.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P1 — Robustez e operação

- [ ] **PLAYER-04 — Listar personagens/slots/atributos via serviço autorizado**
  - **Depende de:** gnet-admin-bridge, control-api.
  - **Aceite:** Dados consistentes do servidor sem leitura direta de storage.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **PLAYER-05 — Adicionar MFA, sessões/dispositivos e revogação global**
  - **Depende de:** control-api, @pw/ui.
  - **Aceite:** MFA exigido para operações de alto risco.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **PLAYER-06 — Oferecer suporte/tickets/privacy export e requests de exclusão**
  - **Depende de:** database, control-api.
  - **Aceite:** Trilha auditada e limites de retenção.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P2 — Evolução

- [ ] **PLAYER-07 — Wallet e extrato de gold via ledger idempotente**
  - **Depende de:** economy futuro, WDB adapter.
  - **Aceite:** Crédito uma vez, conciliação e reversão formal.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **PLAYER-08 — Pagamentos reais com PSP e auditoria**
  - **Depende de:** PLAYER-07, webhooks assinados.
  - **Aceite:** Pagamento liquidado antes de addcash; testes de duplicação.
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
