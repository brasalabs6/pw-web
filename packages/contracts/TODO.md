# TODO.md — Contratos e schemas compartilhados da plataforma

> **Componente:** `packages/contracts` · **Especificação canônica:** [SPECS.md](SPECS.md) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Legenda:** `[x]` código visto no repositório; `[ ]` requisito pendente. Prioridade P0 = bloqueante; P1 = hardening/integracão; P2 = evolução.

## Baseline existente

- [x] realmStatus = offline | maintenance | starting | online; releaseStatus = draft | validated | published | revoked.
- [x] operationStatus = queued | dispatched | running | ready | failed | cancelled | blocked; atenção: operations-worker usa também `dispatching`, discrepância atual do contrato.
- [x] previewKind = client | server | paired; createPreviewInput exige commits 40 hex pelo kind, TTL 1..72, realmId opcional.
- [x] catalogRealm: id uuid, slug, name, description, status, population int|null, defaultChannel; apresentação pública exclui segredos do host.
- [x] registerInput: email normalizado, password com mínimo 12 e max 128, displayName 2..60; loginInput email+password.
- [x] Documentação local SPEC + TODO criada, com ownership compartilhado e dependências.
- [ ] Aceite integral do package em ambiente de produção com prova cruzada, sem falhas de segurança.

## Backlog detalhado (cada item é workorder independentemente verificável)

### P0 — Fundação/bloqueantes

- [ ] **CONTRACT-01 — Alinhar operationStatus com dispatching e state machine persistida**
  - **Depende de:** operations-worker, database.
  - **Aceite verificável:** Nenhum estado SQL sem variante no schema; teste de transições.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **CONTRACT-02 — Extrair schemas de payload/response de todas as rotas control-api**
  - **Depende de:** control-api, api-sdk.
  - **Aceite verificável:** OpenAPI e TS convergem sem validators locais duplicados.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **CONTRACT-03 — Criar schema de runtime bootstrap assinado/ticket curto e canonicalização**
  - **Depende de:** shell, gateway, update-kit.
  - **Aceite verificável:** Troca de release/realm e replay rejeitados.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **CONTRACT-04 — Garantir identidade de cliente→servidor por pairing e manifest v2**
  - **Depende de:** tools/release, PR #23.
  - **Aceite verificável:** Compatibilidade não depende de string declarativa somente.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **CONTRACT-05 — Validar paths/sizes/URLs do manifest (NFC, traversal, bytes, origins)**
  - **Depende de:** update-kit.
  - **Aceite verificável:** Paths inválidos e object bombs rejeitados.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

### P1 — Integração/robustez

- [ ] **CONTRACT-06 — Modelar schema de previews/jobs/health+proof, CLI e eventos**
  - **Depende de:** admin-console, agent, worker.
  - **Aceite verificável:** Publisher e UI usam estados e evidências únicas.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **CONTRACT-07 — Gerar OpenAPI/golden fixtures e consumidores Rust**
  - **Depende de:** gateway, realm-agent.
  - **Aceite verificável:** Schema drift detectado na CI.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **CONTRACT-08 — Documentar compatibilidade de v1 asset/release de PR #23 com v2**
  - **Depende de:** content-manifest, releases.
  - **Aceite verificável:** Migração sem alterar digests antigos.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

### P2 — Evolução

- [ ] **CONTRACT-09 — Versionar protocolos futuros 1.4.4/1.3.6 somente com provas**
  - **Depende de:** future adapters.
  - **Aceite verificável:** Client/asset/wire profile suportados com testes reais.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

## Cadeia de dependências e sequência de execução

1. Fechar cada P0 que cria contrato compartilhado usado por apps: teste cross-module antes de alteração de consumidor.
2. Executar P0 de segurança/compatibilidade com cenário negativo; falta de schema/arquivo não pode causar fallback silencioso.
3. Executar P1 de durabilidade, monitoramento, testes e ergonomia após end-to-end mínimo.
4. P2 somente depois de validar o port 1.5.3 no navegador e o servidor com `gs` jogável.
5. Atualizar as evidências e marcar `[x]` por workorder somente após execução real.

## Gate do package

- [ ] Código e schema consistentes com `SPECS.md`; nenhum documento afirma feature ausente.
- [ ] Dependências de outras apps/packages integradas sem duplicação.
- [ ] Testes de regressão e adversariais atualizados e verdes no commit do PR.
- [ ] Segurança, concorrência e segregação de dados por realm/env verificadas quando aplicável.
- [ ] Documentação revisada após code review e migração preserva releases/sessões antigas.
- [ ] Qualquer ação de produção tem aprovação explícita, evidência e rollback.

**Contrato obrigatório:** o jogador escolhe o servidor; o backend resolve a versão do cliente automaticamente; o cliente verifica release/assetset/assinatura antes de baixar ou iniciar o jogo.
