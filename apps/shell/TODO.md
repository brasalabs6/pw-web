# TODO.md — PW Shell / launcher do jogador

> **Escopo:** `apps/shell` · **Fonte de arquitetura:** [SPECS.md](SPECS.md) · **Data base:** 2026-10-28.
> **Convenção:** `[x]` = implementado no código analisado (não significa produção); `[ ]` = pendente. `P0` bloqueia MVP; `P1` operacionalização; `P2` evolução. Não marcar checklist concluída sem evidência de execução.

## Avanço verificado em 2026-10-28

**Escopo:** alterações na branch `feat/multi-realm-platform-shell-mvp` com jobs de TypeScript, PostgreSQL, update-kit, SDK e Rust. `[x]` significa aceite local de workorder específico, não certificação de produção.

- **Parcial** — SHELL-01: seleção só do realm, versão do cliente automática via pairing; falta cenário E2E com 2 clientes reais.
- **Parcial** — SHELL-02: faz preflight e update sem criar ticket; endpoint de sessão posterior exige digest/pairing; jogo ainda não inicia.
- **Parcial** — SHELL-06: abort de requisição/download no switch realm; falta quota, OPFS, leases e retries.

**Bloqueio transversal:** gameplay real depende do cliente Angelica WASM PR #23 e do servidor com `gs`, WDB e login→mundo comprovados; previews de servidor continuam apenas diagnósticos.

## Estado atual de referência

- [x] Estrutura/escopo presente: estado efêmero da UI: lista de realms, selected realm, usuário web, progresso, mensagens de erro
- [x] Estrutura/escopo presente: coordenação do update-kit e do futuro adaptador WASM, sessão de jogo e transição mundo/launcher
- [x] Estrutura/escopo presente: semântica de downloads na UI, cache visual e suporte futuro a PWA/mobile
- [x] Arquivo SPEC/TODO criado para guiar implementação no diretório deste componente.
- [ ] Funcionalidade plenamente homologada em produção com prova real de gameplay, segurança e restore.

## Backlog rastreável com dependências e critério de aceite

### P0 — Bloqueantes

- [ ] **SHELL-01 — Resolver bootstrap apenas por realm/channel stable sem qualquer seleção manual de versão**
  - **Depende de:** @pw/api-sdk, control-api, pairings.
  - **Aceite:** Reprodução E2E de dois realms com versões distintas; sem escolha de cliente.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **SHELL-02 — Separar obtenção de manifest/update de emissão do ticket de 90 s**
  - **Depende de:** control-api, update-kit.
  - **Aceite:** Download lento não consome nem expira ticket; launch revalida pairing.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **SHELL-03 — Integrar ABI Emscripten/Angelica do PR #23 ao runtime com teardown**
  - **Depende de:** src/client153, gateway.
  - **Aceite:** Jogo conecta e executa entrada real no mundo; failures não marcam READY.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **SHELL-04 — Integrar loader DAG/entryBundles e asset profile por versão**
  - **Depende de:** update-kit, PR #23 content-manifest.
  - **Aceite:** Só assets necessários à entrada são baixados e verificados.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P1 — Robustez e operação

- [ ] **SHELL-05 — Política de threads/WebGL/áudio/memória/capabilities por dispositivo**
  - **Depende de:** @pw/contracts, client runtime.
  - **Aceite:** Desktop e mobile passam smoke incluindo memória limitada.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **SHELL-06 — Suportar cancel/retry/progress, quota/OPFS fallback e cache leases**
  - **Depende de:** update-kit.
  - **Aceite:** Cancelamento não deixa dados parciais e duas abas não corrompem cache.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **SHELL-07 — Adicionar sessão WSS e tratamento de desconexão/maintenance**
  - **Depende de:** gateway, game adapter.
  - **Aceite:** Ticket consumido uma vez e erros mostrados sem fallback.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

- [ ] **SHELL-08 — Implementar observabilidade de carregamento, crashes e atualização**
  - **Depende de:** observability futuro, control-api.
  - **Aceite:** Métricas agregadas sem dados pessoais/segredos.
  - **Evidência esperada:** testes automatizados, commit, link de run CI, logs sanitizados e resultado reproduzível.
  - **Estado:** pendente (não inferir de outra branch).

### P2 — Evolução

- [ ] **SHELL-09 — Acessibilidade, dispositivos de toque, localizações e status por mapa**
  - **Depende de:** @pw/ui, realm-agent.
  - **Aceite:** Teclado/leitores e touch suportados, status observável.
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
