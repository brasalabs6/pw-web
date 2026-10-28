# SPECS.md — Gateway de sessão WSS → GNET/TCP

> **Componente:** `apps/game-gateway` · **Status:** implementação inicial (MVP em evolução) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Fonte de verdade do código:** caminhos abaixo. **Referências integradas:** `docs/PAINEL.md`, `docs/PLATFORM-IMPLEMENTATION.md`, `packages/contracts/src/index.ts`, `packages/database/migrations/001_platform.sql`.
> **Regra:** `[ATUAL]` descreve o commit inspecionado; `[ALVO]` descreve contrato/feature ainda não implementados. Não confundir especificação com comportamento testado.

## 1. Missão, problema e decisões de produto

- Expor uma fronteira de transporte para cliente browser sem deixar o browser acessar diretamente TCP 29000 nem permitir destino escolhido pelo jogador.
- Validar ticket de sessão por deployment e identidade da control-api; após autorização, transferir bytes GNET intactos, com terminação cooperativa.

**Stack e deploy:** Rust 2021, Tokio, Axum WS, futures-util, reqwest rustls, tracing.

**Situação comprovada [ATUAL]:** Compila no CI, roteia bytes binários via TCP upstream fixado e valida ticket; não faz tradução GNET, multiplex, provas de gameplay ou TLS/WSS público.

**Regra transversal:** jogador escolhe **servidor** e autentica; o control plane fixa **automaticamente** a versão de cliente e os assets homologados daquele servidor. Nenhuma release não comprovada é lançada no navegador.

## 2. Localização e árvore do componente

```text
apps/game-gateway/
├── Cargo.toml
├── src/main.rs
├── SPECS.md  # contrato deste componente
└── TODO.md   # backlog executável com dependências e aceite
```

Dependências de build e runtime devem ser lidas no `package.json` ou `Cargo.toml` local; `Cargo.toml` na raiz do repo inclui as aplicações Rust.

## 3. Responsabilidades e fronteiras

### 3.1 Domínio exclusivo / ownership

- GET /healthz processo vivo e GET /readyz tentativa TCP com gameplayReady:false, GET /connect WebSocket
- primeira mensagem WS JSON {ticket}, timeout 8 s, token base64url de 43 caracteres exatamente, consumo via POST /v1/internal/tickets/consume com bearer token
- TCP upstream por env GNET_UPSTREAM, bidirectional WS Binary↔TCP bytes, fechamento com select; logs de transport sem token

### 3.2 O que é expressamente proibido neste componente

- alterar framing/packet IDs GNET, autenticar usuário web diretamente, fazer hashing de senha, manipular personagens
- escolher ou abrir TCP por URL no handshake, resolver server/client version arbitrariamente
- declarar realm jogável só porque glinkd aceita socket

A UI/admin **nunca** executa scripts arbitrários; dados persistentes do jogo permanecem no WDB/BDB gerenciado pela stack do servidor, enquanto versões, deploys, contas web e jobs pertencem ao PostgreSQL do control plane.

## 4. Interfaces, operações e contratos públicos

1. WS GET /connect: primeiro frame textual JSON {ticket}, depois frames binários; mensagem textual posterior encerra.
2. HTTP GET /healthz -> {service,status}; GET /readyz -> transport reachable/unreachable, gameplayReady:false sempre.
3. POST control-api /v1/internal/tickets/consume com {ticket,deploymentId} e header Authorization Bearer GATEWAY_CONTROL_TOKEN; token consumido atomicamente.
4. Env GATEWAY_LISTEN default 127.0.0.1:9001, GNET_UPSTREAM default 127.0.0.1:29000, GATEWAY_DEPLOYMENT_ID, GATEWAY_CONTROL_TOKEN, CONTROL_API_URL.
5. Futuro: token binding a pairing/realm/zone e upstream route, session expiry/revocation, heartbeat, GNET conformance/flow metrics.

**Norma de evolução de contratos:** mudanças em wire JSON/HTTP seguem `packages/contracts` + migracões compatíveis; mudanças C++ GNET precisam evidência de framing e compatibilidade do port PR #23 e do servidor PR #17. Não criar contratos duplicados por aplicação.

## 5. Modelo operacional, fluxo e máquina de estados

1. WS_UPGRADE -> FIRST_FRAME_WAIT(8s) -> VALIDATE_TICKET -> TCP_CONNECT(3s) -> STREAM_BINARY -> CLOSE.
2. Auth failure ou TCP down fecha socket; WS text após login não vira comando GNET.
3. Usa tokio::select! em duas direções; falha de uma fecha a sessão; não garante backpressure per connection além do WS/TCP atual.

**Princípios:** identificadores estáveis `realm_id` e `deployment_id`; releases pinadas pelo SHA-256; channel pin protegido por `expectedRevision`; session ticket curto e de uso único. Diferenciar `liveness`, `transport_ready` e `gameplay_ready`.

## 6. Mapa de dependências e colaboração

- apps/shell futuro cria WS com ticket e executa handshake original; control-api autoriza ticket/deployment.
- server153 glinkd recebe stream TCP; realm-agent/probe informa se gs/daemons estão READY.
- @pw/contracts futuramente define contrato de ticket sem traduzir wire; operations-worker instala gateway por deployment.

**Regra anti-duplicação:** interfaces de HTTP, dados, assets, segurança e versionamento pertencem aos packages indicados. Este app deve consumir essas APIs; lógica comum não deve ser colada em várias aplicações.

## 7. Segurança, consistência e tolerância a falhas

- TLS obrigatório na borda externa; ligação interna API e TCP pela rede privada/loopback. Publicar 9001 sem TLS não é seguro.
- Aplicar limite de sockets/usuário/IP, byte rate, idle timeout, close codes, fair queue e quotas antes de produção.
- Não logar ticket, senha ou bytes de gameplay; admin bearer/GATEWAY_CONTROL_TOKEN deve ter rotação e escopo por deployment.
- O cliente 1.5.3 inclui handshake GNET complexo; transporte correto precisa evidência byte a byte e latência real.
- Readiness HTTP/TCP não equivale a login/CreateRole/EnterWorld; fluxo do jogo de produção depende de probe externo real.

Falhas devem resultar em código/estado explícito e **fail-closed**: não trocar silenciosamente para outra versão, realm, endpoint, backend, release de produção, dados WDB ou chaves desconhecidas. Log sem PII, password, token ou payload sensível.

## 8. Plano de validação e evidências exigidas

- CI cargo test workspace passou, incluindo unit tests de ticket/UUID. Ainda não há suite de frames reais/duas sessões demonstrada.
- Criar WS unit/integration com upstream fake TCP, mensagens truncadas, binary chunks, auth replay, concorrência e stop.
- Rodar cliente real PR #23 com glinkd/gs PR #17; valida login, roles, mapa, movimentação, reconexão desktop/mobile.
- Testes de carga ping p50/p95/p99, memória por sessão, floods, backpressure e desconexão.

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
