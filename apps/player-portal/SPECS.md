# SPECS.md — Portal do jogador e identidade web

> **Componente:** `apps/player-portal` · **Status:** implementação inicial (MVP em evolução) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Fonte de verdade do código:** caminhos abaixo. **Referências integradas:** `docs/PAINEL.md`, `docs/PLATFORM-IMPLEMENTATION.md`, `packages/contracts/src/index.ts`, `packages/database/migrations/001_platform.sql`.
> **Regra:** `[ATUAL]` descreve o commit inspecionado; `[ALVO]` descreve contrato/feature ainda não implementados. Não confundir especificação com comportamento testado.

## 1. Missão, problema e decisões de produto

- Centralizar identidade web e operações de autosserviço de contas, sem confundir web_sessions com sessão de gameplay GNET.
- Projetar evolução para contas vinculadas por realm/zone e recuperação, segurança, personagens, gold, histórico e suporte.

**Stack e deploy:** React/Vite + @pw/ui + @pw/api-sdk; credenciais web mantidas pelo control-api/PostgreSQL.

**Situação comprovada [ATUAL]:** Registro/login/logout/perfil web; não há vínculo com gauthd, personagens, guilda, saldo ou pagamento.

**Regra transversal:** jogador escolhe **servidor** e autentica; o control plane fixa **automaticamente** a versão de cliente e os assets homologados daquele servidor. Nenhuma release não comprovada é lançada no navegador.

## 2. Localização e árvore do componente

```text
apps/player-portal/
├── src/App.tsx
├── src/main.tsx
├── vite.config.ts
├── package.json
├── SPECS.md  # contrato deste componente
└── TODO.md   # backlog executável com dependências e aceite
```

Dependências de build e runtime devem ser lidas no `package.json` ou `Cargo.toml` local; `Cargo.toml` na raiz do repo inclui as aplicações Rust.

## 3. Responsabilidades e fronteiras

### 3.1 Domínio exclusivo / ownership

- UI de registro, login/logout e visualização de email/nome/role fornecidos pela API
- fluxos futuros de MFA, recuperação, consentimento, privacidade, vínculo realm-account e self-service
- avisos explícitos quando uma operação só existe na identidade web

### 3.2 O que é expressamente proibido neste componente

- armazenar senha ou token em localStorage; emitir sessão; consultar WDB diretamente
- mutar saldos e inventários sem gateway de domínio/autorização
- permitir compra de gold ou criação de personagem simulada

A UI/admin **nunca** executa scripts arbitrários; dados persistentes do jogo permanecem no WDB/BDB gerenciado pela stack do servidor, enquanto versões, deploys, contas web e jobs pertencem ao PostgreSQL do control plane.

## 4. Interfaces, operações e contratos públicos

1. POST /v1/auth/register {email,password,displayName} -> user; senha >=12 caracteres; web identity apenas.
2. POST /v1/auth/login {email,password} -> cookie HttpOnly pw_session e user; 401 para credenciais inválidas.
3. GET /v1/auth/me -> user ou 401; POST /v1/auth/logout -> revogação da web_session.
4. Futuro: GET /v1/player/realms, POST /v1/player/game-accounts/link, GET /v1/player/characters, GET /v1/player/wallet, POST /v1/player/recovery.

**Norma de evolução de contratos:** mudanças em wire JSON/HTTP seguem `packages/contracts` + migracões compatíveis; mudanças C++ GNET precisam evidência de framing e compatibilidade do port PR #23 e do servidor PR #17. Não criar contratos duplicados por aplicação.

## 5. Modelo operacional, fluxo e máquina de estados

1. ANONYMOUS -> REGISTERED -> AUTHENTICATED_WEB -> GAME_ACCOUNT_UNLINKED ou LINKED -> sessão delegada pelo launcher.
2. Registro atual não cria ID GNET nem usa conta de teste em panel-server153.py.
3. Todo vínculo de personagem requer prova por realm/zone, autenticação forte e autorização do owner; avatar/GM não é dado do usuário normal.

**Princípios:** identificadores estáveis `realm_id` e `deployment_id`; releases pinadas pelo SHA-256; channel pin protegido por `expectedRevision`; session ticket curto e de uso único. Diferenciar `liveness`, `transport_ready` e `gameplay_ready`.

## 6. Mapa de dependências e colaboração

- control-api e @pw/database: fonte exclusiva de users/web_sessions; futuros game_accounts por realm.
- @pw/api-sdk: contrato frontend de auth; @pw/ui: AuthPanel compartilhado com shell/admin-console.
- apps/shell: recebe identidade web compartilhada para launch; apps/realm-agent/gnet-admin-bridge: operações nativas sempre mediadas por API tipada.
- Economia futura: ledger transacional em DB control + addcash sequencial no WDB; nunca debugaddcash.

**Regra anti-duplicação:** interfaces de HTTP, dados, assets, segurança e versionamento pertencem aos packages indicados. Este app deve consumir essas APIs; lógica comum não deve ser colada em várias aplicações.

## 7. Segurança, consistência e tolerância a falhas

- Não expor senha web ao servidor GNET sem fluxo explicitamente modelado; evitar conta previsível.
- Rate limit, proteção brute-force, confirmação email, verificação de sessão, recuperação e MFA são bloqueantes de produção.
- Minimizar PII em logs e operações; exigir consentimento e separar tratamento por realm.
- Não afirmar gold/character sem integração WDB real; RBAC mantém papel player como default.

Falhas devem resultar em código/estado explícito e **fail-closed**: não trocar silenciosamente para outra versão, realm, endpoint, backend, release de produção, dados WDB ou chaves desconhecidas. Log sem PII, password, token ou payload sensível.

## 8. Plano de validação e evidências exigidas

- register/login/logout/me reais com PostgreSQL, 401/403; suites atuais api-smoke.
- CSRF, brute force, força de senha, recuperação, multi-device, token revocation e impersonation (pendentes).
- integração conta web→game account GNET com casos positivos e duplicação/cross-realm negativos (pendente).

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
