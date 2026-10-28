# Execução P0/P1 — rodada de implementação de 2026-10-28

> **Branch:** `feat/multi-realm-platform-shell-mvp` (derivada de `feat/pw-source-1.5.3-ci-build`).
> **Tipo de evidência:** commits de código + testes automatizados GitHub Actions. Não houve publicação em produção, gameplay real ou execução de server `gs` nesta rodada.
> **Definição de status:** concluído = requisito isolado com teste e integração observados; parcial = funcionalidade avançou mas falta item de aceite; bloqueado = depende de port/runtime/infra externa.

## Prioridade decidida

**Prioridade P0.1:** impedir que o shell baixe/execute a versão errada ou misture realm/release/cookie/ticket; a segurança dos contratos e do banco vem antes de mais UI.
**P0.2:** garantir autoupdate seguro e transições com testes completos de cliente/servidor.
**P0.3:** conectar WASM Angelica 1.5.3, glinkd/gauthd, gs e WDB para entrada REAL no mundo.
**P0.4:** operar realms/mapas/health e preview isolado, então performance/capacidade.

Não otimizar FPS de combate antes de comprovar gameplay básico; trabalho de port segue separado no PR #23.

## Realizado nesta rodada

| Área | Mudança implementada | Evidência / contrato |
| --- | --- | --- |
| SQL/migrations | tracking SHA de migrations + advisory lock, migrations 002-004, constraints de estados/releases, FK game_tickets ON DELETE CASCADE | npm run db:migrate repetido e teste HTTP PostgreSQL |
| Operações duráveis | criação realm+channel+audit e preview+audit atômicas; idempotency_key rejeita payload/ator divergentes | testes do control-api e DB |
| Identidade | rate-limit no PostgreSQL por identidade para login/registro; logout revoga tickets pendentes | smoke com 9 tentativas e revogação pós-logout |
| Contratos | operationStatus inclui dispatching; tipo/preflight/LaunchBootstrap schemas Zod; canal stable obrigatório | TypeScript + SDK runtime |
| Lançamento | `POST /v1/launcher/releases` faz preflight sem ticket; `POST /v1/launcher/sessions` exige pairingId e clientDigest esperados | teste de 409 em digest divergente |
| Atualização | manifesto canônico assinado Ed25519, URL de assets mesma origem HTTPS, validação de path e limite em leitura streaming | test:updater: assinatura, adulteração, warm cache, duplicate key |
| Frontend Shell | auto-resolução por realm, cancelamento de update no switch e ignorar resposta obsoleta | builds/web source test + SDK |
| Health de deployment | assinatura Ed25519 do relatório de probe com ID + timestamp + evidência + token, aceite de até 20 s, auditoria | smoke sintético rejeita assinatura falsa, aceita válida |
| Gateway | ticket base64url restrito, UUID deployment, consumo one-shot no banco com corrida de 2 consumidores | Rust unit tests + HTTP smoke |
| Realm Agent | allowlist de unidades `pw-*.service` e bearer autenticado com testes unitários | cargo test --workspace |
| Worker | tentativa, lease, crash→blocked manual sem duplicar dispatch; GC periódico de buckets/tickets/sessões expirados | código + build; reconciliação GitHub não implementada |
| API SDK | runtime parser de catálogo/usuário/release/bootstrap, timeout, aborto, fail-closed, CORS confiável | test:sdk e smoke preflight |
| CI | testes novos em update-kit, SDK, agent e gateway, API SQL/health/corrida e reapply migrations | workflow platform-ci.yml |

### Itens marcados integralmente [x] nos TODOs

- `CONTRACT-01`: estado dispatching agora está no contrato.
- `DB-03`: idempotência é consistente para mesmo ator/payload e conflitante para requisição divergente.
- `DB-04`: banco restringe state machines e cria índices úteis.

O restante dos itens permanece aberto no `TODO.md` do componente, com anotações **Parcial** e blockers, mesmo quando recebeu código. Isso evita alegar conclusão de partes que dependem de WDB/gs ou port WASM.

## Vulnerabilidades/bugs descobertos e corrigidos por teste

1. Logout 500 quando existia game_ticket vinculado ao web_session: FK agora faz CASCADE e revoga tickets.
2. Worker após crash podia deixar job em dispatching para sempre: reserva com deadline e estado blocked para reconciliação manual.
3. Manifest com chaves JSON repetidas podia ser interpretado ambiguamente: publicação e leitura canônicas.
4. Download de binário grande acima do tamanho declarado podia esgotar memória antes de verificação: leitura streaming limitando tamanho.
5. Seleção rápida de outro realm durante update podia mostrar release antiga como READY: cancelamento e geração do request.
6. Mesmo Idempotency-Key com payload diferente poderia retornar job não equivalente: agora HTTP 409.
7. Gateway aceitava ticket com formato arbitrário: exige token base64url opaco, consumo one-shot.
8. Health do servidor podia depender apenas de bearer+booleans: agora também exige prova Ed25519 temporal por deployment.
9. Ticket era emitido antes de baixar WASM, expirando em downloads lentos: preflight/resolve separado do endpoint de sessão final.

## Testes executáveis

```bash
npm run typecheck
npm run build:web
npm run test:platform
npm run test:updater
npm run test:sdk
npm run db:migrate
npm run db:migrate  # deve registrar SKIP nos scripts existentes
# com DATABASE_URL e apps/control-api rodando:
node tests/platform/api-smoke.mjs
cargo test --workspace
```

**Referência de CI bem-sucedida já observada:** [GitHub Actions 40713745869](https://github.com/brasalabs6/pw-web/actions/runs/40713745869) (TypeScript, builds, PostgreSQL/API smoke, SDK/update-kit, Rust). Commits posteriores requerem sua própria execução antes de alegar PASS.

## Dependências externas que ainda bloqueiam o produto

- PR #23: runtime e entry ABI do cliente C++→Emscripten/WASM, VFS, DAG de assets, GL, animação/rede e gameplay.
- PR #17: servidores GNET + `gs` em host real, WDB persistente, credenciais e fixture autorizada, segurança e smoke login→world.
- Publisher/cloud: chaves de produção, assinatura vinculada à prova CI real (não apenas URL), assets CDN/R2 e run de preview verdadeiro.
- Sonda real de gameplay: o endpoint valida assinatura de prova, mas o **processo de probe não está implementado**; o teste CI usa dados **sintéticos**, nunca deve ser exibido como prova de um realm real.
- Hardening: email verification/recovery, MFA, CSRF completo, quotas por IP/usuário, auditoria de GM/economia, scopes por realm, backups/restore/rollbacks.

## Próxima seleção de workorders

1. `API-03` + `CONTRACT-03`: atrelar release e pairing a artefato+prova CI assinados, assinar bootstrap completo.
2. `UPDATE-01` + `SHELL-03` + `GATEWAY-01`: ABI de entrada Angelica/WASM, loader DAG, protocolo login→role→world.
3. `AGENT-01/02`: unidades/health da stack real com `gs` e WDB seguro.
4. `OPS-02/03/04/05`: reconciliação e previews reais em ambientes isolados.
5. `API-02/06` + `PLAYER-01`: MFA/CSRF/contas GNET, scoping e vínculo de jogador por realm.

**Riscos:** documentos não substituem evidência de runtime, e contas web não são contas GNET. Evitar apresentar 'ready' com simples RPC WebSocket/TCP, badge ou job de dispatch.
