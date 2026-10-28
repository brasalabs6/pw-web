# PW Web — sete aplicações e launcher automático

**Nova branch**: feat/multi-realm-platform-shell-mvp, derivada do servidor
feat/pw-source-1.5.3-ci-build no SHA 3e1caf44cc08200fb4f7d6510284f883a3953fcd.
A implementação é um **primeiro corte de plataforma**; NÃO é jogo jogável,
não integra ainda o cliente do PR #23 e NÃO foi publicada em produção.

## O que cada app faz

| App | Implementado | Bloqueios |
| --- | --- | --- |
| apps/shell | React/Vite: lista realms, seleciona, autentica, resolve versão automaticamente, cache SHA-256 | WASM Angelica e bridge de login/GNET do PR #23 não integrados; botão Jogar desabilitado |
| apps/player-portal | Conta web, registro, login, logout | Não é conta GNET; personagens e gold ainda faltam |
| apps/admin-console | Realms, criação e manutenção, lista de builds/releases, fila de previews | UI de GM, mapas, rollout, configs e backups ainda faltam |
| apps/control-api | Hono Node/PostgreSQL; catálogo, contas, sessões, compatibilidade, releases, deployments, readiness, tickets únicos, RBAC e audit | Ainda não é app Workers; publisher e auth GNET ainda faltam |
| apps/operations-worker | Despacha jobs de preview para workflow GitHub Actions | O workflow inicial só valida input; não provisiona preview jogável |
| apps/game-gateway | Rust: WebSocket binário autenticado → TCP GNET fixo | Precisa proxy TLS/WSS e cliente real para homologação de wire |
| apps/realm-agent | Rust: loopback, bearer e allowlist systemd, start/stop/restart serviços | Não altera mapa em runtime nem monitora saúde semântica sozinho |

**A versão nunca é escolha do jogador.** O shell apresenta somente servidores.
Depois que o jogador seleciona um e faz login, a API resolve
realm → stable channel → homologação de pairing → release cliente
→ manifest assinado → assets/cache. Servidor sem deployment REAL+READY ou
cliente sem release compatível devolve HTTP 409; nunca inicia runtime falso.

## Atualização e segurança

packages/update-kit verifica:
- manifesto cliente v2, releaseDigest = SHA-256 do JSON canônico de seus campos
  (sem releaseDigest/signature);
- assinatura detached Ed25519 sobre bytes UTF-8 da sequência
  pw-client-release-v2, nova linha e digest hexadecimal;
- chave pública confiável configurada em VITE_RELEASE_KEYS_JSON no build do shell;
- bytes/tamanho/SHA-256 de cada objeto. CacheStorage usa chave por hash e
  corrige caches adulterados; objeto igual não é baixado de novo;
- URLs HTTPS (exceção localhost) e apenas cliente do pairing retornado pelo servidor.

O signer está em tools/release/sign-client-v2.mjs. A chave privada jamais deve
ser incluída em assets ou build público. Exemplo de operação confiável:

    RELEASE_KEY_ID=preview-test \
    RELEASE_ED25519_PRIVATE_KEY_PEM="$(cat /path/private.pem)" \
    node tools/release/sign-client-v2.mjs unsigned.json signed.json

O comando retorna digest e chave pública raw base64. Configure a chave pública
no shell; não autorize assets/release sem direitos de distribuição.

**O update-kit é funcional para conferir e cachear manifest/objetos, mas o
entrypoint do cliente e o streaming de mapas ainda NÃO estão conectados.**
É necessário importar/adaptar os contratos de release-manifest v1 do PR #23,
resolver o asset DAG/entry bundles por mapa, instalar um entry ABI seguro
e testar cold/warm/mobile com o cliente original.

## Execução local

Node 22+, npm, PostgreSQL 16, Docker Compose e Rust stable:

    docker compose -f compose.platform.yml up -d
    npm install
    export DATABASE_URL='postgres://pw:pw_local_only@127.0.0.1:5433/pw_control'
    export APP_ORIGINS='http://localhost:5173,http://localhost:5174,http://localhost:5175'
    npm run db:migrate
    npm run dev:api          # 127.0.0.1:8787
    npm run dev:shell        # 127.0.0.1:5173
    npm run dev:portal       # 127.0.0.1:5174
    npm run dev:admin        # 127.0.0.1:5175
    npm run dev:operations
    npm run typecheck && npm run build:web && npm run test:platform

Rode os comandos dev em terminais separados. Variáveis em .env.platform.example
são exemplos, não são carregadas automaticamente. Controle de acesso: registre
a conta normalmente; promova explicitamente um operador verificado por SQL:

    UPDATE users SET role='admin' WHERE email='operador@example.com';

Sem admin default, sem conta demo previsível. Realms começam offline.
Banco PostgreSQL é só do control plane; WDB/BDB do Perfect World não é tocado.

Gateway:

    export GATEWAY_CONTROL_TOKEN="$(openssl rand -hex 32)"
    export GATEWAY_DEPLOYMENT_ID='<uuid-do-deployment>'
    export CONTROL_API_URL=http://127.0.0.1:8787
    export GNET_UPSTREAM=127.0.0.1:29000
    cargo run -p pw-game-gateway

Usar MESMO GATEWAY_CONTROL_TOKEN no control-api; o gateway só escuta
127.0.0.1:9001, exige ingress TLS/WSS para browsers. Destino GNET é fixado
por env no host e nunca informado pelo browser. O gateway consome ticket único
via control API antes de abrir o TCP.

Agente:

    export AGENT_SHARED_TOKEN="$(openssl rand -hex 32)"
    export AGENT_ALLOWED_UNITS='pw-gauthd.service,pw-glinkd.service,pw-gs01.service'
    cargo run -p pw-realm-agent

Só opera unidades systemd reais e allowlisted no próprio host. Sem SystemD
e privilégios mínimos corretamente provisionados, ações falham com erro real.
Por padrão escuta apenas 127.0.0.1:9010.

## Preview e CI

platform-ci.yml executa análise de TypeScript, build das três SPAs,
teste de PostgreSQL com API HTTP real, smoke de código e build Rust. O workflow
diagnóstico pw-platform-preview.yml **não** provisiona preview de servidor
porque ainda não existe infraestrutura isolada de volumes WDB, gs, assets e
gate de licença. O worker não marca READY por um mero dispatch. Para habilitar
dispatch no GitHub, o workflow deve estar disponível conforme política da
branch padrão e credenciais devem ser configuradas. Não compartilhar secrets
de produção com PRs não confiáveis.

## Próximos P0 até jogar no browser

1. Importar a ABI de entrada Angelica/Emscripten e assets autorizados do PR #23.
2. Provar GNET handshake completo login→CreateRole→EnterWorld com browser.
3. Criar vínculo seguro identidade web ↔ gauthd e contas de jogo reais.
4. Produzir manifests assinados do cliente real com asset DAG e atualizar por
   versão do servidor, com testes de navegador e mobile.
5. Subir stack completo com gs, WDB seguro, gateway TLS e health por mapa.
6. Validar CI, reteste de crashes, hardening, rate limiting e observabilidade.
7. Completar os fluxos de gerenciamento/admin e provider de previews isolados.

Documentação detalhada de produto/arquitetura: docs/PAINEL.md.
