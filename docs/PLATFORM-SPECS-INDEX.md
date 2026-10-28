# PW Web — Índice mestre de SPECS.md e TODO.md por aplicação/package

> **Branch:** `feat/multi-realm-platform-shell-mvp` · **Atualização:** 2026-10-28.
> Este é um **índice de navegação e ordem de execução**, não uma segunda fonte concorrente dos contratos.
> **Fonte de verdade de interface existente:** código do módulo. **Fonte de verdade de arquitetura planejada:** `SPECS.md` daquele módulo e [PAINEL.md](PAINEL.md). **Trabalho pendente:** `TODO.md` daquele módulo.
> As specs distinguem implementado [ATUAL] de proposto [ALVO]. Nenhuma afirma gameplay funcional.

## 1. Todas as sete aplicações

| Aplicação | Spec detalhada | Backlog e aceite | IDs | Contrato/ownership |
| --- | --- | --- | --- | --- |
| `apps/shell` | [SPECS](../apps/shell/SPECS.md) | [TODO](../apps/shell/TODO.md) | `SHELL-*` | Player escolhe realm → login web → pairing estável → update validado → WASM futuro |
| `apps/player-portal` | [SPECS](../apps/player-portal/SPECS.md) | [TODO](../apps/player-portal/TODO.md) | `PLAYER-*` | Identidade web e, depois, vínculo com gauthd/personagens por realm |
| `apps/admin-console` | [SPECS](../apps/admin-console/SPECS.md) | [TODO](../apps/admin-console/TODO.md) | `ADMIN-*` | Operações em realms, releases, deploys, preview e saúde; sem shell arbitrário |
| `apps/control-api` | [SPECS](../apps/control-api/SPECS.md) | [TODO](../apps/control-api/TODO.md) | `API-*` | Autoridade dos dados/pareamentos/tickets/RBAC/health; sem manipular WDB diretamente |
| `apps/operations-worker` | [SPECS](../apps/operations-worker/SPECS.md) | [TODO](../apps/operations-worker/TODO.md) | `OPS-*` | Fila SQL e GitHub dispatch; depois provisioning/promoção/rollback de previews |
| `apps/game-gateway` | [SPECS](../apps/game-gateway/SPECS.md) | [TODO](../apps/game-gateway/TODO.md) | `GATEWAY-*` | Autorização de ticket único, fluxo binário TCP e futuro handshake completo |
| `apps/realm-agent` | [SPECS](../apps/realm-agent/SPECS.md) | [TODO](../apps/realm-agent/TODO.md) | `AGENT-*` | Allowlist systemd, observabilidade, gs/map readiness e operação segura no host |

## 2. Todos os cinco packages compartilhados

| Package | Spec detalhada | Backlog e aceite | IDs | Contrato/ownership |
| --- | --- | --- | --- | --- |
| `packages/contracts` | [SPECS](../packages/contracts/SPECS.md) | [TODO](../packages/contracts/TODO.md) | `CONTRACT-*` | Zod/TypeScript, estados e wire HTTP; futuro OpenAPI/golden cross-language |
| `packages/database` | [SPECS](../packages/database/SPECS.md) | [TODO](../packages/database/TODO.md) | `DB-*` | PostgreSQL; não converte o WDB do servidor para SQL |
| `packages/api-sdk` | [SPECS](../packages/api-sdk/SPECS.md) | [TODO](../packages/api-sdk/TODO.md) | `SDK-*` | Reutilização dos endpoints pelo shell, portal e admin |
| `packages/ui` | [SPECS](../packages/ui/SPECS.md) | [TODO](../packages/ui/TODO.md) | `UI-*` | Layout, AuthPanel, Status e futuro padrão acessível |
| `packages/update-kit` | [SPECS](../packages/update-kit/SPECS.md) | [TODO](../packages/update-kit/TODO.md) | `UPDATE-*` | Assinatura Ed25519, SHA-256, cache por hash; DAG/streaming futuro |

**Contagem:** 12 componentes documentados, 12 arquivos `SPECS.md` e 12 `TODO.md`. Os diretórios `engine/`, `src/server153/`, `src/client153/`, `web/`, `infra/` (futura) e `tools/` são referências/integrações externas ao recorte das sete apps e cinco packages; continuar respeitando os documentos originais e os PRs de cada área.

## 3. Fluxo principal: servidor escolhe cliente e assets automaticamente

```text
                 PW SHELL (não oferece seletor de client version)
                                   │
                         GET /v1/catalog/realms
                                   │
                    seleciona realm + login web
                                   │
                                  ▼
                      CONTROL-API + PostgreSQL
               realms → realm_channels stable → pairings
                                   │
                client_release ───┼── server_release
                          |       │          |
                   asset manifest │   deployment + GNET
                     + SHA/sign   │    readiness proof
                          |       │          |
                          ▼       ▼          ▼
                     UPDATE-KIT → SHELL → GATEWAY WS/TCP
                                   │             │
                                   │             ▼
                                   │        PW server153: glinkd
                                   │             │
                                   ▼             ▼
                       WASM Angelica PR #23     gs + WDB
                        [INTEGRAÇÃO PENDENTE]   [RUNTIME A PROVAR]
```

Se cliente/servidor não têm pairing `supported`, release `published`, deployment `ready` e health recente, a API devolve 409 e o shell não prossegue. Mesmo com update concluído, o botão Jogar está desabilitado até a integração do runtime original.

## 4. Dependência entre os componentes

| Produto | Depende de (interno) | Dependências externas/branch | Estado do bloqueio |
| --- | --- | --- | --- |
| shell | api-sdk, ui, contracts, update-kit, control-api | PR #23 runtime WASM e manifests de assets, gateway GNET | **Não jogável** |
| player-portal | ui, api-sdk, contracts, control-api, database via API | Bridge segura para gauthd/WDB | Conta de jogo ainda indisponível |
| admin-console | ui, api-sdk, contracts, control-api | provider, realm-agent e operações GM | CRUD parcial entregue |
| control-api | contracts, database | gateway, WDB adapter, signing/publisher | Pendente hardening e proof |
| operations-worker | contracts, database, control-api | GitHub Actions, R2, host provider, PRs | Workflow só diagnóstico |
| game-gateway | control-api (HTTP privado) | glinkd + cliente real | Sem prova EnterWorld |
| realm-agent | control-api/operations-worker indiretamente | systemd no host, daemons+gs, scripts server153 | Só units allowlisted |
| contracts | — (Zod) | serialização Rust e manifest PR #23 | Drift a resolver |
| database | PostgreSQL 16, pg | backup, scoping, provider | State machines e transações |
| api-sdk | contracts | API JSON HTTP confiável | Falta OpenAPI gerado |
| ui | api-sdk (AuthPanel atual) | UX/a11y E2E | Acoplamento de UI→rede a reduzir |
| update-kit | contracts, WebCrypto/CacheStorage | signer CI, assets/CDN, WASM/PR #23 | Não suporta DAG completo nem launch |

**Regra central de ownership:** persistência de gerenciamento é control-api/database, protocolo de game é gateway/server adapter, updater/asset cache é update-kit/PR #23, operação de processos é realm-agent, orquestração assíncrona é operations-worker, UX é shell/player/admin; nenhuma camada deve oferecer comando shell arbitrário ao usuário.

## 5. Contratos canônicos e fluxos cross-component

| Contrato/identidade | Owner atual | Consumidores | Garantia e gap |
| --- | --- | --- | --- |
| `CatalogRealm` e `realmStatus` | contracts/control-api | shell/admin | Realms listados sem fingir online |
| `launcherSessionInput` + `LaunchBootstrap` | contracts/control-api | shell, gateway | Release selecionada pelo realm; preflight sem ticket e emissão de ticket após patch por endpoint com CAS; assinatura bootstrap pendente |
| `UpdateManifest` schema 2 + `assetSetDigest` | contracts/update-kit | shell, publisher | Ed25519 + SHA-256, mas falta DAG/streaming v1 do PR #23 |
| `client_releases` + `server_releases` | database/control-api | admin, shell, worker | Registros por digest; falta attestation cripto de publish |
| `pairings` + `realm_channels` revision CAS | database/control-api | admin, shell | Compat profile é pré-filtro, testes reais necessários |
| `game_tickets` (use-once) | database/control-api | gateway | Atomic consumed_at, sem authorization multi-deployment avançado |
| `deployments` e health | database/control-api | admin, shell, agent/probe | Semântica gs/login→mundo; probe autenticada ainda insuficiente |
| `operation_jobs` | database/control-api | operations-worker, admin | Jobs queued/dispatching/dispatched/blocked; alinhar enum contracts |
| `pw-*.service` allowed units | realm-agent | control-api via operations | systemd host real não provisionado nesta feature |
| GNET 1.5.3 v145 | servidor e cliente originais | gateway/adapter | Não definir wire por inferência; testes de cliente real indispensáveis |

## 6. Sequência recomendada de workorders (não confundir com status concluído)

### Bloco A — contratos e segurança de fundação

- `CONTRACT-01/02/03/05`: states, validação de API, bootstrap/ticket e URLs/path seguros.
- `DB-01/02/03/04/05`: migrations, transações, job idempotente, constraints, multi-realm scoping.
- `API-01/02/04/05/06`: módulos, auth hardening, bootstrap compatível, saúde confiável, SQL CAS.
- `SDK-01/02/03/04`, `UI-01/02/03`: consumo tipado e interface sem acoplamento inseguro.

### Bloco B — core funcional do cliente web

- `UPDATE-01/02/03/04/05` → `SHELL-01/02/03/04` → `GATEWAY-01/02/03/04`.
- Dependências concretas: port de Angelica via Emscripten no PR #23, source 1.5.3 servidor via PR #17, data files autorizados, GNET session+entry testes.
- Critério: seletor de servidor, atualização SHA/assinatura, autenticação e **entrada real no mundo**.

### Bloco C — realms administráveis

- `AGENT-01/02/03/04`, `ADMIN-01/02/03/04`, `API-03/05/07/09`.
- Critério: múltiplos realms isolados com WDB próprios, serviço/gs/map health e mudança de versão por pairing homologado.

### Bloco D — previews e operação confiável

- `OPS-01/02/03/04/05`, `ADMIN-05/06/07/08`, `UPDATE-06/07/08`.
- Critério: previews reais de cliente, servidor e par, com assets autorizados, host/gs/WDB isolados, controle de TTL e custo, rollback seguro.

### Bloco E — produção/multiversão futura

- MFA/approvals, pagamentos/ledger, observabilidade, migrations WDB, backup restore, análise de performance, mudanças de provider.
- `1.4.4` e `1.3.6` só entram quando cliente, assets, backend e wire tiverem compatibilidade comprovada; arquitetura suporta famílias futuras, mas não afirma ports concluídos.

## 7. Política obrigatória para agentes implementadores

1. Ler este índice, a `SPECS.md` da área e seu `TODO.md`, então comparar com a branch atual antes de codificar.
2. Ler contracts e packages transitivos e **não replicar lógica** de validação, cache, identidade, autorização ou estado entre apps.
3. Antes de alterar interface pública, documentar alteração no contrato e impacto nos consumidores.
4. Escolher um ID de TODO como workorder, registrar dependências e fixar evidência CI SHA/commit; uma PR pode implementar mais de um ID somente com isolamento explícito.
5. Nenhum `[x]` sem testes reproduzíveis; se o teste exigir servidor/asset indisponível, marcar BLOCKED com motivo.
6. Não mexer no cliente PR #23 ou servidor PR #17 sem base/merge explícito e sem respeitar autoria/branch e direitos de distribuição.
7. Sem exposição de secrets em Git, sem WDB de produção em preview e sem endpoint de execução shell arbitrária.
8. `GITHUB ACTIONS DISPATCHED` não equivale a `PREVIEW READY`; `TCP CONNECTED` não equivale a `GAMEPLAY READY`.
9. Atualizar as SPECS/TODO por componente e este índice quando adicionar/remover package, app ou contrato. Consulte docs/P0-IMPLEMENTATION-REPORT.md para o avanço P0 com testes.
10. Ao concluir, reportar commit/PR, testes realmente executados, achados, bloqueios e próximos IDs.

## 8. Critério de completude da documentação

Para cada componente, o par `SPECS.md` + `TODO.md` deve responder: *quem é o owner? o que produz? quem consome? quais schemas/IDs/HTTP paths/capabilities? quais estados e falhas? onde há credenciais/dados? como testar? o que já existe e o que falta? quais workorders dependem de quais? como validar e reverter?*

**Atenção:** estes arquivos organizam o trabalho futuro; não fazem o gameplay ou a infraestrutura de preview existir. O baseline factual de testes anterior está em `docs/PLATFORM-IMPLEMENTATION.md` e nos workflows GitHub Actions por SHA, com CI dos sete apps e testes criptográficos já executados em commits anteriores.

> **Nota de manutenção (2026-10-28):** status verificado, dependências e itens parciais desta área são acompanhados em [P0-IMPLEMENTATION-REPORT.md](P0-IMPLEMENTATION-REPORT.md). Fluxos de gameplay dependentes do PR #23/gs continuam pendentes.
