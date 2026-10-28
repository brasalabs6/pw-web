# TODO.md — Atualizador seguro de cliente WASM e assets

> **Componente:** `packages/update-kit` · **Especificação canônica:** [SPECS.md](SPECS.md) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Legenda:** `[x]` código visto no repositório; `[ ]` requisito pendente. Prioridade P0 = bloqueante; P1 = hardening/integracão; P2 = evolução.

## Avanço verificado em 2026-10-28

**Escopo:** alterações na branch `feat/multi-realm-platform-shell-mvp` com jobs de TypeScript, PostgreSQL, update-kit, SDK e Rust. `[x]` significa aceite local de workorder específico, não certificação de produção.

- **Parcial** — UPDATE-02: manifesto JSON canônico, assinatura Ed25519, URL mesma origem, caminhos e objetos limitados, testes de cache e duplicidade; ainda falta NFC e testes completos de redirect real.
- **Parcial** — UPDATE-03: endpoint de release/preflight separado de emissão do ticket; iniciar WASM e reemitir ticket após patch fica para PR #23.
- **Parcial** — UPDATE-08: cache por SHA com revalidação e cancelamento por AbortController; faltam retries, prioridades e DAG.

**Bloqueio transversal:** gameplay real depende do cliente Angelica WASM PR #23 e do servidor com `gs`, WDB e login→mundo comprovados; previews de servidor continuam apenas diagnósticos.

## Baseline existente

- [x] digest(Uint8Array)->SHA-256 hex; canonicalJson(value) ordena keys lexicograficamente, proíbe floats não seguros e serializa arrays/objetos.
- [x] verifyManifest(raw,expectedDigest,keys): Zod schema2, recalcula digest de campos sem releaseDigest/signature, compara com expectedDigest e verifica assinatura Ed25519 de `pw-client-release-v2\n${digest}` usando public key allowlisted.
- [x] prepareClient({manifestUrl,expectedDigest,trustedKeys,onProgress,signal}): GET manifesto no-store, limite texto 2MiB, verifica e busca entry.js, entry.wasm e files; progresso completed/total/downloaded/phase.
- [x] verifiedBytes: HTTPS (localhost permite HTTP), bytes max 256 MiB por objeto, CacheStorage `pw-assets-sha256-v1` por SHA-256, rehash do cache, fetch com credentials omitted, verificação de tamanho/digest e put após prova.
- [x] canLaunchOriginalClient(): boolean retorna false; não simula um WASM funcional; integração com PR #23 pendente.
- [x] Documentação local SPEC + TODO criada, com ownership compartilhado e dependências.
- [ ] Aceite integral do package em ambiente de produção com prova cruzada, sem falhas de segurança.

## Backlog detalhado (cada item é workorder independentemente verificável)

### P0 — Fundação/bloqueantes

- [ ] **UPDATE-01 — Vincular manifests v1/asset DAG da branch PR #23 ao schema2 de update**
  - **Depende de:** contracts, port PR #23.
  - **Aceite verificável:** EntryBundle apenas e streaming de mapa por demanda.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UPDATE-02 — Fazer validação canonical bytes/duplicate-key/paths/URLs allowlisted**
  - **Depende de:** contracts, signer.
  - **Aceite verificável:** Truncamento, traversal e CDN manipulada rejeitados.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UPDATE-03 — Mover ticket curto GNET para pós-patching e renovar session pin seguro**
  - **Depende de:** control-api, shell.
  - **Aceite verificável:** Update lento não expira conexão antes do start.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UPDATE-04 — Implementar entry ABI JS/WASM + mount VFS por release**
  - **Depende de:** src/client153, shell.
  - **Aceite verificável:** Runtime original abre arquivos homologados e inicia jogo real.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UPDATE-05 — Garantir assinatura preview/prod separada e publisher confiável**
  - **Depende de:** tools/release, CI.
  - **Aceite verificável:** Private key nunca em bundle ou PR externo.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

### P1 — Integração/robustez

- [ ] **UPDATE-06 — Suportar chunked streaming/OPFS/IndexedDB/CacheStorage fallback e quotas**
  - **Depende de:** PR #23, shell.
  - **Aceite verificável:** Baixo pico de RAM mobile medido.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UPDATE-07 — Adicionar leases multi-tab/LRU e recuperação de crash/partial update**
  - **Depende de:** shell.
  - **Aceite verificável:** Sem corromper assets de sessão ativa.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UPDATE-08 — Adicionar dedupe/hash por objeto, retries finitos, cancel, priorities**
  - **Depende de:** @pw/contracts, content DAG.
  - **Aceite verificável:** Sem arquivo sem hash ou status READY parcial.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UPDATE-09 — E2E Chrome mobile/desktop cold/warm/offline e telemetria**
  - **Depende de:** CI browser, shell.
  - **Aceite verificável:** Bench p50/p95 e bytes úteis registrados.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

### P2 — Evolução

- [ ] **UPDATE-10 — Avaliar patches diferenciais por chunk/codec após medições**
  - **Depende de:** asset cooker, CDN.
  - **Aceite verificável:** Economia comprovada sem violar integridade.
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
