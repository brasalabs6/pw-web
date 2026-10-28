# TODO.md — Design system compartilhado dos três frontends

> **Componente:** `packages/ui` · **Especificação canônica:** [SPECS.md](SPECS.md) · **Branch:** `feat/multi-realm-platform-shell-mvp`.
> **Legenda:** `[x]` código visto no repositório; `[ ]` requisito pendente. Prioridade P0 = bloqueante; P1 = hardening/integracão; P2 = evolução.

## Avanço verificado em 2026-10-28

**Escopo:** alterações na branch `feat/multi-realm-platform-shell-mvp` com jobs de TypeScript, PostgreSQL, update-kit, SDK e Rust. `[x]` significa aceite local de workorder específico, não certificação de produção.

- **Parcial** — UI-02: catálogo retorna status derivado de health, mas o componente Status ainda aceita string livre; contrato visual permanece aberto.

**Bloqueio transversal:** gameplay real depende do cliente Angelica WASM PR #23 e do servidor com `gs`, WDB e login→mundo comprovados; previews de servidor continuam apenas diagnósticos.

## Baseline existente

- [x] Layout({title,subtitle,children,actions}) -> topbar, main e footer; mensagem padronizada de ambiente em desenvolvimento.
- [x] Status({value}) -> class `status-<value>` e label visível; enum de status ainda é string livre.
- [x] AuthPanel({onLogin}) -> modos login/register, email/password/displayName, validação HTML básica, chamadas api.register/login, erro/busy.
- [x] style.css define root dark, cards, grid, status, buttons, forms, progress, table e media query <=700px.
- [x] React components usam @pw/api-sdk diretamente em AuthPanel; desacoplar via controller/auth model para testes e reuso.
- [x] Documentação local SPEC + TODO criada, com ownership compartilhado e dependências.
- [ ] Aceite integral do package em ambiente de produção com prova cruzada, sem falhas de segurança.

## Backlog detalhado (cada item é workorder independentemente verificável)

### P0 — Fundação/bloqueantes

- [ ] **UI-01 — Desacoplar AuthPanel do transporte: callbacks/hooks injetados**
  - **Depende de:** api-sdk, shell, portal.
  - **Aceite verificável:** Componente testável sem rede real.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UI-02 — Padronizar tipos Status por contrato e casos ready/maintenance/error**
  - **Depende de:** contracts.
  - **Aceite verificável:** Visual não anuncia online com liveness falso.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UI-03 — Criar acessibilidade base com focus, aria, keyboard, contrast**
  - **Depende de:** shell, admin.
  - **Aceite verificável:** WCAG testes automáticos e manuais.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

### P1 — Integração/robustez

- [ ] **UI-04 — Implementar component library inputs/tables/modal/confirm/progress/toast**
  - **Depende de:** admin, shell.
  - **Aceite verificável:** Sem estilos duplicados em apps.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UI-05 — Criar stories/fixtures/visuais e testes de regressão**
  - **Depende de:** CI frontends.
  - **Aceite verificável:** Desktop/mobile consistentes.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UI-06 — Internacionalização e temas com design tokens**
  - **Depende de:** shell, portal.
  - **Aceite verificável:** pt-BR default e fallback explícito.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

- [ ] **UI-07 — Skeleton/loading/error states e layouts com memória/latência**
  - **Depende de:** api-sdk.
  - **Aceite verificável:** Sem blank screen ou anúncio de sucesso falso.
  - **Evidência requerida:** teste positivo/negativo, revisão de contrato, log de CI reproduzível, commit SHA e reviewer.
  - **Concluído?** Não.

### P2 — Evolução

- [ ] **UI-08 — Componentes avançados de mapa/health, diff, audit e approvals**
  - **Depende de:** admin-console, observability.
  - **Aceite verificável:** Table virtualization e desempenho medido.
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
