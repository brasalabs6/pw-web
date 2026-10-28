# SPECS.md — PW Shell / launcher do jogador

> **Componente:** `apps/shell` · **Status:** implementação inicial (MVP em evolução) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Fonte de verdade do código:** caminhos abaixo. **Referências integradas:** `docs/PAINEL.md`, `docs/PLATFORM-IMPLEMENTATION.md`, `packages/contracts/src/index.ts`, `packages/database/migrations/001_platform.sql`.
> **Regra:** `[ATUAL]` descreve o commit inspecionado; `[ALVO]` descreve contrato/feature ainda não implementados. Não confundir especificação com comportamento testado.

## 1. Missão, problema e decisões de produto

- Oferecer a única porta de entrada pública ao jogo web: descoberta de realms, identidade, obtenção de bootstrap, atualização verificável, carregamento futuro do cliente e recuperação de falhas.
- Nenhuma escolha manual de versão: o pairing stable do realm define unicamente clientRelease, serverRelease, protocolProfile e dataProfile. Testers podem receber canal restrito pela API, nunca fornecer URL binária arbitrária.

**Stack e deploy:** React 19, Vite 7, TypeScript, fetch com cookies HttpOnly, CacheStorage e WebCrypto nativo.

**Situação comprovada [ATUAL]:** Interface e seleção de realm funcionam em código; jogo 1.5.3 ainda não é iniciado, sem assets originais e sem runtime WASM integrado.

**Regra transversal:** jogador escolhe **servidor** e autentica; o control plane fixa **automaticamente** a versão de cliente e os assets homologados daquele servidor. Nenhuma release não comprovada é lançada no navegador.

## 2. Localização e árvore do componente

```text
apps/shell/
├── src/App.tsx
├── src/main.tsx
├── vite.config.ts
├── index.html
├── package.json
├── SPECS.md  # contrato deste componente
└── TODO.md   # backlog executável com dependências e aceite
```

Dependências de build e runtime devem ser lidas no `package.json` ou `Cargo.toml` local; `Cargo.toml` na raiz do repo inclui as aplicações Rust.

## 3. Responsabilidades e fronteiras

### 3.1 Domínio exclusivo / ownership

- estado efêmero da UI: lista de realms, selected realm, usuário web, progresso, mensagens de erro
- coordenação do update-kit e do futuro adaptador WASM, sessão de jogo e transição mundo/launcher
- semântica de downloads na UI, cache visual e suporte futuro a PWA/mobile

### 3.2 O que é expressamente proibido neste componente

- geração de releases/assinaturas, RBAC, credenciais GNET, banco WDB, gateway TCP ou seleção de build manual
- definição de par compatível (única fonte control-api/registry)
- conteúdo de gameplay original (PR #23) e criação de contas de jogo

A UI/admin **nunca** executa scripts arbitrários; dados persistentes do jogo permanecem no WDB/BDB gerenciado pela stack do servidor, enquanto versões, deploys, contas web e jobs pertencem ao PostgreSQL do control plane.

## 4. Interfaces, operações e contratos públicos

1. GET /v1/catalog/realms -> {realms: CatalogRealm[]}; falha de rede exibe erro e não inventa servidor.
2. GET /v1/auth/me -> usuário ou 401; POST /v1/auth/login/register/logout -> cookie pw_session; login é web, não GNET.
3. POST /v1/launcher/releases {realmId,channel:'stable'} -> release sem ticket para patch. Futuro start: POST /v1/launcher/sessions {realmId,expectedPairingId,expectedClientDigest} -> bootstrap com ticket somente após atualização; 409 se ocorrer troca.
4. prepareClient({manifestUrl,expectedDigest,trustedKeys,onProgress,signal}) -> manifesto Ed25519 verificado e bytes JS/WASM; files de entrada armazenados com chave SHA-256.
5. Interface prevista ClientAdapter.start({manifest/assetset,realmId,zone,gatewayUrl,ticket}) -> runtime handle com stop/dispose; não implementada.

**Norma de evolução de contratos:** mudanças em wire JSON/HTTP seguem `packages/contracts` + migracões compatíveis; mudanças C++ GNET precisam evidência de framing e compatibilidade do port PR #23 e do servidor PR #17. Não criar contratos duplicados por aplicação.

## 5. Modelo operacional, fluxo e máquina de estados

1. BOOT -> GET catalog -> SELECT_REALM -> AUTH_REQUIRED ou AUTHENTICATED -> USER_REQUEST_LAUNCH -> RESOLVE_PAIRING -> UPDATING -> VERIFIED -> GAME_START_PENDING.
2. A implementação atual retorna READY (arquivos preparados), mas exibe botão de gameplay desabilitado; não fingir IN_GAME.
3. [ATUAL] O shell prepara arquivos por preflight sem emitir ticket. [PENDENTE] A chamada final a /launcher/sessions ocorrerá ao integrar o entry ABI do cliente; ela revalida o pairing e emite um ticket novo na fronteira do WSS.
4. No switch de realm, desmontar JS/WASM, VFS, WebGL/audio e sockets; nenhum cache por filename sem hash, e não carregar release mais recente silenciosamente.

**Princípios:** identificadores estáveis `realm_id` e `deployment_id`; releases pinadas pelo SHA-256; channel pin protegido por `expectedRevision`; session ticket curto e de uso único. Diferenciar `liveness`, `transport_ready` e `gameplay_ready`.

## 6. Mapa de dependências e colaboração

- @pw/api-sdk: único cliente HTTP do control-api; compartilha sessão cookie com player-portal/admin-console por ambiente/origem corretamente isolados.
- @pw/contracts: CatalogRealm/LaunchBootstrap; schema exato da versão escolhida pelo servidor.
- @pw/update-kit: integridade de manifest, assinatura, SHA de objetos, cache; nada de confiar em ETag.
- @pw/ui: Layout, AuthPanel, Status e identidade visual.
- apps/control-api: autoridade de realms/pairings/releases, emite ticket; apps/game-gateway: após bootstrap validado, sessão WSS/TCP GNET.
- src/client153/PR #23: futura ABI Emscripten/Angelica; tools/assets/PR #23: DAG/streaming de assets, sem duplicar manifests v1.

**Regra anti-duplicação:** interfaces de HTTP, dados, assets, segurança e versionamento pertencem aos packages indicados. Este app deve consumir essas APIs; lógica comum não deve ser colada em várias aplicações.

## 7. Segurança, consistência e tolerância a falhas

- Não instanciar runtime sem prova Ed25519 e hash do manifesto/bytes. Chaves públicas confiáveis devem vir de build protegido e não de resposta arbitrária da API.
- Não expor segredos do servidor, credenciais permanentes ou arquivos originais sem direito; permitir origem local HTTP somente no desenvolvimento.
- Não anunciar online com flag de banco; backend verifica daemon+gs+login ao mundo e freshness do deployment.
- Não iniciar download do mundo inteiro; usar entry bundles, budget de memória, streaming por mapas e leases LRU na integração PR #23.
- Tratar cookie/CSRF e múltiplas origens; configuração futura do app em produção deve usar URLs HTTPS/WSS e CSP restritiva.

Falhas devem resultar em código/estado explícito e **fail-closed**: não trocar silenciosamente para outra versão, realm, endpoint, backend, release de produção, dados WDB ou chaves desconhecidas. Log sem PII, password, token ou payload sensível.

## 8. Plano de validação e evidências exigidas

- teste de catálogo vazio, manutenção, realm offline e falha HTTP; seleciona o realm sem dropdown de versão.
- teste de login, 401, logout, falha de compatibilidade 409, estado de loading e progresso; abort/duplo clique.
- teste Ed25519 positivo/negativo, hash divergente e cache corrupto; replay/downgrade/URL maliciosa.
- teste E2E Chrome desktop/mobile com WASM real, cold/warm cache, switch de realm, loss of context e quota negada (a criar).

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
