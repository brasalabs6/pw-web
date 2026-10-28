# SPECS.md — Design system compartilhado dos três frontends

> **Componente:** `packages/ui` · **Package:** `@pw/ui` · **Status:** implementação inicial em `feat/multi-realm-platform-shell-mvp`.
> **Autoridade:** o código versionado neste package, `docs/PAINEL.md` e `docs/PLATFORM-IMPLEMENTATION.md`. `[ATUAL]` = implementado no repo, `[PROPOSTO]` = objetivo ainda sujeito a código/teste.
> **Objetivo central da plataforma:** shell mostra servidores; o cliente e sua atualização são determinados automaticamente pelo pairing do realm. O package mantém sua própria fronteira sem inventar suporte 1.3.6/1.4.4.

## 1. Propósito e princípios de design

- Fornecer componentes comuns coerentes entre shell, portal e admin sem duplicar CSS/layout/status e controles.
- Aplicar acessibilidade, responsividade, identidade visual e segurança de interface; manter renderização de dados não confiáveis segura.
- Separar componentes visuais (presentational) de fluxo de autenticação/rede para reutilizar em múltiplos ambientes.

**Stack:** React 19, CSS nativo, TypeScript JSX; @pw/ui com entry ./src/index.tsx e ./src/style.css.

**Situação comprovada [ATUAL]:** Expõe Layout, Status e AuthPanel; CSS responsivo base. Sem biblioteca headless, testes a11y, tokens semânticos de múltiplos temas, i18n e forms desacoplados.

### Princípios inegociáveis

- Cada dado/ação tem um *owner*; evitar lógica repetida em apps ou packages.
- Tipos TypeScript não provam dado remoto: validar na borda e em runtime quando necessário.
- Compatibilidade entre cliente, assets, GNET, servidor e DB do jogo deve ser explicitamente homologada.
- Não confundir referências de release ou schema v2 com a versão v1 dos manifests do PR #23.
- `[PROPOSTO]` não constitui serviço/deployment ou aceitação de produção.

## 2. Árvore de arquivos e superfície exportada

```text
packages/ui/
├── package.json
├── src/index.tsx
├── src/style.css
├── SPECS.md
└── TODO.md
```

### Contratos atuais (tipos, endpoints, dados ou funções)

1. Layout({title,subtitle,children,actions}) -> topbar, main e footer; mensagem padronizada de ambiente em desenvolvimento.
2. Status({value}) -> class `status-<value>` e label visível; enum de status ainda é string livre.
3. AuthPanel({onLogin}) -> modos login/register, email/password/displayName, validação HTML básica, chamadas api.register/login, erro/busy.
4. style.css define root dark, cards, grid, status, buttons, forms, progress, table e media query <=700px.
5. React components usam @pw/api-sdk diretamente em AuthPanel; desacoplar via controller/auth model para testes e reuso.

### Modelo conceitual e invariantes

- Sem persistência direta: estado local em AuthPanel de mode/email/password/busy/error.
- Status não define categoria semântica de operação; status online da API exige readiness, não apenas cor verde.
- Layout contém links e marca PW Web; admin e player compartilham elementos, mas permissões não são da UI.

## 3. Mapa de relações: produtores, consumidores e ownership

- apps/shell importa Layout/Status/AuthPanel, estiliza lista de realms e barra de download.
- apps/player-portal importa Layout/AuthPanel; apps/admin-console importa Layout/Status e formulários.
- @pw/api-sdk usado pelo AuthPanel atualmente; @pw/contracts define categorias status e validação para componentes futuros.
- Control-api controla RBAC; esconder um botão por role não equivale à autorização no backend.

| Direção | Módulo | Responsabilidade de integração |
| --- | --- | --- |
| Produtor | Este package | Exporta tipos/funções/dados com contrato estável e versionado |
| Consumidores | Apps listadas acima | Importam pelo package name; não clonam implementação |
| Infra/persistência | Control API/servidor/CI conforme camada | Não contornar autenticação ou dono do banco |
| Evolução | `SPECS.md` + `TODO.md` + contratos compartilhados | Atualizar código/teste antes de marcar status concluído |

## 4. Regras de segurança, consistência e integridade

- Contrast ratios, foco visível, labels, teclado/touch e ARIA/roles precisam validação real no mobile.
- Nunca interpolar logs/URLs não confiáveis como HTML; React escaping padrão deve permanecer.
- Erros de autenticação não devem revelar email existente ou status interno; desabilitar submit durante busy e cancels.
- Evitar componentes com side effects de rede; AuthPanel hoje tem esse acoplamento e precisa teste/migração.
- Respeitar prefers-reduced-motion, texto multi-idioma, layout com 320px e suporte a telas grandes.

**Falha segura:** rejeitar formato desconhecido, versão incompatível, hash incorreto, endpoint não confiável, inconsistência de realm/zone/edition ou credencial inválida. Nunca fazer fallback para latest, produção ou binário não homologado.

## 5. Fluxo de implementação e integração

1. Definir contrato local e consumidor/produtor explícitos.
2. Criar schema/fixture ou migration idempotente de dados/artefato e documentar sua versão.
3. Validar erro, replay, concurrency, compatibilidade e limites (tempo/memória/tamanho) antes da promoção.
4. Atualizar primeiro o package; só depois os apps de consumo. Não criar uma segunda autoridade.
5. Homologar com PR #17 e PR #23 quando envolver GNET, WDB, WASM ou asset profile.
6. Publicar evidência do build e testes com commit SHA; informar precisamente o que ainda não foi executado.

## 6. Testes e critérios de aceitação

- Vitest React Testing Library + axe para forms, loading, keyboard, focus restoration (pendente).
- Playwright desktop/mobile: shell choose realm, portal login, admin actions, status semantics.
- Visual snapshots/Chromium com dados de exemplo, sem capturar dados pessoais.

### Comandos de CI e reprodução

- TypeScript: `npm run typecheck`; três SPAs: `npm run build:web`.
- Database e integração HTTP: `npm run db:migrate` e `node tests/platform/api-smoke.mjs`, requerem PostgreSQL e control-api.
- Contratos/updater: `npm run test:platform` e `npm run test:updater`.
- Rust: `cargo test --workspace` para integração gateway/realm-agent (quando aplicável).
- Para gameplay verdadeiro: test harness Chrome desktop/mobile, servidor `gs` com dados autorizados, login→CreateRole→EnterWorld. **Ainda não está comprovado nesta branch.**

## 7. Riscos técnicos e decisões abertas

1. A política de versionamento de manifests e OpenAPI precisa evitar ambiguidades entre schemas v1/v2.
2. Envolver backend real exige isolation por realm/env/zone e prevenção contra vazamento de WDB/secret.
3. Mudanças que dependem de runtime/client original precisam ser executadas em branch/PR compatíveis; não considerar fixture diagnóstico como prova de jogo.
4. A estabilidade do package depende de interfaces negativas testadas (hash, protocolo, schema, assinatura, replay, storage, concorrência e sessão).
5. Review humano e verificação jurídica são gates para publicar assets ou fontes de terceiros.

## 8. Definition of Done e governança do package

- [ ] API e contratos versionados; compatibilidade aprovada entre produtor/consumidor.
- [ ] Cobertura positiva/negativa em CI e testes de runtime onde aplicável.
- [ ] Security review, PII/secrets e operação multi-realm revisados.
- [ ] Sem lógica duplicada em outras apps/packages, imports consistentes.
- [ ] Documentos de arquitetura, schemas e TODO alinhados ao código.
- [ ] Evidência no PR com commit, resultado e limites, e aprovação antes de produção.

**Backlog executável:** [TODO.md](TODO.md), com responsabilidades, dependências e critérios de aceite individuais.
