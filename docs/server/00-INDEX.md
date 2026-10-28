# Documentação do servidor Perfect World 1.5.3 (`src/server153`)

**Versão da documentação:** 2026-10-28 (UTC) · **Branch:** `feat/pw-source-1.5.3-ci-build` (PR #17)

Esta é a documentação estruturada do servidor PW 1.5.3 que compilamos,
corrigimos, operamos e validamos neste repositório. Ela consolida tudo que foi
aprendido com o código, com o runtime e com o cliente real PWBR 1.5.3 v145
(Build 2555) durante o goal de 2026-10-25 → 2026-10-28.

## Como ler esta documentação

| Arquivo | Conteúdo |
| --- | --- |
| [`01-OVERVIEW.md`](01-OVERVIEW.md) | O que é este servidor, proveniência, capacidades provadas e limitações |
| [`02-ARCHITECTURE.md`](02-ARCHITECTURE.md) | Topologia dos daemons, portas medidas, bibliotecas, fluxo de mensagens |
| [`03-BUILD.md`](03-BUILD.md) | Toolchain, estágios de build, workarounds dos Makefiles, CI |
| [`04-PROTOCOL.md`](04-PROTOCOL.md) | Protocolo GNET: framing, CompactUINT, handshake, RPCs, criptografia |
| [`05-LOGIN-CHAIN.md`](05-LOGIN-CHAIN.md) | Cadeia de login passo a passo, retcodes, dev-auth, re-login |
| [`06-OPERATIONS.md`](06-OPERATIONS.md) | Operação diária: container, boot, watchdog, logs, troubleshooting |
| [`07-DATA-AND-STORAGE.md`](07-DATA-AND-STORAGE.md) | Storage WDB, tabelas, clsconfig, data files v145, MariaDB |
| [`08-WORLDS-AND-INSTANCES.md`](08-WORLDS-AND-INSTANCES.md) | gs, fork de instâncias, worldtags, mundo paralelo is61, mapas |
| [`09-BUGS-AND-FIXES.md`](09-BUGS-AND-FIXES.md) | Catálogo completo de bugs: sintoma → causa raiz → fix → evidência |
| [`10-CLOUDFLARE.md`](10-CLOUDFLARE.md) | Deploy em Cloudflare Containers: worker, DO, limites da plataforma |
| [`11-TESTING.md`](11-TESTING.md) | Suite de testes: wire conformance, probes, health, critérios |
| [`12-SECURITY.md`](12-SECURITY.md) | Chaves de sessão, dev-auth, exposição de rede, segredos |
| [`13-GLOSSARY.md`](13-GLOSSARY.md) | Glossário de termos e siglas |

## Fontes canônicas (hierarquia)

1. `src/server153/PROVENANCE.md` — origem da source, revisão pinada, autorização.
2. `docs/LEGAL.md` — decisão jurídica 2026-10-25 que autoriza o port.
3. `docs/SERVER153-REPORT.md` — relatório factual de completude, tamanhos e
   descobertas; os *findings* numerados (§4) são a fonte dos commits.
4. `docs/RESEARCH.md` — ledger de upstreams, evidências de protocolo, hipóteses.
5. `docs/DECISIONS/ADR-0006-webtransport-gateway.md` — arquitetura de gateway
   browser→glinkd (fase WebSocket/WebTransport).
6. Esta pasta (`docs/server/`) — documentação de referência do servidor em
   operação; quando houver conflito com os relatórios acima, os relatórios
   prevalecem e esta doc deve ser corrigida.

## Convenções de escrita

- **PT-BR** para prosa; **identificadores, código e nomes de arquivo em inglês**.
- Toda afirmação factual carrega a fonte: `arquivo:linha`, nome de
  função/classe, log real, ou commit SHA do branch.
- Estado de conhecimento explícito:
  - **PROVADO** — evidência reproduzível existe (log, teste, captura).
  - **INFERIDO** — lido no código, coerente com observações, sem prova direta.
  - **DESCONHECIDO** — não mapeado; dizer isso é preferível a inventar.
- Datas em UTC. Medidas de runtime citam o momento da medição.
- **Nunca** incluir credenciais, chaves de sessão completas, tokens ou IPs
  internos. Chaves aparecem apenas como padrão/primeiro caractere (ver
  [`12-SECURITY.md`](12-SECURITY.md)).

## Estado no momento desta versão

- Build 100% verde localmente e no CI (`server153-build.yml`, 5/5 checks).
- Stack rodando no container `pw153-runtime` (8 daemons + gs, watchdog ativo).
- Fluxo completo PROVADO com o cliente real via Wine: login → criação de
  personagem → entrada no mundo (is61) → movimento persistido → re-login.
- Deploy Cloudflare Containers VALIDADO: 8 daemons no container gerenciado,
  health público 200, TCP de mediação ao glinkd 200.

## Manutenção desta doc

- Qualquer fix de bug novo entra em [`09-BUGS-AND-FIXES.md`](09-BUGS-AND-FIXES.md)
  com commit e evidência; atualizar também o *finding* correspondente em
  `docs/SERVER153-REPORT.md`.
- Mudanças de comportamento de protocolo atualizam
  [`04-PROTOCOL.md`](04-PROTOCOL.md) e, se houver captura de wire, o teste de
  conformância (`tests/server153_createrole_wire.cpp`).
- Mudanças operacionais (portas, ordem de boot, scripts) atualizam
  [`06-OPERATIONS.md`](06-OPERATIONS.md) e o cabeçalho do script alterado.
