# TODO.md — Gateway de sessão WSS → GNET/TCP

> **Escopo:** `apps/game-gateway` · **Fonte de arquitetura:** [SPECS.md](SPECS.md) · **Data base:** 2026-10-28.
> **Convenção:** `[x]` = implementado no código analisado (não significa produção); `[ ]` = pendente. `P0` bloqueia MVP; `P1` operacionalização; `P2` evolução. Não marcar checklist concluída sem evidência de execução.

## Estado atual de referência

- [x] Estrutura/escopo presente: GET /healthz processo vivo e GET /readyz tentativa TCP com gameplayReady:false, GET /connect WebSocket
- [x] Estrutura/escopo presente: primeira mensagem WS JSON {ticket}, timeout 8 s, comprimento 32..256, consumo via POST /v1/internal/tickets/consume com bearer token
- [x] Estrutura/escopo presente: TCP upstream por env GNET_UPSTREAM, bidirectional WS Binary↔TCP bytes, fechamento com select; logs de transport sem token
- [x] Arquivo SPEC/TODO criado para guiar implementação no diretório deste componente.
- [ ] Funcionalidade plenamente homologada em produção com prova real de gameplay, segurança e restore.

## Backlog rastreável com dependências e critério de aceite

### P0 — Bloqueantes

- [ ] **GATEWAY-01 — Integrar client browser ABI e protocolo GNET 1.5.3 do PR #23**
  - **Depende de:** shell, src/client153, server153.
  - **Aceite:** EnterWorld real comprovado em sessão browser.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **GATEWAY-02 — Gateway route validation por deployment/realm/zone e assinatura bootstrap**
  - **Depende de:** control-api.
  - **Aceite:** Ticket nunca permite outro TCP deployment.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **GATEWAY-03 — Ingress WSS/TLS e service auth privados com rotação**
  - **Depende de:** infra, control-api.
  - **Aceite:** No public cleartext, healthy sessions.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **GATEWAY-04 — Testes de frame, stream, close, replay e backpressure**
  - **Depende de:** CI Rust e fake GNET.
  - **Aceite:** Probes cobrindo DoS e fechamento.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P1 — Robustez e operação

- [ ] **GATEWAY-05 — Quota por IP/usuário/realm, keepalive e idle/disconnect**
  - **Depende de:** control-api, proxy.
  - **Aceite:** Recursos limitados sob conexões adversariais.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **GATEWAY-06 — Medir latência/throughput/CPU por player e telemetria sem PII**
  - **Depende de:** metrics.
  - **Aceite:** p50/p95 e baseline 1.5.3 medidos.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **GATEWAY-07 — Health semântico GNET e failover com reconexão segura**
  - **Depende de:** realm-agent, probe.
  - **Aceite:** Transporte sem declarativo falso de gameplay.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P2 — Evolução

- [ ] **GATEWAY-08 — Testar WebTransport/QUIC como caminho futuro sem mudar MVP**
  - **Depende de:** client adapter, network.
  - **Aceite:** ADR + benchmark e fallback WSS.
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
