# SPECS.md — Realm Agent / gerenciamento local dos serviços do PW

> **Componente:** `apps/realm-agent` · **Status:** implementação inicial (MVP em evolução) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Fonte de verdade do código:** caminhos abaixo. **Referências integradas:** `docs/PAINEL.md`, `docs/PLATFORM-IMPLEMENTATION.md`, `packages/contracts/src/index.ts`, `packages/database/migrations/001_platform.sql`.
> **Regra:** `[ATUAL]` descreve o commit inspecionado; `[ALVO]` descreve contrato/feature ainda não implementados. Não confundir especificação com comportamento testado.

## 1. Missão, problema e decisões de produto

- Executar ações administrativas com autoridade limitada no host real da stack Perfect World, mantendo API pública/console fora do host.
- Ser ponte para supervisão dos daemons e, futuramente, operação tipada do gs e das instâncias, alterações de config revisáveis e provas de readiness.

**Stack e deploy:** Rust 2021, Tokio, Axum, systemd, autenticação bearer e subtle constant-time compare.

**Situação comprovada [ATUAL]:** Compila no CI; lê is-active de unidades allowlisted e executa start/stop/restart via systemctl; sem leitura WDB, hot-map/graceful drain, telemetria ou deploy provider.

**Regra transversal:** jogador escolhe **servidor** e autentica; o control plane fixa **automaticamente** a versão de cliente e os assets homologados daquele servidor. Nenhuma release não comprovada é lançada no navegador.

## 2. Localização e árvore do componente

```text
apps/realm-agent/
├── Cargo.toml
├── src/main.rs
├── SPECS.md  # contrato deste componente
└── TODO.md   # backlog executável com dependências e aceite
```

Dependências de build e runtime devem ser lidas no `package.json` ou `Cargo.toml` local; `Cargo.toml` na raiz do repo inclui as aplicações Rust.

## 3. Responsabilidades e fronteiras

### 3.1 Domínio exclusivo / ownership

- HTTP GET /healthz (liveness, gameplayReady false), GET /v1/services (is-active das unidades), POST /v1/services/:name/:action {reason}
- AGENT_ALLOWED_UNITS explicitamente restrito a pw-*.service; AGENT_LISTEN padrão 127.0.0.1:9010, proíbe bind não-loopback
- Authorization: Bearer AGENT_SHARED_TOKEN >=32 e comparação em tempo constante; logs reason e ação, timeout 6s por systemctl

### 3.2 O que é expressamente proibido neste componente

- aceitar comandos arbitrários Bash/SSH, manipular banco WDB com SQL ou shell
- controlar servidores fora da allowlist, disponibilizar endpoints públicos ou ler secrets de outro realm
- declarar gameplay READY sem teste gs+login+world; assumir restart systemd = drain safe

A UI/admin **nunca** executa scripts arbitrários; dados persistentes do jogo permanecem no WDB/BDB gerenciado pela stack do servidor, enquanto versões, deploys, contas web e jobs pertencem ao PostgreSQL do control plane.

## 4. Interfaces, operações e contratos públicos

1. GET /healthz {service:'pw-realm-agent',status:'alive',gameplayReady:false}; público apenas local.
2. GET /v1/services com token -> array {name,systemd_status}; permissão interna por allowlist.
3. POST /v1/services/{unit}/{start|stop|restart} {reason:min 10 chars}; resultado accepted indica apenas systemctl exit successful.
4. Env AGENT_SHARED_TOKEN, AGENT_ALLOWED_UNITS, AGENT_LISTEN, SYSTEMCTL_PATH. Nenhum endpoint para alterar env arbitrária.
5. Futuro: /v1/maps/{map}/plan|apply, /v1/config-revisions, /v1/checkpoints, /v1/backups, /v1/health/gameplay.

**Norma de evolução de contratos:** mudanças em wire JSON/HTTP seguem `packages/contracts` + migracões compatíveis; mudanças C++ GNET precisam evidência de framing e compatibilidade do port PR #23 e do servidor PR #17. Não criar contratos duplicados por aplicação.

## 5. Modelo operacional, fluxo e máquina de estados

1. CONFIG_BOOT -> validate token/allowlist/loopback -> LISTEN -> AUTH -> validate service/action/reason -> systemctl -> return observed command result.
2. Governação futura: OPERATION_REQUESTED -> PLAN -> APPROVAL -> DRAIN -> CHECKPOINT -> APPLY -> READINESS_PROBE -> COMPLETE/ROLLBACK.
3. Integrar watchdog/server153 sem process name mismatch gs-ours/gs; detectar netos/orphans/instâncias antes de stop.

**Princípios:** identificadores estáveis `realm_id` e `deployment_id`; releases pinadas pelo SHA-256; channel pin protegido por `expectedRevision`; session ticket curto e de uso único. Diferenciar `liveness`, `transport_ready` e `gameplay_ready`.

## 6. Mapa de dependências e colaboração

- control-api e operations-worker solicitam operations tipadas via rede privada; admin-console só vê resultados via API.
- src/server153 e scripts/gs-server153.sh, boot-server153.sh e watchdog-server153.sh são sistema sob gestão; não os reescreve automaticamente.
- apps/game-gateway permanece separado do agente e não obtém root; tools/metrics futuro exporta observabilidade por daemon e mapa.

**Regra anti-duplicação:** interfaces de HTTP, dados, assets, segurança e versionamento pertencem aos packages indicados. Este app deve consumir essas APIs; lógica comum não deve ser colada em várias aplicações.

## 7. Segurança, consistência e tolerância a falhas

- Executar como usuário não-root com sudoers systemd mínimos; no current root/public port policy.
- Secrets não são impressos; rotacionar token e caminhar para mTLS, identity por realm e replay protection.
- Não usar boot-server153.sh --fresh em produção (apaga WDB); backup/checkpoint antes de alterações e sem enviar WDB ao control plane.
- Não operar mapas legados por kill imediato; gs/instance precisam drenagem compatível e prova de arquivo de configuração.
- Systemd unit names são fontes controladas do host; não aceitar path traversal/command injection em parâmetros.

Falhas devem resultar em código/estado explícito e **fail-closed**: não trocar silenciosamente para outra versão, realm, endpoint, backend, release de produção, dados WDB ou chaves desconhecidas. Log sem PII, password, token ou payload sensível.

## 8. Plano de validação e evidências exigidas

- CI cargo test workspace inclui testes unitários de auth bearer e nome de unidade; ainda faltam systemd sandbox, timeout, stderr, sudoers e drain/gs real.
- Executar dry-run de unidade allowlisted e confirmar que unidade não allowlisted recebe 404.
- Em ambiente real: provar ordenação dos oito daemons + gs, map readiness, save/checkpoint, graceful stop e restore.
- Crash simulation/power loss não deve corromper WDB; health stale sinaliza degraded.

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

> **Nota de manutenção (2026-10-28):** status verificado, dependências e itens parciais desta área são acompanhados em [P0-IMPLEMENTATION-REPORT.md](../../docs/P0-IMPLEMENTATION-REPORT.md). Fluxos de gameplay dependentes do PR #23/gs continuam pendentes.
