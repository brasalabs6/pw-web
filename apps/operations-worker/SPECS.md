# SPECS.md — Operations worker / orquestrador assíncrono

> **Componente:** `apps/operations-worker` · **Status:** implementação inicial (MVP em evolução) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Fonte de verdade do código:** caminhos abaixo. **Referências integradas:** `docs/PAINEL.md`, `docs/PLATFORM-IMPLEMENTATION.md`, `packages/contracts/src/index.ts`, `packages/database/migrations/001_platform.sql`.
> **Regra:** `[ATUAL]` descreve o commit inspecionado; `[ALVO]` descreve contrato/feature ainda não implementados. Não confundir especificação com comportamento testado.

## 1. Missão, problema e decisões de produto

- Executar operações lentas e perigosas fora das requisições HTTP; promover um registro de intenção em job auditável e reconciliável.
- Acionar pipelines confiáveis de release, preview isolado, atualização e rollback; respeitar limites, aprovações, TTL e isolamento.

**Stack e deploy:** TypeScript Node/tsx, pg SQL queue, GitHub Actions workflow_dispatch.

**Situação comprovada [ATUAL]:** Poller consome jobs preview.create e despacha workflow diagnóstico; não provisiona runtime, WDB/gs, não atualiza READY, nem monitora concluir o workflow.

**Regra transversal:** jogador escolhe **servidor** e autentica; o control plane fixa **automaticamente** a versão de cliente e os assets homologados daquele servidor. Nenhuma release não comprovada é lançada no navegador.

## 2. Localização e árvore do componente

```text
apps/operations-worker/
├── src/index.ts
├── package.json
├── SPECS.md  # contrato deste componente
└── TODO.md   # backlog executável com dependências e aceite
```

Dependências de build e runtime devem ser lidas no `package.json` ou `Cargo.toml` local; `Cargo.toml` na raiz do repo inclui as aplicações Rust.

## 3. Responsabilidades e fronteiras

### 3.1 Domínio exclusivo / ownership

- polling de operation_jobs queued com SELECT FOR UPDATE SKIP LOCKED, attempts/reserved_until e conversão de reserva vencida a BLOCKED
- workflow_dispatch com kind/clientCommit/serverCommit/operationId/TTL via GitHub API; marca 'dispatched' após HTTP 204
- erro estruturado em result e estado blocked no momento do dispatch; nunca anuncia READY por isso

### 3.2 O que é expressamente proibido neste componente

- interpretar código de PR como comando, assinar artifacts com segredo em PR não confiável, publicar em prod sem approvals
- recriar container/volume WDB no workflow atual, testar GNET/GS diretamente do app
- gerenciar idempotency_key do request HTTP (control-api é dono); consumir comandos não allowlisted

A UI/admin **nunca** executa scripts arbitrários; dados persistentes do jogo permanecem no WDB/BDB gerenciado pela stack do servidor, enquanto versões, deploys, contas web e jobs pertencem ao PostgreSQL do control plane.

## 4. Interfaces, operações e contratos públicos

1. Tabela SQL operation_jobs: id uuid, kind text, state text, payload jsonb, result jsonb, actor_id, idempotency_key UNIQUE, created_at, updated_at.
2. SELECT queued WHERE kind='preview.create' ORDER BY created_at LIMIT 1 FOR UPDATE SKIP LOCKED; update dispatching na transação.
3. GitHub POST /repos/{owner}/{repo}/actions/workflows/{workflow}/dispatches; inputs operation_id,kind,client_sha,server_sha,ttl_hours; 204 = aceitação de dispatch, não execução.
4. Workflow .github/workflows/pw-platform-preview.yml: somente valida os inputs e resume gates; não gera URL jogável.
5. Futuro: consumer de callback assinado, artifact digest, reconcile RUNNING→READY/FAILED, logs, TTL, cleanup, stage/promote.

**Norma de evolução de contratos:** mudanças em wire JSON/HTTP seguem `packages/contracts` + migracões compatíveis; mudanças C++ GNET precisam evidência de framing e compatibilidade do port PR #23 e do servidor PR #17. Não criar contratos duplicados por aplicação.

## 5. Modelo operacional, fluxo e máquina de estados

1. QUEUED -> DISPATCHING [transação] -> DISPATCHED [GitHub 204] ou BLOCKED [falha]; estados RUNNING/READY não têm handler implementado.
2. Na inicialização sem GITHUB_TOKEN: avisar e não consumir jobs; workers múltiplos coordenam via SKIP LOCKED.
3. Crash em DISPATCHING agora tem lease/attempt e expiração para BLOCKED (sem redispatch ambíguo); falta reconciliar run GitHub e permitir retry seguro após confirmação.
4. Job preview não é preview criado: precisa provisionamento + health GNET/gs e link assinado.

**Princípios:** identificadores estáveis `realm_id` e `deployment_id`; releases pinadas pelo SHA-256; channel pin protegido por `expectedRevision`; session ticket curto e de uso único. Diferenciar `liveness`, `transport_ready` e `gameplay_ready`.

## 6. Mapa de dependências e colaboração

- control-api registra requests e IAM, @pw/contracts valida preview schema, @pw/database gerencia job queue.
- admin-console exibe estado e pedidos; GitHub Actions é executor confiável; realm-agent opera servidor local e gateway participa smoke.
- tools/release + cloudflare + provider host são futuras etapas; assets autorizados mantidos privados até publicação aprovada.

**Regra anti-duplicação:** interfaces de HTTP, dados, assets, segurança e versionamento pertencem aos packages indicados. Este app deve consumir essas APIs; lógica comum não deve ser colada em várias aplicações.

## 7. Segurança, consistência e tolerância a falhas

- Separar token GitHub por ambiente e nunca executar script do PR com publicador privilegiado.
- Validar SHA completo e PR ownership/repo ref antes do dispatch; input não pode virar shell string.
- Há reserva e attempts para dispatch; na falha de resultado ambíguo bloqueia manualmente. Falta retry finito e reconciliador com run_id/artefatos.
- Arquitetura atual é processo Node permanente; se migrar a Workers/Queues, reescrever driver mantendo contrato de job e event outbox.
- Distinguir PENDING/BLOCKED/FAILED de READY, registrar metrics/trace por operationId.

Falhas devem resultar em código/estado explícito e **fail-closed**: não trocar silenciosamente para outra versão, realm, endpoint, backend, release de produção, dados WDB ou chaves desconhecidas. Log sem PII, password, token ou payload sensível.

## 8. Plano de validação e evidências exigidas

- teste com token ausente mantém queued sem mentira; 204 dispatch marca dispatched, 403/404 marca blocked.
- teste concorrência dois pollers + queda pós-transação + retry; webhook assinatura fake e spoof.
- integração com ambiente de preview realmente isolado e smoke login→role→world antes de READY.

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
