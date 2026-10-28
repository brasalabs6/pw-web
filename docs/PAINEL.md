# PAINEL.md — Plano diretor integral dos painéis de jogadores e administração do Perfect World Web 1.5.3

> **Documento mestre de investigação, arquitetura, especificação funcional, contratos, segurança, operações, migração e backlog.**
>
> **Data de elaboração:** 2026-10-28 · **Repositório:** `brasalabs6/pw-web` · **Branch analisada e destino deste arquivo:** `feat/pw-source-1.5.3-ci-build` (PR #17, base `main`). **Snapshot consultado:** `6f798f803098f7afe56027b02dd81345460f336e`; confirmar HEAD antes de implementar porque o PR pode continuar recebendo commits.
>
> **Natureza:** análise de código e documentos versionados + proposta detalhada de implementação. **Nenhum endpoint, dashboard, banco, métrica ou recurso futuro deve ser anunciado como já funcional.** Os resultados de runtime históricos reproduzidos abaixo vêm dos relatórios do próprio projeto; esta elaboração não realizou novo benchmark nem acessou o container em execução.
>
> **Convenção:** **[CÓDIGO]** = verificado em arquivo-fonte versionado; **[RELATADO]** = evidência de execução registrada no repositório, mas não reexecutada nesta análise; **[PROPOSTO]** = arquitetura/capacidade ainda a implementar; **[A VALIDAR]** = depende de prova ou decisão; **[BLOQUEANTE]** = risco impeditivo para publicar.

---

## 0. Objetivo, escopo e decisões essenciais

Este documento especifica **dois produtos distintos, compartilhando identidade e back-end de integração**:

1. **Portal do jogador:** criar conta real, verificar identidade, entrar no jogo, escolher realm, visualizar personagens, gerenciar segurança, recuperar credenciais, acompanhar gold/consumo, compras, suporte, bloqueios e eventos.
2. **Console administrativo:** saúde do stack, visão de daemons/zonas/instâncias/mapas, operação de infraestrutura, controle de configurações, jogadores, contas, GMs, economia e loja, eventos, moderação, backup, restauração, auditoria, deploys, alertas e capacidade.

O console administrativo **não deve ser uma página que executa shell/SSH arbitrário**. Deve ser um produto de operações sobre recursos tipados, políticas verificáveis e agentes locais restritos.

### 0.1 Resposta executiva às perguntas originais

| Pergunta | Descoberta e resposta |
| --- | --- |
| Qual branch está importando o servidor? | **`feat/pw-source-1.5.3-ci-build`, PR #17**, ainda draft no snapshot. Código sob `src/server153/`; origem comunitária `hrace009/PW-Source-1.5.3`, revisão de referência `7dde52f`. [CÓDIGO] |
| O servidor compila? | Build dos oito daemons e `gs` relatado como verde em Debian 12 com toolchain 32 bits; testes de wire e integração histórica registrados em `docs/server/`. **Build não equivale a operação pronta para produção.** [RELATADO] |
| Qual banco persiste os personagens? | **WDB/BDB embutidos**, sobretudo no `gamedbd` e no `uniquenamed`, não MariaDB. São ~48 storages nomeados na configuração de runtime do gamedbd. [CÓDIGO] |
| Há MySQL/MariaDB? | Documentação relata MariaDB com `pw.sql` importado no ambiente de teste, **não consumido por nenhum daemon**. Uma nova fonte de identidade terá de ser integrada a `gauthd`. [RELATADO] |
| Existe cadastro de contas real? | **Não.** `scripts/panel-server153.py` apenas valida/gera número múltiplo de 16; a senha de teste é igual ao número e não há insert persistente. [CÓDIGO, BLOQUEANTE] |
| Há loja de gold? | Existem arquivos `gshop*.data`, `gshopsev*.data`, o saldo em `User`/WDB e handlers `AddCash`, `DebugAddCash`, `InstantAddCash`, `cashserial` etc. **Não existe uma loja web transacional segura integrada.** [CÓDIGO] |
| É possível monitorar online/offline? | Daemons e portas: sim, pelos processos, sockets, logs e probes. **Jogadores online:** `GMOnlineNum` retorna tamanho de `UserContainer`; `GMListOnlineUser` enumera sessões. Faltam interfaces autenticadas e instrumentação confiável. [CÓDIGO] |
| É possível ligar/desligar mapas? | `gs` executa instâncias como processos independentes com `worldtag` e `gameid`; hoje a invocação conhecida inicia `gs01` e `is61`. **Não há controlador seguro e granular de ciclo de vida de mapas**, drenagem e reconciliação. [CÓDIGO/PROPOSTO] |
| Quanto consome? | Relatório do projeto menciona **26 MiB para oito daemons + HTTP** no Cloudflare `lite`; **isso não mede `gs`, mundo carregado, jogadores ou pico**. O custo real por mapa/jogador **ainda precisa ser medido**. [RELATADO] |
| Cloudflare já hospeda o jogo inteiro? | Worker/DO e oito daemons com health e TCP mediation relatados como funcionais; `cloudflare/entrypoint.sh` **não sobe o `gs`**, não expõe uma sessão de jogo pública completa nem comprova persistência durável. [CÓDIGO] |
| Adotar um painel existente? | **Não importar sem auditoria.** `shadowvzs/pwAdminV2` é a melhor referência arquitetural próxima de React/Nest; `hrace009/pwAdmin` é relevante para comandos; `tomirons/pw-web`, `ilhaaam/pw-web` e `hrace009/PW-Panel` são referências de produto, com tecnologia/segurança antiga ou bugs reportados. [PESQUISA EXTERNA] |

### 0.2 Decisões arquiteturais recomendadas

- **Manter intacto o banco WDB** como origem de verdade de personagens e economia do jogo; não tentar converter registros WDB em tabelas SQL genéricas nem gravar seus arquivos enquanto daemons os possuem.
- **Adicionar banco transacional próprio do painel** para identidades web, sessões, autorização, pedidos, ledger de cobrança, reconciliação, configuração desejada, jobs e auditoria; adotar PostgreSQL para o plano de controle é recomendação, não dependência comprovada do servidor.
- **Implementar um adaptador de autenticação no `gauthd`** para credenciais reais. A autenticação dev atual é insegura e deve ser removida/negada fora de testes.
- **Implementar uma ponte administrativa interna** que converse com APIs/RPCs tipados já existentes e com um agente de host restrito; nunca usar uma rota HTTP pública como proxy GNET sem autorização.
- **Separar plano de controle e plano de dados**: indisponibilidade do painel não pode travar o mundo ou a autenticação do jogo; indisponibilidade do jogo não pode destruir registros financeiros ou de auditoria.
- **Tratar mudanças de mapa/realm/configuração como operações declarativas, com plano, preflight, janela, confirmação e rollback**, não como simples `kill -9`.
- **Começar pela confiabilidade operacional + auth + painel observável** e só então ativar comércio e mutações de personagem.

---

## 1. Evidências, referências e hierarquia de confiança

### 1.1 Arquivos examinados diretamente

- `src/server153/cnet/{gauthd,gdeliveryd,gamedbd,glinkd,gacd}/` e `src/server153/cgame/gs/`.
- `src/server153/cnet/gamedbd/addcash.hpp`: atualização transacional de cash em `StorageEnv::Storage("user")`, `add_serial`, `cash_add` e cálculos de VIP.
- `src/server153/cnet/gamedbd/debugaddcash.hpp`: via de debug que altera gold — **proibir acesso em produção**.
- `src/server153/cnet/gamedbd/gamedbd.cpp`: CLI de manutenção e saída com checkpoint.
- `src/server153/cnet/gdeliveryd/{gmonlinenum.hpp,gmlistonlineuser.hpp,gmrestartserver.hpp,gmsetgameattri.hrp}`.
- `src/server153/cgame/gs/{gs.conf,gmserver.conf}`, ``cnet/{glinkd,gdeliveryd,gamedbd}/gamesys.conf`, `cnet/gauthd/gauthd.conf`, `cnet/gacd/gacd.xml`.
- `scripts/{build-server153.sh,boot-server153.sh,gs-server153.sh,watchdog-server153.sh,panel-server153.py,probe-server153.sh}`.
- `cloudflare/{worker.js,entrypoint.sh}`.
- `AGENTS.md`, `docs/SERVER153-REPORT.md`, `docs/server/00-INDEX.md` a `13-GLOSSARY.md`, `docs/DECISIONS/ADR-0006-webtransport-gateway.md`.

### 1.2 Documentos que devem continuar canônicos

- [Relatório real de fonte/runtime](SERVER153-REPORT.md).
- [Índice dos documentos de servidor](server/00-INDEX.md).
- [Topologia e protocolos](server/02-ARCHITECTURE.md).
- [Cadeia de login](server/05-LOGIN-CHAIN.md).
- [Operação diária](server/06-OPERATIONS.md).
- [Persistência](server/07-DATA-AND-STORAGE.md).
- [Mapas e instâncias](server/08-WORLDS-AND-INSTANCES.md).
- [Bugs e correções](server/09-BUGS-AND-FIXES.md).
- [Cloudflare](server/10-CLOUDFLARE.md).
- [Segurança](server/12-SECURITY.md).
- [ADR de transporte](DECISIONS/ADR-0006-webtransport-gateway.md).

**Regra de manutenção:** este documento propõe e consolida o painel, mas **não sobrescreve evidências operacionais**. Se um relatório antigo divergir de uma evidência mais recente, vincular data, commit, teste e resultado em vez de escolher a versão mais conveniente.

### 1.3 Distinção entre provas históricas

O relatório `SERVER153-REPORT.md` é um histórico cumulativo: em trecho anterior descreve login com cliente real e entrada no mundo ainda pendente. O índice `docs/server/00-INDEX.md`, atualizado em 2026-10-28, declara **login → criação → entrada is61 → movimento persistido → relogin** com cliente real. Tratar a prova mais recente como **[RELATADO]** até reproduzir com captura, artefatos de CI e duas sessões simultâneas. Uma prova local de sessão **não demonstra** capacidade de produção, concorrência nem recuperação após crash.

---

## 2. Anatomia real do stack

### 2.1 Diagrama de responsabilidades

```text
                     JOGADOR (cliente nativo, futuro WASM)
                                  │ GNET / TCP 29000
                                  ▼
                              [glinkd]
                        frontend e sessão do jogo
                        │       │           │
               29100    │       │ 29301     │ 29500
                        ▼       │           ▼
                   [gdeliveryd] │        [gfactiond]
                     plano de   │           ▲
                     mensagens  │           │ 29600
                     │ │ │ │    │           │
              9200   │ │ │ └────┼──> [gacd] 29702/29712
                     ▼ │ │      │
                 [gauthd]       │    (dev-auth atual)
                       │ │      │
          29400        │ ▼      │
                       [gamedbd]│
                       WDB/BDB  │
          29401        │        │
                   [uniquenamed]│
                       WDB      │
                                ▼
                      [gs: gs01 / is61]
                     mundo, NPCs, simulação
                      ↘ logservice ↙
                      11100/11101
```

**Nota:** o diagrama é funcional, não um diagrama de conexões exaustivas; `gs` estabelece conexões de provider com `gdeliveryd`, `glinkd` e `gfactiond`, consulta `gamedbd` e publica logs.

### 2.2 Serviços: papel e gerenciamento pretendido

| Serviço | Papel provado | Estado e interfaces desejadas |
| --- | --- | --- |
| `glinkd` | aceita clientes, challenge, negociação de chaves, lista de personagens e roteamento | taxa de accept/login, latência de handshake, sockets, half-login, sessões pendentes, limites; restart **coordenado** |
| `gdeliveryd` | UserContainer e roteador de RPC; sessão/online, compras, leilões, batalhas, eventos | player count, filas por prioridade, locks, providers registrados, forbids, eventos e GM API; alta criticidade |
| `gauthd` | nó terminal de auth e RPC de senha | latência/erro de autenticação, identidade real, retries e bloqueios; não expor porta |
| `gamedbd` | 48 storages WDB de dados do jogo e transações | checkpoint, backup, tamanho/cache por tabela, consultas tipadas, safe-mode, latência, integridade |
| `uniquenamed` | reserva nomes e IDs globais | colisões, tamanho das tabelas, atraso de alocação, estado WDB |
| `gfactiond` | clãs, relação de facções, fortalezas e guild operations | tamanho das filas, ações administrativas permitidas e diagnóstico de provider duplicado |
| `gacd` | subsistema anticheat configurado por XML | relatórios, estado do serviço, configurações de política versionadas, quarentena; sem expor detalhes exploráveis a clientes |
| `logservice` | centraliza logs de jogo, TCP e UDP | volume/retention, dropped logs, ingest lag, conversão GBK e correlação |
| `gs` | processo do mundo e das instâncias; terreno, colisão, NPC, física, regras | lista de instâncias e PID real, worldtag, capacidade, jogadores por instância, tick, CPU/RSS, readiness, graceful drain |

### 2.3 Portas **medidas historicamente** no ambiente local

| Porta/protocolo | Processo | Utilidade | Exposição pública |
| --- | --- | --- | --- |
| 29000/TCP | `glinkd` | cliente de jogo, GNET | somente via gateway público autorizado |
| 29301/TCP | `glinkd` | provider/link interno | **nunca** |
| 29100/TCP | `gdeliveryd` | delivery para links | **nunca** |
| 29300/TCP | `gdeliveryd` | providers de `gs` | **nunca** |
| 9200/TCP | `gauthd` | auth no runtime local | **nunca** |
| 29400/TCP | `gamedbd` | RPCs de dados | **nunca** |
| 29401/TCP | `uniquenamed` | nome/UID | **nunca** |
| 29500 e 29600/TCP | `gfactiond` | facções, GS provider | **nunca** |
| 29702 e 29712/TCP | `gacd` | anticheat e controle interno | **nunca** |
| 11100/UDP e 11101/TCP | `logservice` | logs | **nunca** |
| 10901 e 11971/TCP | `gs01` / `is61` | sockets por instância | **nunca** |
| 8080/HTTP | health Cloudflare / script de teste | **porta duplicada** em contextos separados | saúde sanitizada ou interna somente |

**Importante:** 9200/29200, 9001/29000 e 29701/29702/29709 variam nos arquivos upstream. **Os valores da tabela são do runtime corrigido** (`boot-server153.sh`), não promessa de todos os deployments. Um `port inventory` no agente deve descobrir configurações ativas e sockets por PID; não confiar em constantes.

### 2.4 Ordem de inicialização atualmente documentada

```text
gauthd → gdeliveryd → gamedbd → uniquenamed → gfaction → gacd
       → logservice → glinkd → gs (gs01 e is61)
```

Reprodução do ambiente de desenvolvimento documentado:

```bash
# Somente em ambiente descartável ou já preparado com as fontes e dados autorizados.
/work/scripts/boot-server153.sh
/work/scripts/gs-server153.sh
# Watchdog de laboratório:
/work/scripts/watchdog-server153.sh --interval 10
# Smoke de protocolo:
./scripts/probe-server153.sh
```

**Não usar** `--fresh` em produção: remove os bancos persistentes de vários daemons. **Não adotar** `boot-server153.sh --stop` como operação segura: o script usa sinais comuns, não executa todo o protocolo de drain/checkpoint e não gerencia plenamente a árvore dupla de processos do `gs`. O script é um estágio de laboratório, não um operator.

### 2.5 Arquivos de configuração e precedência

Inventário mínimo real:

| Diretório/arquivo | Função |
| --- | --- |
| `cnet/glinkd/gamesys.conf` | porta de login, max_users/max_conns, halflogin, buffers, compressão, version e thread pool |
| `cnet/gdeliveryd/gamesys.conf` | zoneid/aid, política de login, contatos entre daemons, filtros, intervalos, recompensas e workers |
| `cnet/gauthd/gauthd.conf` | listener, chaves de sessão e pool; **há material sensível versionado, não reproduzir** |
| `cnet/gamedbd/gamesys.conf` | storage, tabelas, cache, zonas, classes, checkpoint e pool |
| `cnet/uniquenamed/gamesys.conf` | nomes, UID e banco próprio |
| `cnet/gfaction/gamesys.conf` | facções, provider e filtros |
| `cnet/gacd/{gamesys.conf,gacd.xml}` | anticheat, regras e daemon |
| `cnet/logservice/logservice.conf` | transporte e sinks de logs |
| `cgame/gs/{gs.conf,gmserver.conf,gsalias.conf,ptemplate.conf}` | instâncias, terreno, provider, aliases, parâmetros do mundo e jogador inicial |
| `scripts/boot-server153.sh` | geração das cópias runtime de configs locais, com patches `sed` |
| `cloudflare/entrypoint.sh` | outra geração de configs, **duplicada e sujeita a drift** |
| pacote externo v145 de dados | `gamed/config/*` e assets/mapas indispensáveis |

**Precedência recomendada:** (1) baseline versionada compatível com o binário; (2) definição por realm/zone; (3) overrides de ambiente permitidos por schema; (4) segredo via secret manager; (5) artefato de configuração renderizado e imutável por release. **O serviço do painel não edita configs de source enquanto estão em uso**.

---

## 3. Persistência — banco de contas versus banco de jogo

### 3.1 O banco **não é MySQL**

`gamedbd` usa `StorageEnv` e abre storages em WDB/BDB. `uniquenamed` possui próprio conjunto WDB. Os arquivos são binários, serializados em estruturas GNET/Octets e administrados por daemons com transações, caches e checkpoints. A presença de SQL no ambiente não converte esse banco para relacional.

Consequências para o painel:
- Não ligar um ORM diretamente às tabelas binárias WDB.
- Não permitir ao frontend alterar bytes de `base`, `status`, `user`, `inventory` ou `rolename`.
- Não fazer leitura inconsistente durante escrita. Para consultas online usar RPC explícito; para relatórios em massa usar snapshot **consistente** exportado pelo daemon.
- Não apagar storage para “recuperar servidor”; criar runbooks de backup/restore e testar recuperação antes de receber jogadores reais.
- Não confundir cache administrativo de leitura com banco autoritativo.

### 3.2 Catálogo completo dos 48 storages informados no runtime

```text
auction, base, city, clsconfig, config, crslogicuid, equipment,
factionfortress, factioninfo, factionname, factionrelation, force,
friendext, friends, globalcontrol, gtask, inventory, kingelection,
mailbox, mappassword, messages, mndomainbonus, mndomaininfo,
mnfactionapplyinfo, mnfactioninfo, order, playerprofile, playershop,
recalluser, rolename, rolenamehis, sellpoint, serverdata, shoplog,
solochallengerank, status, storehouse, syslog, task, translog,
uniquedata, user, userfaction, userstore, waitdel, weborderitem,
webtrade, webtradesold
```

| Storage | Significado conhecido | Operações administrativas desejadas |
| --- | --- | --- |
| `user` | user info, rolelist, contas do mundo e campos `cash_*` | consultar com máscaras de dados, gold via transação, bloquear mutação direta |
| `base` | `GRoleBase` — classe, raça, gênero, level, referências de personagem | consultar e comparar, teleporte apenas pelo serviço de jogo |
| `status` | `GRoleStatus` — HP/MP/EXP, posição e atributos persistidos | consulta, histórico/snapshot; alterar somente via comando validado |
| `equipment` | itens equipados | view read-only e assistência GM auditada |
| `inventory` | inventário | view read-only; correção de item via transação autorizada |
| `storehouse` | armazém | igual ao inventário |
| `task` / `gtask` | missões/progresso | consulta, reset apenas com protocolo de compatibilidade |
| `clsconfig` | templates iniciais/spawn por classe | editar via release/revisão, não via formulário livre |
| `rolename` / `rolenamehis` | nomes/identidade histórica | consultas de nomes e renomeio com uniquenamed |
| `waitdel` | personagens pendentes de deleção | fila, TTL e políticas de restauração |
| `mailbox` | correio do jogo | suporte e auditoria de anexos |
| `factioninfo` / `factionname` / `factionrelation` | dados de facções | consulta e moderação |
| `auction` / `sellpoint` / `playershop` / `webtrade` | mercado e troca | consultas, prevenção a fraude, trilha de auditoria |
| `translog` / `shoplog` / `order` | evidência transacional | relatório financeiro, reconciliação e antifraude |
| `syslog` | registros de sistema | diagnóstico e investigação |
| `config` / `serverdata` / `globalcontrol` | estado/config do DB e controles globais | leitura tipada e mudanças com contrato |
| restantes | aspectos de arena, domínio, ranking, guild, recall etc. | primeiro inventariar `rpcdata` e structs por storage, depois habilitar operações |

**Nunca inferir o schema binário de uma tabela só pelo nome.** A especificação de cada adaptador deve apontar para a struct de serialização, versão, chave de índice, semântica transacional, compatibilidade com cliente v145 e testes golden.

### 3.3 Storage de nomes

`uniquenamed` mantém `config`, `uidrole`, `unamefaction` e `unamerole` e oferece RPCs de reserva de nomes antes da criação/renomeio. O painel **não pode** fazer um simples `UPDATE role.name` em banco relacional, pois a unicidade tem seu próprio fluxo e poderá envolver `gamedbd` e `gdeliveryd`.

### 3.4 O que o `gamedbd` já oferece via CLI

`gamedbd.cpp` possui comandos de diagnóstico/manutenção: `query`, `exportrole`, `importrole`, `listrole`, `listrolebrief`, `listuserbrief`, `listfaction`, `listroleinventory`, `listcity`, `listshoplog`, `listsyslog`, `listid`, `read`, `tablestat`, `tablestatraw`, `exportuserstore`, `printlogicuid`, `querywebtrade`, `queryglobalcontrol`, `returncash`, `deletewaitdel`, `repairdb`, `merge`, `mergewdb`, `convertdb`, `compressdb`, `decompressdb`, entre outros.

**O fato de uma subopção existir não significa que ela é online-safe.** O próprio `main()` abre `StorageEnv` e frequentemente executa `checkpoint/Close` ao final. Não disparar segunda instância de `gamedbd` sobre arquivos abertos sem confirmar o modelo de concorrência e recovery. O painel deve encapsular comandos em modo **offline com lock exclusivo**, snapshot e permissões distintas, ou substituí-los por RPC read-only seguro.

### 3.5 O que precisa de um novo banco relacional

O jogo e o painel têm ciclos de vida diferentes. Banco de **plano de controle** [PROPOSTO] com tabelas:

| Tabela proposta | Campos indispensáveis e semântica |
| --- | --- |
| `identities` | `id UUID`, email normalizado, hash Argon2id, status, verificação, criado/atualizado |
| `game_accounts` | `id`, `identity_id`, `realm_id`, `legacy_userid INT` único por zone, estado, auth adapter version |
| `realms` | `id`, `slug`, `name`, `version`, `maintenance`, `region`, `cluster_id` |
| `zones` | `id`, `realm_id`, `legacy_zoneid`, max_players, status |
| `characters_index` | espelho eventual de `roleid`, `userid`, `zoneid`, name, class, level, sync cursor; **não autoritativo** |
| `admin_users` / `roles` / `permissions` / `role_grants` | RBAC e escopo realm/zone/instância |
| `admin_sessions` / `mfa_factors` | sessão, MFA e políticas |
| `service_instances` | daemon, container, host, PID, revision, start time, heartbeat, status |
| `world_instances` | `instance_id`, worldtag, gameid, parent realm, map key, desired/observed, PID, population |
| `config_revisions` | tipo, scope, schema version, diff, hash, autor, aprovação, rollout, rollback |
| `operation_jobs` | type, target, idempotency key, actor, plan, status, progress, output sanitizado, TTL |
| `operation_events` | transições append-only de jobs |
| `audit_events` | ator, sujeito, action, alvo, antes/depois redigidos, IP aproximado, request id, UTC |
| `economy_ledger` | registros append-only de moeda externa/compra/concessão, quantia e unidade, origem, razão |
| `payment_orders` / `payment_events` | provedor, webhook id, verificação, estado, conciliação, chargeback |
| `cash_grants` | pedido → serial monotônico por userid/zoneid, status do RPC, confirmação em WDB |
| `store_catalog` / `store_catalog_revisions` | itens editáveis com source/version, aprovação, preço, pré-requisitos |
| `tickets` / `ticket_events` | suporte, anexos, notas e intervenções |
| `notification_jobs` | mensagens oficiais, push/mail, preferências, status |
| `backups` / `restore_drills` | origem, horário, artefato, checksum, retenção, resultado do teste de restauração |
| `alerts` / `incidents` / `maintenance_windows` | SRE, atribuição, estados e pós-incidentes |

**Chaves:** chave numérica `userid`/`roleid` é identidade técnica do legado, não substituto de `identity_id`; sempre indexar com `realm_id`/`zoneid` e impedir colisões entre realms. Não guardar senha em log, job, webhook, auditoria ou payload de erro.

---

## 4. Login, cadastro de contas e sessões: desenho seguro

### 4.1 Estado atual **não utilizável comercialmente**

`scripts/panel-server153.py` escuta em `127.0.0.1:8080`, apresenta apenas um formulário e testa conexão à porta 29000. Ele gera um número de conta (múltiplo de 16) e revela **senha igual ao próprio número**. Não cria registro, não valida e-mail, não persiste senha, não aplica políticas, não fornece recovery, MFA, ban nem duplicidade real. O `gauthd` aceita o algoritmo de desenvolvimento sem consulta SQL.

**Bloqueante:** uma conta numérica elegível é previsível e a senha é trivial. O produto não pode expor isso à internet, dar gold real ou receber pagamentos enquanto `dev-auth` estiver ativo.

### 4.2 Cadeia GNET atual

```text
Client → glinkd:29000 (Challenge)
       → Response → MatrixPasswd2 / MatrixPasswd / MatrixToken
       → gdeliveryd → gauthd (auth terminal atual)
       ← UserLogin2 success/failure
       → OnlineAnnounce → RoleList (via gamedbd)
       → CreateRole (uniquenamed + gamedbd)
       → EnterWorld (gdeliveryd seleciona gs por worldtag)
```

O cliente PWBR 1.5.3 v145 usa version `0x010503`, edition correlacionada aos dados e payload `CreateRole` compatível com **23 campos e 1704 bytes** nos testes documentados. **Não reinventar a autenticação ignorando GNET**: qualquer mudança precisa de teste com cliente real e com bridge WASM.

### 4.3 Estratégias para `gauthd` real

**Alternativa A — ponte de autenticação compatível com cliente legado (MVP preferencial):** criar store de contas reais e modificar/integrar `gauthd` para obter um verificador por userid e responder à challenge original. O login web usa Argon2id e sessão independente. O verificador exigido pelo protocolo legado **não pode ser derivado de um hash Argon2id**; manter um material de credencial específico do protocolo, criptografado em repouso e separado do hash web, ou emitir credenciais distintas para jogo. Desenhar exatamente qual derivação GNET o cliente necessita e validar por golden test. Nunca usar `senha = conta` ou manter backdoor dev.

**Alternativa B — adaptador para auth SQL compatível:** avaliar esquema MariaDB de auth já empregado pelo ecossistema PW, **sem presumir que o `pw.sql` local seja o correto**. Fixar dialeto, campos, flags de GM/bloqueio, comportamento de RPC e migrações. Usar um principal de DB com privilégios mínimos; não permitir query SQL do painel no processo de jogo.

**Alternativa C — autenticação por ticket dedicada ao cliente browser [FUTURO]:** conta web autentica por sessão/MFA, recebe ticket curto, de uso único, vinculado ao realm, e um bridge validado o traduz em autenticação server-side. Isso exige mudanças conscientes em cliente/protocolo/servidor e compatibilidade paralela com cliente original; não apresentar como capacidade existente.

**Escolha:** A para começar, preservando GNET. Projetar interface `AuthProvider` interna para suportar B/C sem acoplar cadastro ao WDB.

### 4.4 Sequência de criação de conta

1. Frontend solicita email/usuário/senha ou login federado com proteção CSRF e rate-limit.
2. `identity-service` valida, normaliza e reserva identidade em transação SQL; registra fluxo de verificação.
3. Após verificação, cria `game_accounts` com `legacy_userid` único e provedor de credencial seguro.
4. `auth-adapter` confirma capacidade real de autenticar o novo userid no `gauthd` (sem dev fallback).
5. Teste automatizado entra com cliente GNET de teste, confirma retcodes, logout e proibição quando bloqueada.
6. Painel exibe status `active/pending/error`; não mostra senha em texto claro, nunca gera password previsível.
7. Eventos assíncronos idempotentes atualizam espelho de personagem; não alteram WDB no cadastro sem contrato explícito.
8. Na falha, compensar pendências com job e preservar histórico para diagnóstico.

### 4.5 Funcionalidades do portal do jogador

| Tela | Visualização | Ações autorizadas | Dependência |
| --- | --- | --- | --- |
| Início | realm online/offline, avisos, status de manutenção, últimos logins | jogar, escolher realm | health consolidado |
| Criar conta | termos, senha forte, verificação | registrar | auth real |
| Login | email/username, autenticação segura | entrar/sair | sessions/mfa |
| Segurança | MFA, sessões/dispositivos, histórico | revogar, trocar senha, recuperar | identity |
| Minhas contas | conta game vinculada e zone | vincular/criar conforme política | auth adapter |
| Personagens | nome, classe, level, última atividade, status | exibir; ações restritas explicitamente permitidas | game query adapter |
| Gold | saldo **confirmado no WDB**, compras pendentes, histórico | consultar e comprar gold via checkout | economia/ledger |
| Loja | catálogo, disponibilidade, preço, taxa e condições | checkout; entrega após conciliação | store/payment adapter |
| Suporte | tickets, logs de compra, resposta | abrir, acompanhar e anexar | tickets |
| Punições | sanções com prazo e fundamento adequado | recorrer | moderation |
| Notícias/eventos | agenda/eventos/ranking quando autorizado | inscrever-se, acompanhar | CMS/events |
| Privacidade | dados, consentimentos, solicitações LGPD | exportação/solicitação de eliminação compatível com retenção legal | privacy |

### 4.6 Senhas, MFA, recuperação e bloqueio

- Argon2id para credencial web com política adaptada a hardware; TLS e cookies `Secure/HttpOnly/SameSite`.
- Recuperação por token aleatório, único, com expiração e revogação; nunca enviar senha atual.
- MFA obrigatório para GM/admin, recomendado para jogador; autenticação step-up para gold, e-mail e ações sensíveis.
- Login do jogo com controle de tentativas e bloqueio progressivo; não permitir enumeração de contas.
- Ban/suspensão devem ser aplicados **em auth + delivery**, inclusive sessões já abertas; ação idempotente, logs e justificativa.
- `status=online` web ≠ online game; usar presença de `UserContainer`, com atraso e confidence exibidos.

---

## 5. Loja de gold, cash shop e economia: análise real + especificação

### 5.1 Existem **duas dimensões** de loja

1. **Cash shop dentro do jogo:** arquivos binários de catálogo consumidos pelo `gs` e pelo cliente; `gshop.data`, `gshop1.data`, `gshop2.data`, `gshopsev.data`, `gshopsev1.data`.
2. **Venda web de crédito/gold:** pedido financeiro externo, conciliação do pagamento, concessão de cash ao `User` do jogo por protocolo transacional, revisão/chargeback/refund, histórico e auditoria.

É possível ter uma loja web visual sem conceder cash; **não** é possível vender gold de modo correto apenas adicionando um número em SQL.

### 5.2 Layout e compatibilidade do catálogo

`scripts/gs-server153.sh` detecta a versão antiga do `gshopsev*.data` de **196 bytes por `GSHOP_ITEM`** e a converte para **220 bytes**, preservando arquivos originais como `.old`. Isso é compatibilidade de **formato da fonte e dados v145**, não autorização para editar preços em tempo real.

**Editor proposto:**
- importar com versão e hash do arquivo;
- parser com tipos explícitos e golden tests; mostrar id, categoria, preço, moeda, quantidade, limites, VIP, prazo, preview e referências a `elements.data`;
- validação de item existente e compatibilidade cliente/servidor;
- diff semântico entre releases;
- aprovação de duas pessoas para alteração de preço;
- exportar os dois formatos quando necessário, nunca converter por offset heurístico sem fixture;
- publicação em janela com backup/rollback e verificação in-game;
- flag de manutenção da loja se os arquivos divergirem de hash.

### 5.3 Caminho de gold no código real

Em `gamedbd/addcash.hpp`, `AddCash::Process` abre o storage `user`, carrega `User` em `CommonTransaction`, verifica `user.add_serial < sn`, calcula disponibilidade, atualiza `user.cash_add`, serial, VIP e logs, e emite `AddCash_Re`. Isso indica **mecanismo de serial para evitar reprocessamento**; ainda é necessário provar semântica end-to-end e limites de inteiros. Outros handlers `getaddcashsn.hrp`, `cashserial.hrp`, `getcashtotal.hrp`, `useraddcash` e `instantaddcash.hrp` sugerem fluxos alternativos. Alguns trechos possuem `TODO` e fluxo de billing externo não conectado.

Existe `debugaddcash.hpp` que altera saldo sem o mesmo fluxo de cobrança: **não utilizá-lo como API comercial; bloquear em produção**.

### 5.4 Ledger e concessão segura — protocolo recomendado

```text
1. Jogador cria pedido de gold na API
2. API registra order=PENDING + ledger entry (sem crédito)
3. PSP recebe checkout (nenhum dado de cartão passa pelo jogo)
4. Webhook autenticado/replay-safe marca PAID
5. Worker serializa GrantCash(userId, zoneId, orderId)
6. Reserva serial único monotônico por (userId, zoneId)
7. Adapter interno chama AddCash (não DebugAddCash)
8. Consulta confirmação no gamedbd e concilia valor
9. Ledger registra CREDITED (ou INVESTIGATE, jamais "assumido")
10. Portal apresenta estado final verificável
```

Regras:
- `payment_order_id`, `provider_event_id`, `cash_grant_id` e `add_serial` distintos, com índices de unicidade.
- Saldos em inteiros com unidade documentada; definir conversão de R$ → gold em versionamento de oferta; evitar float.
- O código mostra campos `cash_add`, `cash_buy`, `cash_used`, `cash_sell`, `cash_sysauction`; a noção de “saldo” pode depender também de `cash`, `cash_2` e `cash_used_2` no leilão; não simplificar sem mapear todos os estados.
- Webhook duplicado não concede duas vezes. Timeout não significa falha: consultar `AddCash`/WDB antes de repetir.
- Erro de conciliação congela novas concessões dessa conta e gera incidente, **não** adiciona gold à força.
- Refund e chargeback geram eventos financeiros e política específica; jamais retirar gold de modo que produza saldo negativo/inconsistente ou cause corrupção.
- Gold gratuito/GM exige permissão especial, segundo aprovador, limites, rate-limit e justificativa.
- Operação financeira e economia do jogo devem sobreviver a queda do painel/realm.
- `shoplog`, `translog` e logs de `addcash` são fontes para conciliação, não única auditoria do PSP.

### 5.5 Administração da economia

Dashboard: ouro emitido, resgatado e em circulação; fluxo 24h/7d; top grants, top itens, cash por origem, pedidos pendentes, duplicidades, reversões, fraude, discrepância `ledger↔WDB`, VIP, shop price versions, leilões e webtrade.

Ações: pausar loja, criar campanha, emitir voucher, creditar promocional **com política e auditoria**, revisar item, abrir investigação, reconciliar período, reprocessar de forma idempotente, exportar relatório. **Nunca expor `DebugAddCash` e não permitir SQL/WDB livre.**

---

## 6. Realms, zonas, mapas, instâncias e ciclo de vida

### 6.1 Definições — não confundir conceitos

- **Realm:** agrupamento lógico que jogadores enxergam como servidor de jogo (pode ter várias zonas).
- **Zone (`zoneid`):** unidade lógica usada por delivery/DB/auth; mapear escopo de conta e personagem.
- **World/map:** conteúdo espacial ou tipo de mundo definido em `gs.conf`, frequentemente indexado por `worldtag`.
- **Game server (`gs`):** processo que executa mundo/instância; tem `gameid`/`index` e anuncia provider.
- **Instância:** mundo limitado ou plano paralelo independente, com `PID` e ciclo de vida.
- **Shard:** divisão de capacidade [FUTURO]; não existe só por declarar dois PIDs.

### 6.2 O que existe hoje

`gs-server153.sh` executa:

```bash
./gs-ours gs01 gs.conf gmserver.conf gsalias.conf is61
```

`cgame/gs/start.cpp` carrega dados globais antes do fork e executa **double-fork** para cada instância. O pai encerra; `gs01` e `is61` sobrevivem como processos do mundo. `gs.conf` contém centenas de seções `World_*`, `Instance_*`, `Terrain_*` e `MsgReceiver*`, mas um mapa configurado **não está automaticamente online**.

Instâncias registradas documentadas:

| Nome | Tipo | `gameid/index` | `worldtag` | Situação |
| --- | --- | --- | --- | --- |
| `gs01` | mundo aberto | 1 | 1 | sobe no script atual |
| `is61` | mundo paralelo inicial | 101 | 161 | sobe no script atual; spawn do personagem novo |
| `is62` | outra instância definida | 102 | 162 | definida no conf; não incluída no comando atual |
| `is42` | instância definida | 82 | 142 | definida no conf; não incluída no comando atual |
| demais `World_*` / `Instance_*` | depende da seção | variável | variável | inventariar; **não presumir executável** |

`is61` usa `parallelworld_server=1`, `player_per_instance=1000`, `instance_capacity=10`, thresholds de pool e autoalocação de planos. Estes valores configurados **não provam que atende 10 mil jogadores**; precisam ser validados pela memória real, latência e modelo de isolamento.

### 6.3 Como o mundo é escolhido

`gs` anuncia `AnnounceProviderID` com `gameid/index`, limites de região, `worldtag` e edition. `gdeliveryd` registra isso em `gameservermap`; `FindGameServer(point, worldtag)` escolhe provider. O template de classe/spawn vem de `clsconfig` no `gamedbd`. Mudar worldtag ou matar instância incorretamente faz o personagem entrar com `result=31/gameid=-1` ou ficar em estado de transição.

### 6.4 Catálogo administrativo de mapas [PROPOSTO]

Cada entrada deve exibir:

```yaml
map_key: is61                 # identificador simbólico
realm_id: realm153
zone_id: 1
worldtag: 161
game_id: 101
kind: parallel_world
base_path: a61/
config_revision: sha256:...
data_revision: sha256:...
desired_state: RUNNING        # STOPPED/RUNNING/DRAINING/MAINTENANCE
observed_state: HEALTHY       # UNKNOWN/STARTING/READY/DEGRADED/FAILED
runtime:
  pid: 1234                    # exemplo de schema; NÃO medição real
  start_time: null
  host_id: host-a
  rss_bytes: null
  cpu_pct: null
  players_online: null
  last_provider_announce: null
  last_heartbeat: null
limits:
  player_per_instance: 1000
  instance_capacity: 10
  pool_threshold_low: 1
  pool_threshold_high: 10
controls:
  can_hot_start: false         # só true depois de teste de implementação
  can_safe_stop: false
  can_scale: false
```

As flags `can_*` não devem ser habilitadas antes de testes; estado desejado e observado precisam ser reconciliados, não presumidos.

### 6.5 Operação “iniciar mapa”

1. Validar autorização `world.start`, realm e janela.
2. Carregar `gs.conf` validado, dependencies, `worldtag` e `gameid` exclusivos.
3. Confirmar arquivos de mapa, colisão, `npcgen`, navmesh, templates e hashes do cliente.
4. Verificar se já há um provider ativo com mesmo ID/tag e se qualquer filho órfão ainda roda.
5. Reservar recursos; exibir CPU/RAM esperados **estimados por baseline medido**.
6. Iniciar instância com supervisor process-aware; **não alterar lista fixa com `sed` e reiniciar tudo por padrão**.
7. Aguardar socket, anúncio provider e prova semântica de acesso do `gdeliveryd`.
8. Marcar `READY` só após conexão e leitura de player count.
9. Falha → parar apenas processo novo, limpar registro stale, liberar reserva e gerar log de operação.

**[A IMPLEMENTAR]** o código original faz fork de conjunto no boot; um launcher de mapa isolado precisa ser prototipado com o contrato `gs` antes de prometer hot-start. Pode exigir um bootstrap do GS separado ou supervisor no código C++.

### 6.6 Operação “desligar mapa” e drenagem

O painel deve ter **STOP IMEDIATO DESABILITADO POR PADRÃO**. Processo proposto:
1. Marcar `DRAINING`; proibir novas entradas/teleports para o mapa.
2. Informar jogadores e mostrar contagem de presentes.
3. Remover ou bloquear novas reservas no `gdeliveryd` sem perder provider existente durante saída.
4. Solicitar movimentação segura dos jogadores a mundo permitido **ou logout persistido**, conforme API comprovada; nunca teletransportar à força sem teste do protocolo.
5. Confirmar `players_online = 0`, operações DB em curso concluídas e checkpoint/flush.
6. Encerrar instância pelo mecanismo gracioso testado; aguardar e retirar provider.
7. Confirmar no `gdeliveryd` que `worldtag` não resolve mais para um PID morto.
8. Só então `STOPPED`; rollback/restart controlado se timeout.

**Se não houver mecanismo seguro de drain em `gs`/delivery, a opção deve ser “maintenance + restart total em janela”** até ele ser implementado. Um botão “kill map” em produção antes dessa fase é fonte de perda de progresso.

### 6.7 Restrições de sistema

- Controlar processo por **PID + starttime + pid namespace + realm_id + instance_id**, não por `comm`.
- Não permitir duplicação de `gameid`/`worldtag`; `gfactiond` ainda tem risco registrado de rejeição de ID duplicado.
- Reinícios isolados de `gdeliveryd`/providers podem romper links e causar “servidor suspenso”; suportar operação coordenada até existir reconnect robusto.
- Acompanhar `status` de jogadores e reentrada, mensagens pendentes, despawn de NPC, instâncias paralelas e relógio de eventos.
- `watchdog-server153.sh` considera o stack ativo quando existe algum `gs-ours`; isso **não certifica que todos os mapas** estão operacionais.

---

## 7. Matriz de configurações que o painel poderá controlar

**Regra:** para cada chave registrar `source_file`, `section`, `key`, `type`, default comprovado, faixa válida, dependencies, `effective_value`, impacto, classificação `HOT/RESTART/REBUILD/DATA-MIGRATION`, aprovação, teste e procedimento de rollback. O conjunto abaixo **não afirma suporte hot-reload**.

### 7.1 Catálogo por domínio

| Domínio | Exemplos de parâmetros detectados | Impacto e estratégia |
| --- | --- | --- |
| Rede/links | `address`, `port`, `max_users`, `max_conns`, `halflogin_users`, `listen_backlog`, buffers TCP, `tcp_nodelay`, `compress` | restart do daemon/gateway, coordenado |
| Identidade do realm | `zoneid`, `aid`, `zondname`, `accepted_zone_list`, `cross_zone_pos`, `version`/`edition` | **crítico**; validação de protocolo e dados, maintenance |
| Pools | `[ThreadPool].threads`, `max_queuesize`, `prior_strict`, `logic_threads` | restart, testes de concorrência |
| WDB/BDB | `tables`, `homedir`, `datadir`, `logdir`, `backupdir`, `cachesize`, `checkpoint_interval`, `backup_interval`, `times_incbackup`, `cache_high_default`/`cache_low_default` | alteração com snapshot/backup, nunca ao vivo sem suporte |
| Login/usuários | `freecreatime`, `checkforbidlogin_interval`, `delrole_interval`, `challenge_algo` | classificar por código e testar, risco de compatibilidade |
| Recompensas/exp | `[DoubleExp].exp`, `sp`, `item`, `money`, `task_exp`, `task_sp`, `task_money`, `[ConsumeReward]` | mudança de regras: checar broadcast/GMSetGameAttri |
| Mundo | `world_servers`, `instance_servers`, `[World_*]`, `[Instance_*]`, `index`, `tag`, `base_path` | exige verificador worldtag/asset e ciclo de vida |
| Capacidade | `player_capacity`, `player_per_instance`, `instance_capacity`, `pool_threshold_*`, `max_sight_range`, `npc_count`, `matter_count` | benchmark + load test antes de aumentar |
| Geografia | `grid`, `base_region`, `local_region`, `save_point`, `height_limit`, `terrain`, `mapres_type` | pode corromper spawn/colisão; versionar mapa |
| Anti-wallow | `AntiWallow`, `WallowLight`, `WallowHeavy`, `WallowTime` | política de contas/idade, testar legalidade e gameplay |
| Loot/quests | `itemDataFile`, `QuestPackage`, `GlobalData`, `PolicyData`, `DropData`, `NPCGenFile` | editar arquivo data versionado, deploy e restart |
| Loja/mall | `MallData`, `Mall2Data`, `RandomShopLimit`, `gshopsev*.data`, CASHVIP | compatibilidade de binário, revisão dual |
| Economia | `PRV_DOUBLEMONEY`, `PRV_DOUBLEOBJECT`, `PRV_DOUBLEEXP`, `PRV_DOUBLESP`, `PRV_NOSELLPOINT` | alguns atributos têm caminho GM RPC; testar significado real |
| Anticheat | `gacd.xml`, políticas de eventos/punições, protocolos de reload | tornar reversível e permissionado, nunca expor regras detalhadas |
| Filtros/encoding | `table_charset`, `name_charset`, `filters` | testes GBK/UCS2, riscos de nomes/chat |
| Comunicação interna | `GProviderClientN`, `GameDBClient`, `LogclientClient`, `AddrAlias` | rewire de serviços, janelas e health |
| Scripting e tarefas | `global_api.lua`, `tasks.data`, `dyn_tasks.data` | data release compatível, não edição arbitrária de Lua pela UI |
| Eventos | arena, battleground, countrybattle, faction fortress, rankings | mapear configs e jobs específicos antes de habilitar |

### 7.2 Superfície encontrada em `gs.conf`

O arquivo rastreado contém **316 seções**. Entre as famílias encontradas estão `General`, `WallowLight`, `WallowHeavy`, `WallowTime`, `DoubleExp`, `MoveMap`, `Template`, `MsgTCPSession`, `MsgUNIXSession`, múltiplos `World_*`, `Terrain_*`, `MsgReceiverTCP_*`, `MsgReceiverUNIX_*`, `Instance_*`. Identificadores como `player_capacity`, `player_per_instance`, `instance_capacity`, `pool_threshold_low`, `pool_threshold_high`, `parallelworld_server`, `faction_server`, `battleground_server`, `countrybattle_server`, `life_time` e `idle_time` estão presentes.

**Spec de catalogação [PROPOSTO]:** construir um gerador de schema que percorre todos os arquivos conf rastreados, identifica seções/chaves e referências de leitura `Conf::GetInstance()->find(...)` no código, elimina comentários, registra formatos de arrays/tuplas e cria `config-catalog.json`. **O parser não deve aceitar alterações de chaves desconhecidas** sem aprovação do schema correspondente.

### 7.3 Exemplo de revisão de configuração

```json
{
  "revision_id": "cfg_20261028_001",
  "scope": {"realm_id": "realm153", "zone_id": 1},
  "base_revision": "sha256:baseline",
  "changes": [
    {"path": "gdeliveryd.ThreadPool.threads",
     "from": "(1,3)(100,1)(101,4)(0,1)",
     "to": "(1,4)(100,1)(101,4)(0,1)",
     "apply_mode": "COORDINATED_RESTART"}
  ],
  "status": "DRAFT",
  "validation": {"schema": "PENDING", "compatibility": "PENDING", "load_test": "PENDING"},
  "approvals_required": 2
}
```

Os valores de exemplo são uma proposta, **não recomendação para mudar workers agora**. Uma revisão só vira aplicada quando a API confirma versão, restart, health e rollback.

---

## 8. Administração existente no protocolo e GAP de integração

### 8.1 APIs/RPCs/protocolos encontrados

| Função | Fonte real | Integração |
| --- | --- | --- |
| Jogadores online | `gdeliveryd/gmonlinenum.hpp` → `UserContainer::Size()` | adaptador administrativo read-only |
| Listar jogadores online | `gdeliveryd/gmlistonlineuser.hpp` → `PartlyWalkUser` com cursor | paginação segura, autorização, rate-limit |
| Reinício programado | `gdeliveryd/gmrestartserver.hpp` | broadcast/forbid login, **não confundir com shutdown de host seguro** |
| Alterar atributo global | `gdeliveryd/gmsetgameattri.hrp` | `GMSetGameAttri`, checar privilege, enum e propagação |
| Kick | `gmkickoutuser.hpp` / `gmkickoutrole.hpp` | ação moderada e auditada |
| Silenciar | `gmshutup.hpp` / `gmshutuprole.hpp` | motivo, expiração e reversão |
| Ban role | `gmforbidrole.hpp` | política, expiração, sincronização |
| Bloqueio de vendas | `gmforbidsellpoint.hpp` | validação de impacto |
| Alternar chat | `gmtogglechat.hpp` | mudar atributo com dupla confirmação em realm |
| Info de consumo | `gmgetplayerconsumeinfo.hpp` | investigação econômica |
| Concessão de cash | `gamedbd/addcash.hpp` | RPC transacional apenas, nunca endpoint público |
| Anticheat | `gacd`: `ACQuestion`/`ACAnswer`, `ACKickoutUser`, `ACReloadConfig` etc. | análise defensiva e controle interno |
| DB offline | `gamedbd.cpp` | ferramentas segregadas, exclusivas e com backup |

**Importante:** arquivos `.hpp`/`.hrp` serem implementados **não provam que uma sessão administrativa externa consegue chamá-los**. Os state arrays de GNET, permissões de GM, autenticação de serviço e os parâmetros de cada mensagem devem ser verificados antes de disponibilizar o botão. Não utilizar credenciais do jogo para autorizar a API de control plane.

### 8.2 Contrato de backend para comandos

```typescript
type Actor = {
  identityId: string;
  adminRoleIds: string[];
  scopes: Array<{realmId: string; zoneId?: number; instanceId?: string}>;
  mfaLevel: "none" | "verified" | "step_up";
};

type OperationRequest = {
  type: "PLAYER_KICK" | "ROLE_MUTE" | "REALM_RESTART" |
        "WORLD_START" | "WORLD_DRAIN" | "CONFIG_APPLY" |
        "CASH_GRANT" | "BACKUP_CREATE" | "RESTORE_EXECUTE";
  target: {realmId: string; zoneId?: number; roleId?: number; instanceId?: string};
  reason: string;
  idempotencyKey: string;
  expectedRevision?: string;
  dryRun: boolean;
};

type OperationStatus =
  | "REQUESTED" | "VALIDATING" | "WAITING_APPROVAL" | "QUEUED"
  | "RUNNING" | "VERIFYING" | "SUCCEEDED"
  | "FAILED" | "ROLLING_BACK" | "ROLLED_BACK" | "MANUAL_INTERVENTION";

type OperationResult = {
  operationId: string;
  status: OperationStatus;
  createdAt: string;
  lastUpdatedAt: string;
  progress?: number;
  warnings: string[];
  errorCode?: string;
  evidenceRefs: string[];
};
```

Todos os comandos mutantes exigem idempotência, escopo, permissões, razão, limite e audit trail. Operações que podem causar perda de dados ou movimentação de dinheiro exigem `dryRun`, aprovação e prova de backup.

---

## 9. Arquitetura proposta: módulos desacoplados, sem replicação de regras

### 9.1 Visão de componentes

```text
                         Internet / browser
               ┌───────────────────────────────────┐
               │ player-portal / admin-console      │
               └─────────────────┬─────────────────┘
                                 │ HTTPS + MFA
                         ┌───────▼─────────┐
                         │ control-api     │
                         │ auth/RBAC       │
                         │ typed commands  │
                         │ audit/jobs      │
                         └──┬─────┬────────┘
               PostgreSQL   │     │ job queue / events
             (control DB) ◄─┘     ▼
                            [realm-controller]
                                │ mutual auth
                      ┌─────────▼──────────┐
                      │ realm-agent        │
                      │ constrained APIs   │
                      │ health/metrics     │
                      │ lifecycle          │
                      │ GNET admin adapter │
                      └─────┬────────┬─────┘
                            │        │
                      [GNET RPC] [host supervisor]
                            │        │
                  PW daemons/WDB   gs/map processes
```

O portal pode rodar fora do servidor; o `realm-agent` fica **na rede privada do host onde o jogo roda**, com credenciais de serviço limitadas e API não pública. Proposta compatível com um deployment inicial em VM Docker com persistência; migrar para containers coordenados somente após validar ciclos de vida e storage.

### 9.2 Organização proposta no monorepo

```text
apps/
  player-portal/         # páginas públicas e privadas do jogador
  admin-console/         # centro administrativo
services/
  control-api/           # identidade, RBAC, API, audit, jobs
  realm-controller/      # reconciler, config revisions, operação
  realm-agent/           # listener interno com privilégios mínimos
  game-auth-adapter/     # ponte de credenciais para gauthd
  game-query-adapter/    # consultas GNET tipadas e snapshots
  economy-worker/        # PSP webhook / ledger / RPC AddCash
packages/
  panel-types/           # schemas e types versionados
  panel-sdk/             # cliente da API pública e eventos
  panel-protocol/        # tipos/serialização da ponte GNET
  config-catalog/        # parser e schema de configurações
infra/
  panel/                 # migrations, compose, manifests, runbooks
docs/
  PAINEL.md
src/server153/            # servidor original adaptado — preserve limites
web/                      # cliente/browser existente — não duplicar
```

**É uma proposta, não pastas criadas.** Adequar a convenções de monorepo que surgirem; preservar a separação já existente entre `web/` do jogo e UI de painel. Não duplicar implementações de autenticação, autorização, RPC nem validação de schema entre serviços.

### 9.3 Responsabilidade única de cada módulo

- `control-api` possui identities, sessões, RBAC, requisições, audit e API externa; **não** inicia processos com shell.
- `realm-controller` possui estado desejado, dependências, orquestração, transições, rollback e aprovação; **não** persiste personagens.
- `realm-agent` observa e executa ação local permitida com manifest e assinatura; **não** decide RBAC global sozinho, mas faz validação independente da identidade de serviço e escopo.
- `game-auth-adapter` implementa credenciais do game protocol; **não** guarda saldo de gold.
- `game-query-adapter` traduz requests de leitura/GM para GNET sem expor framing bruto.
- `economy-worker` possui entrega financeira com ledger, serial, confirmação e conciliação; **não** executa `DebugAddCash`.
- `panel-types` centraliza nomes, schemas, statuses e eventos; nenhuma equipe cria um tipo divergente em seu frontend.

### 9.4 Comunicação e resiliência

- UI→API: HTTPS JSON OpenAPI, autenticação, CSP, input validation, rate-limit.
- API→jobs: fila persistente (Postgres outbox ou fila gerenciada) e dispatcher idempotente.
- controller→agent: HTTP/gRPC mTLS em rede privada, nonce, deadline, scoped service token; **não via portas internas públicas**.
- agent→PW: RPCs protocolados/localhost e leituras do SO; nenhuma escrita direta de WDB.
- telemetry: métricas agregadas em Prometheus/OpenTelemetry quando aplicável; logs correlacionados por `operation_id`.
- Nenhuma indisponibilidade de `control-api` pode impedir que `glinkd`/`gdeliveryd` processem jogadores já autenticados. Para auth o adapter deve ter timeout e estratégia explícita sem `fail-open`.

---

## 10. API de portal: rotas propostas (v1)

Todos os endpoints são **[PROPOSTOS]**, não implementados. JSON válido, request IDs, versionamento `/v1`, ETag/paginação, validation e erros estruturados.

| Método e rota | Comportamento |
| --- | --- |
| `POST /v1/identities/register` | início do cadastro com verificação |
| `POST /v1/identities/verify` | confirmar token/verificação |
| `POST /v1/sessions` | login web |
| `DELETE /v1/sessions/current` | encerrar sessão |
| `GET /v1/sessions` / `DELETE /v1/sessions/{id}` | listar/revogar sessões |
| `POST /v1/password/recovery` | gerar fluxo de recuperação |
| `POST /v1/password/reset` | definir senha nova |
| `POST /v1/mfa/enroll` / `POST /v1/mfa/verify` | configurar MFA |
| `GET /v1/realms` | discovery público com status sanitizado |
| `GET /v1/realms/{id}/status` | online/offline/maintenance e população aproximada |
| `GET /v1/me` | perfil e conta |
| `GET /v1/me/game-accounts` | contas e zones |
| `POST /v1/me/game-accounts` | provisionamento real e assíncrono |
| `GET /v1/me/characters?realm_id=...` | personagens por escopo, read-only |
| `GET /v1/me/characters/{roleId}` | detalhes permitidos, com ownership check |
| `GET /v1/me/wallet` | gold confirmado/pendente, unidade explícita |
| `GET /v1/me/transactions` | histórico de cobranças/concessões |
| `GET /v1/store/catalog` | catálogo público de ofertas |
| `POST /v1/store/orders` | iniciar pedido com idempotency key |
| `GET /v1/store/orders/{id}` | estado validado no backend |
| `POST /v1/payments/webhooks/{provider}` | autenticar assinatura + replay protection; rota sem sessão de jogador |
| `GET /v1/me/tickets` / `POST /v1/me/tickets` | suporte |
| `GET /v1/news` / `GET /v1/events` | comunicação |

### 10.1 Exemplos de resposta

```json
{
  "realm_id": "realm153",
  "availability": "MAINTENANCE",
  "auth_available": false,
  "gameplay_available": false,
  "population": {"value": null, "confidence": "UNKNOWN"},
  "last_observed_at": "2026-10-28T00:00:00Z",
  "stale": true
}
```

**Nunca confundir `tcp_connect_ok` com `gameplay_available`.** O `/health` estático do container retorna 200 mesmo sem garantir `gs`; portal não deve prometer “jogável” baseado nele.

---

## 11. API administrativa: endpoints e granularidade

Requerer `admin`, MFA, escopo e audit. Para operações de risco, devolver `202 Accepted` e `operation_id`. Não executar sincronamente em request HTTP.

| Método e rota | Uso e pré-condição |
| --- | --- |
| `GET /v1/admin/overview` | consolidação de realmes, alertas e capacidade |
| `GET /v1/admin/realms` | listar e filtrar |
| `GET /v1/admin/realms/{id}/topology` | árvore de serviços/mapas e conexões |
| `GET /v1/admin/services` | estado desejado/observado de daemons e instâncias |
| `GET /v1/admin/services/{id}/health` | readiness, process, socket e dependency status |
| `GET /v1/admin/services/{id}/metrics` | CPU/RAM/FD/latência/filas |
| `GET /v1/admin/services/{id}/logs` | logs sanitizados e filtrados |
| `POST /v1/admin/services/{id}/operations/restart` | restart coordenado; dry run e aprovação |
| `GET /v1/admin/worlds` | mapa, worldtag, PID, população e estado |
| `GET /v1/admin/worlds/{id}` | dados de instância e histórico |
| `POST /v1/admin/worlds/{id}/operations/start` | valida dependências, versão e recurso |
| `POST /v1/admin/worlds/{id}/operations/drain` | bloquear novas entradas, esvaziar |
| `POST /v1/admin/worlds/{id}/operations/stop` | somente após drain comprovado |
| `POST /v1/admin/worlds/{id}/operations/restart` | plano, backup quando necessário |
| `GET /v1/admin/players/online` | cursor de `GMListOnlineUser` com RBAC |
| `GET /v1/admin/accounts` / `GET /v1/admin/accounts/{id}` | perfil e estado de auth |
| `GET /v1/admin/characters` / `GET /v1/admin/characters/{id}` | consulta do WDB pela ponte |
| `POST /v1/admin/players/{id}/operations/kick` | motivo, permissão e retorno |
| `POST /v1/admin/characters/{id}/operations/mute` | duração e justificativa |
| `POST /v1/admin/characters/{id}/operations/ban` | sanção verificada |
| `POST /v1/admin/characters/{id}/operations/teleport` | somente após API suportar, posição segura |
| `GET /v1/admin/config/catalog` | schema de chaves do runtime |
| `GET /v1/admin/config/revisions` | revisão, diff, status |
| `POST /v1/admin/config/revisions` | criar draft/validar |
| `POST /v1/admin/config/revisions/{id}/validate` | sintaxe + compat + rollout |
| `POST /v1/admin/config/revisions/{id}/approve` | segundo aprovador conforme regra |
| `POST /v1/admin/config/revisions/{id}/apply` | job coordenado |
| `POST /v1/admin/config/revisions/{id}/rollback` | revisão anterior |
| `GET /v1/admin/economy/ledger` | gold e saldo conciliado |
| `GET /v1/admin/economy/cash-grants` | concessões e divergências |
| `POST /v1/admin/economy/cash-grants` | concessão manual com limites e aprovação |
| `GET /v1/admin/store/catalog-revisions` | diff de itens/loja |
| `POST /v1/admin/store/catalog-revisions` | draft de loja |
| `POST /v1/admin/store/catalog-revisions/{id}/publish` | export e deploy protegido |
| `GET /v1/admin/backups` | lista de snapshots |
| `POST /v1/admin/backups` | criar backup consistente |
| `POST /v1/admin/restores/plan` | preview isolado de restore |
| `POST /v1/admin/restores` | restore com MFA/approvals/maintenance |
| `GET /v1/admin/operations` / `GET /v1/admin/operations/{id}` | progresso e evidência |
| `GET /v1/admin/audit` | audit log com permissão |
| `GET /v1/admin/incidents` | incidentes |
| `POST /v1/admin/maintenance-windows` | agendar maintenance com avisos |
| `GET /v1/admin/roles` / `POST /v1/admin/role-grants` | IAM de console |
| `GET /v1/admin/diagnostics/versions` | binários, dados, config, checksums |
| `GET /v1/admin/diagnostics/compatibility` | matriz cliente/servidor e status de testes |

### 11.1 Erros padronizados

```json
{
  "error": {
    "code": "WORLD_NOT_DRAINED",
    "message": "Ainda existem jogadores conectados a esta instância.",
    "retryable": false,
    "operation_id": "op_123",
    "details": {"players_online": 2}
  },
  "request_id": "req_456"
}
```

Códigos a prever: `UNAUTHORIZED`, `MFA_REQUIRED`, `SCOPE_DENIED`, `STALE_OBSERVATION`, `DEPENDENCY_UNHEALTHY`, `WORLD_NOT_DRAINED`, `WORLD_PROVIDER_COLLISION`, `CONFIG_INVALID`, `CONFIG_REVISION_CONFLICT`, `AUTH_ADAPTER_UNAVAILABLE`, `GAME_DB_BUSY`, `CASH_GRANT_DUPLICATE`, `CASH_BALANCE_UNCONFIRMED`, `BACKUP_REQUIRED`, `RESTART_REQUIRES_MAINTENANCE`, `OPERATION_TIMED_OUT`.

---

## 12. Console de administração: telas completas

### 12.1 Hierarquia de navegação proposta

```text
Dashboard
├── Saúde e incidentes
│   ├── Visão global de realms
│   ├── Daemons e dependências
│   ├── Métricas em tempo real
│   ├── Logs, traces e exceções
│   └── Alarmes / incidentes / manutenção
├── Infraestrutura
│   ├── Hosts / containers / imagens
│   ├── Deploys e revisões
│   ├── Serviços: start/stop/restart seguro
│   ├── Topologia GNET e listeners
│   ├── Mapas / instâncias / providers
│   ├── Configurações e diffs
│   └── Backups / restauração
├── Jogadores
│   ├── Contas e autenticação
│   ├── Personagens
│   ├── Jogadores online
│   ├── Inventário e armazém (consulta)
│   ├── Missões / progressão (consulta)
│   ├── Clãs / facções
│   ├── Moderação / denúncias
│   └── Suporte
├── Economia
│   ├── Saldos e ledger
│   ├── Gold / concessões / cobranças
│   ├── Loja / editor de catálogo
│   ├── Cupons / campanhas / VIP
│   ├── Leilão / lojas de jogadores
│   └── Auditoria e reconciliação
├── Gameplay
│   ├── Configurações do mundo
│   ├── Multiplicadores e eventos
│   ├── NPCs / spawns / drops
│   ├── Quests / skill policies
│   ├── Guerra territorial / arena
│   └── Agenda e anúncios
└── Administração
    ├── Equipe / papéis / permissões
    ├── Aprovações pendentes
    ├── Trilhas de auditoria
    ├── Integrações e segredos (somente referências)
    ├── Relatórios e exportações
    └── Feature flags / documentação
```

### 12.2 Dashboard de saúde — conteúdo mínimo

- Banner por realm: `ONLINE` apenas se auth, delivery, gamedbd, glinkd, providers essenciais e probes de **gameplay** estiverem prontos; senão `DEGRADED/MAINTENANCE/OFFLINE/UNKNOWN`.
- Gráfico temporal: jogadores online, CPU/memória do `gs`, login success/error, RTT de game, interações DB, process restarts, fila prioritária e incidentes.
- Lista de daemons com PID, uptime, versão, últimas cinco falhas, sockets abertos, fila de trabalho, logs correlacionados e ações de operação.
- Topologia com links vivos/degradados e timestamps de provider announce.
- Mapas: `worldtag`/`gameid`, players, capacidade, tipo, host, pressão de memória, botão de detalhes.
- Alerts priorizados: WDB sem checkpoint, replay de billing, auth indisponível, realm sem `gs`, perda de provider, restart storm, uso de dev-auth, dados do cliente incompatíveis.
- Estado de backups, tempo desde último teste de restauração, retenção e alertas de tamanho.

### 12.3 Lista de jogadores online e perfis

Filtros por realm, zone, worldtag, classe, level, status e último heartbeat; tabelas paginadas com roleid, userid, nome, classe, mapa, instância, login e sessão. Abrir detalhe para inventário (read-only), trilha de moderação, reclamações, histórico de compras conciliado e ações GM permitidas. **Dados privados do jogador devem aparecer só conforme necessidade e perfil de acesso.** Não expor IP completo nem senha.

### 12.4 Editor de arquivos e dados

- Painel baseado em schema, não editor hex/SQL arbitrário.
- Mostrar provenance, hash, tamanho, encoding, versão (v145), resumo dos registros, erros de parsing e preview.
- Editor especializado de `elements.data`, `tasks.data`, `gshop*.data`, `gshopsev*.data`, `npcgen.data`, `aipolicy.data`, `gs.conf`.
- Para ferramentas externas, integrar como import/export **offline e autenticado**; não montar interfaces que editam dados binários em uso.
- Cada mudança passa por `DRAFT → VALIDATED → APPROVED → STAGED → APPLIED → VERIFIED`, com rollback materializável.

---

## 13. Saúde do servidor, métricas e observabilidade

### 13.1 Readiness correta versus mero processo vivo

Definir cinco estados diferentes:

- **process_alive**: PID correspondente e não zombie;
- **socket_ready**: a porta está realmente escutando no namespace certo;
- **protocol_ready**: protocolo GNET é aceito e resposta semanticamente válida;
- **dependency_ready**: todas as conexões obrigatórias e provider announces ativos;
- **gameplay_ready**: login, RoleList, EnterWorld em mapa válido e persistência de movimento provados por teste controlado.

Um endpoint `/health` com 200 estático **não prova** `gameplay_ready`. O CGI atual conta oito daemons, não `gs`; `/tcpprobe` prova conexão TCP de mediação, não login.

### 13.2 Métricas por família

| Família | Métricas propostas | Fonte / método |
| --- | --- | --- |
| Linux | CPU por PID/instância, RSS/PSS, cgroup memory, swap, FD, I/O, thread count, OOM, restarts | `/proc`, cgroups, supervisor |
| Rede | bytes, packets, conexiones TCP, retransmissões, RTT, backpressure, queue, disconnect reason | socket agent + GNET instrumentado |
| Autenticação | throughput, sucesso, erro por retcode, p50/p95/p99, rate-limit, sessão duplicada | `gauthd` e `glinkd` |
| Entrega | size de `UserContainer`, filas por prioridade, lock waits, provider registrations, RPC latency | instrumentar `gdeliveryd` |
| Banco | latência de `StorageEnv`, tamanho de cache, checkpoints, backup status, erro CORRUPT, inserts/reads | instrumentar `gamedbd` |
| Worlds | tick duration, players/plane, RPC inbound/outbound, NPC count, entity count, spawn/despawn, physics and AI cost | instrumentar `gs` e `parallel_world_manager` |
| Economia | grants aprovados, confirmação WDB, duplicados, discrepâncias, itens vendidos, chargebacks | ledger + logs |
| Qualidade | login→mundo, crash-free sessions, disconnect % e stall | probes sintéticos + client telemetry |

Não usar “número de processos = servidor saudável”. Não derivar taxa de atividade por regex de log como fonte exclusiva; logs GBK podem atrasar ou ser perdidos.

### 13.3 Semântica de status e frescor

```typescript
type Health = {
  state: "HEALTHY" | "DEGRADED" | "UNHEALTHY" | "UNKNOWN" | "MAINTENANCE";
  observedAt: string;
  ttlSeconds: number;
  source: "PROCESS" | "SOCKET" | "GNET" | "APP_PROBE" | "OPERATOR";
  reasonCodes: string[];
  dependencies: Array<{name: string; state: string}>;
  evidenceRef?: string;
};
```

Quando `observed_at + ttl < now`, promover para `UNKNOWN`, **não manter “online” congelado**.

### 13.4 Alertas e runbooks

- `auth_login_fail_rate` alta → verificar dev-auth removido, gauthd/DB, credential bridge e rate limiting.
- `provider_missing` → identificar worldtag, PID/gs, link, provider announce e crash.
- `wdb_checkpoint_stale` → suspender operações econômicas, capturar estado, proteger arquivos; nunca `--fresh`.
- `restart_loop` → desabilitar auto-recuperação cega, abrir incidente e exigir revisão de core.
- `world_tick_p99` alto → reduzir NPC AI/pool e perfilar com `perf`/amostrador; não culpar “internet”.
- `cash_grant_stuck` → congelar retry, conciliar `sn`/`add_serial` e valor em WDB.
- `disk_fill_rate` alto → rotação dos logs, backups fora do volume local e escalonamento.
- `version_mismatch` → recusar novos logins até resolver edition/asset hashes.

### 13.5 Logs e traces

- Logs do daemon em `dist/server153/runtime/logs`; `world2.log` via `logservice`, com encoding GBK.
- Incluir `realm_id`, `zoneid`, `roleid` quando permitido, `operation_id` e timestamp UTC; anonimizar e criptografar PII.
- Separar INFO/ERROR/SECURITY/AUDIT, retenção e DLQ; impedir que dados de senha, verifier e chaves saiam em traces.
- Persistir evidência de “ação executada” e “estado confirmado”; logs de tentativa sozinhos não comprovam sucesso.

---

## 14. Dimensionamento, consumo e eficiência

### 14.1 Números **realmente disponíveis**

| Objeto | Dado no repositório | Interpretação |
| --- | --- | --- |
| Árvore `src/server153` com build | ~520 MB | inclui .o, debug info, libs; não é RSS |
| Fonte sem build | ~141 MB | tamanho do source vendor |
| Pacote `tar.gz` de build | ~122–128 MB | distribuição comprimida |
| Binário `gs`/`gdeliveryd`/`libskill.so` com símbolos | ~33 / 37 / 34 MB | tamanho em disco; não é memória residente |
| Imagem de runtime Cloudflare | ~480 MB | tamanho de imagem, não consumo em memória |
| Container Cloudflare `lite` | 1/16 vCPU, 256 MiB, 2 GB disk relatados | recursos da classe, não prova de escala MMO |
| Oito daemons + httpd no Cloudflare | ~26 MiB medidos historicamente | **exclui o `gs` e mapa com assets** |
| `is61` config | `player_per_instance=1000`, `instance_capacity=10` | limites de config, **não capacidade comprovada** |

**Não há medida confiável de:** RSS/PSS por mapa, pico de CPU em tick, consumo para 10/100/1000 usuários, custo por NPC, DB WDB sob write load, tamanho real dos assets montados, cold-start completo do jogo, throughput máximo ou preço por jogador.

### 14.2 Plano de benchmarking obrigatório

Capturar baseline para:
1. stack sem `gs`;
2. `gs01` sozinho;
3. `gs01 + is61` (cada PID);
4. 1/10/50/100/250 jogadores controlados e com carga representativa;
5. cenário PvE com NPCs, movimentação, quests, skills e inventário;
6. cenário de PvP/guerras, itens, loja/gold e mudança de mapa;
7. failure injection: kill/timeout de cada daemon, disco cheio, reload de config, reconnect;
8. cold-start com download/mount de dados, cache frio/quente;
9. recuperação depois de shutdown não gracioso e verificação da integridade WDB.

Coletar média, p95, p99, máximo, desvio, consumo incremental `ΔRSS/player`, `ΔCPU/player` e `ΔRSS/map`; distinguir carga sintética e real. **Nenhuma previsão numérica de “suporta X mil” deve aparecer antes dos resultados.**

### 14.3 Decisão sobre Cloudflare

O atual Cloudflare Worker + Durable Object + Container **é uma prova do caminho de mediação**. `cloudflare/entrypoint.sh` sobe os oito daemons e busybox httpd, **não sobe `gs`, não monta pacote v145 de mapas em runtime nem comprova volume WDB persistente**. O timeout de inatividade configurado em `cloudflare/worker.js` é 30 minutos; **processo de MMO não pode ser desligado por inatividade HTTP ou cold start inesperado** quando há jogadores ou estado ativo.

Para a **primeira implantação jogável** recomendo VM ou host/container estável com volumes persistentes, supervisor, snapshots e proteção de rede; manter Cloudflare para UI/CDN/gateway, se ele cumprir latência e WSS. Retornar a Cloudflare Containers para os processos centrais apenas após provas de armazenamento, disponibilidade contínua, CPU, cold-start e comportamento de escala. **Não migrar WDB entre instâncias efêmeras sem protocolo de recuperação.**

---

## 15. Segurança crítica e produção

### 15.1 Achados prioritários

| Prioridade | Achado | Evidência | Mitigação |
| --- | --- | --- | --- |
| **P0 BLOQUEANTE** | `dev-auth` aceita usuários previsíveis com senha idêntica ao ID | `gauthd`, `panel-server153.py` | auth real, testes negativos e remoção de fallback |
| **P0 BLOQUEANTE** | chaves de sessão/shared secret **literais em scripts e confs rastreados** | `boot-server153.sh`, `cloudflare/entrypoint.sh` e upstream confs | **rotacionar todos os segredos antes de produção**, usar secret manager, revisar exposição histórica |
| **P0 BLOQUEANTE** | DB WDB pode ficar CORRUPT após shutdown sujo; `--fresh` destrói dados | `07-DATA-AND-STORAGE.md` | teste de recovery/restore + backups consistentes, sem wipes |
| P0 | painel local `127.0.0.1:8080` não implementa identidade/CSRF/mutação segura | `panel-server153.py` | limitar a dev ou retirar após API |
| P0 | `DebugAddCash` presente | `gamedbd/debugaddcash.hpp` | negar ou retirar capacidade em release de produção |
| P0 | Cloudflare health não prova jogo pronto | `cloudflare/entrypoint.sh` / `worker.js` | readiness semântica e gs obrigatório |
| P1 | dois geradores de configs de runtime podem divergir | `boot-server153.sh` / `cloudflare/entrypoint.sh` | template único por versão com schema |
| P1 | watchdog por nome do processo não garante cada mapa, nem faz backoff seguro | `watchdog-server153.sh` | supervisor instance-aware, backoff e alarmes |
| P1 | hot-restart de daemon isolado rompe dependências | `06-OPERATIONS.md` | operação coordenada e reconnect testado |
| P1 | comandos GM dependem de states/privilege e não são uma API externa | `glinkd/gdeliveryd` | bridge interna com RBAC |
| P1 | possível impacto econômico ao duplicar comando ou usar valor incorreto | `AddCash` e logs | ledger e verificação idempotente |

**Nunca publicar em documento valores das chaves** já presentes no repositório; sua existência foi auditada sem replicá-las aqui. Remover um segredo de arquivo atual não remove a exposição de histórico Git: criar plano de rotação e avaliação de revisão do histórico, sem reescrever branch compartilhada inadvertidamente.

### 15.2 Princípios de segurança do console

- Zero trust entre internet e agent; tudo via TLS, auth, mutual auth interna, allowlist e short-lived credentials.
- RBAC por `realm`/`zone`/`instance` e ABAC adicional por tipo de ação.
- Separação de funções: suporte não concede gold; GM não aprova próprio aumento; operador de infraestrutura não lê dados de pagamento sem necessidade.
- MFA e reautenticação para ban permanente, cash grant, restore, wipe, chave e deploy.
- Auditoria append-only inviolável, com retenção, export, cadeia de hash ou sink imutável.
- Rate limiting por identidade/IP/realm, antiautomação e detecção de abuso.
- Não confiar em parâmetros `roleid`/`userid` vindos de browser: verificar ownership e escopo.
- Resposta HTTP nunca devolve credencial GNET, hash, token de DO, dump de DB ou conteúdo de arquivo sensível.
- Notas e exportação conforme LGPD e política de retenção; acessos de staff rastreados.
- Testes SAST/DAST/secret scanning, dependências e fuzz da ponte GNET.
- Autorização de edição de arquivos do jogo e redistribuição deve permanecer conforme `docs/LEGAL.md`.

### 15.3 Matriz RBAC inicial

| Permissão | Player | Suporte | Moderador | GM | Operador | Financeiro | Superadmin |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Conta própria | sim | não | não | não | não | não | política |
| Tickets atribuídos | próprio | sim | sim | não | não | não | sim |
| Presença e status agregados | público limitado | sim | sim | sim | sim | sim limitado | sim |
| Lista de personagens | próprio | escopo de atendimento | escopo de moderação | escopo | no necessário | não | controlado |
| Kick/mute/ban | não | não | conforme grant | conforme grant | não | não | controlado |
| Start/stop mapa | não | não | não | não | aprovação/escopo | não | aprovação |
| Config game | não | não | não | proposta restrita | aprova/aplica | não | aprovação |
| Concessão de gold | não | não | não | nunca direto | nunca direto | maker/checker | maker/checker |
| Restore/wipe | não | não | não | não | duas aprovações | não | duas aprovações |
| Gerenciar segredos | não | não | não | não | escopo restrito | não | break glass |

Permissões concretas em `panel-types`: `health.read`, `logs.read`, `player.read`, `player.kick`, `player.mute`, `player.ban`, `world.read`, `world.start`, `world.drain`, `world.stop`, `config.read`, `config.propose`, `config.approve`, `config.apply`, `economy.read`, `economy.grant.propose`, `economy.grant.approve`, `backup.create`, `restore.plan`, `restore.execute`, `iam.manage`, `audit.read`.

---

## 16. Backup, persistência, restauração e recuperação de desastre

### 16.1 Requisitos

- Capturar WDB do `gamedbd`, do `uniquenamed` e stores auxiliares necessários no **mesmo ponto consistente**; considerar logs de transação, checkpointer e dependência entre storages.
- Verificar caminho seguro usando `StorageEnv::checkpoint()` e shutdown gracioso; **não** simplesmente copiar diretórios enquanto alterados sem snapshot consistente.
- Realizar backups incrementais e completos, verificando checksum e tamanho, criptografia em repouso, local separado e retenção.
- Separar backup de **dados de jogo**, **config revisada**, **assets/version**, **identity DB**, **ledger**, **audit**.
- Rodar restores regulares em sandbox isolado e validar login/role/inventory/cash/uniqueness com fixtures.
- Definir RPO/RTO com a equipe de produto antes do primeiro real-money order; medir se a capacidade real atende.
- O painel precisa mostrar: último backup íntegro, último restore de prova, local, revisão, tamanho, erro e validade.

### 16.2 Runbook de restauração (alto nível)

1. Iniciar incidente e marcar manutenção; congelar novas concessões/entradas de jogadores.
2. Quiesce com mecanismo de parada graciosa verificado por daemon, aguardar checkpoint.
3. Snapshot/backup do estado atual mesmo que suspeito, para investigação forense.
4. Validar checksums/version/zone IDs e selecionar ponto do tempo.
5. Restaurar em ambiente de recuperação **isolado**; verificar WDB e nomes únicos, char index, ledger e serial `AddCash`.
6. Confrontar PSP/ledger com progresso WDB pós-backup; **não reemitir gold sem conciliação**.
7. Aprovar corte, restaurar serviços em ordem, fazer probes e real client smoke.
8. Só então reabrir login; registrar postmortem e evidência.

**ATENÇÃO:** o uso indiscriminado de `--fresh` deve ser proibido em runbooks de produção. Mesmo o subcomando `repairdb` no código não equivale a recuperação universal; validar em cópia, documentando escopo e impacto.

---

## 17. Falhas existentes que o painel precisa considerar desde o início

O relatório `docs/SERVER153-REPORT.md` registra bugs estruturais já diagnosticados/corrigidos: discrepância de layout de locks/ODR (`_REENTRANT_`), `gnet_map` hash vs map, wrappers RPC de `gauthd`, state arrays de MatrixPasswd, provider reannounce, formato do GShop, divergência `RoleInfo` do CreateRole, deadlock e corrupção de `UserContainer`, starvation da prioridade 101, use-after-free no logout/relogin, SIGFPE de `hashtab`, instância `is61` indisponível, worldtag incorreta, state CORRUPT do WDB.

Para o painel:
- não classificar “bug corrigido” como incapacidade impossível de voltar;
- registrar `build_revision` e tests por correção;
- preservar a suíte de conformance de `RoleInfo` (1704 bytes/23 campos);
- oferecer correlação de `retcode`:
  - `0` success de login;
  - `10` sessão duplicada;
  - `21` envio RPC falhou no sponsor;
  - `8` erro de comunicação;
  - `45` nome duplicado;
  - `result=31`/`gameid=-1` sem provider;
  - `Decode Error type=84` incompatibilidade CreateRole;
- mostrar `stale provider`, reconnect e restart storm;
- prever graceful stop, locks e health sem depender de contador de PID;
- manter `gfaction` sob observação quanto a provider duplicado;
- impedir que um script de configuração aparentemente “benigno” cause drift de version/edition, charset ou porta.

---

## 18. Avaliação de projetos/painéis externos (referências, não dependências)

| Projeto | Link | O que aproveitar | Risco e recomendação |
| --- | --- | --- | --- |
| `shadowvzs/pwAdminV2` | https://github.com/shadowvzs/pwAdminV2 | separação frontend React/TypeScript e backend NestJS, integração MySQL; bom **benchmark de arquitetura** | README curto, pouca evidência de maturidade; auditar licenças, RPCs, auth e código antes de reutilizar |
| `shadowvzs/pwAdmin` | https://github.com/shadowvzs/pwAdmin | item builder, webshop, registro, conta e ações de servidor via sockets; **benchmark funcional** | alvo 1.4.2, PHP legado e SSH opcional; **não expor à internet** sem reescrever segurança |
| `hrace009/pwAdmin` | https://github.com/hrace009/pwAdmin | Java/JSP com PW-Java API; páginas `serverctrl.jsp`, `role.jsp`, `status.jsp`; útil para inventário de comandos | tecnologia antiga; auth, input validation e compatibilidade exigem auditoria |
| `tomirons/pw-web` | https://github.com/tomirons/pw-web | produto público, contas, rankings, serviços, shop e suporte como referência UX | autor declara **sem suporte**; PHP antigo, instalação pede permissões 777 — inadequado como base de produção |
| `ilhaaam/pw-web` | https://github.com/ilhaaam/pw-web | fork que declara suporte a 1.5.3+ e evolução de painel user/admin | próprio README diz trabalho incompleto/com bugs; não assumir compatibilidade com **nosso** WDB/gauthd |
| `hrace009/PW-Panel` | https://github.com/hrace009/PW-Panel | UI Laravel/Tailwind para usuário; catálogo de recursos e migrations | README declara muitos bugs; não presumir controle completo de daemons |
| `vir5050/PW-Panel` | https://github.com/vir5050/PW-Panel | outra implementação Laravel de painel do ecossistema | verificar autoria/linhagem, SQL e segurança antes de adotar |
| `PW Studio` | https://pwstudio.pw/docs | inspiração para editores `elements.data`, GShop, quests, mapas, NPCs e skills | ferramenta externa de edição, não substituto do controle-plane; validar permissões e licenças |
| `oss-perfectworld/perfectworld` | https://github.com/oss-perfectworld/perfectworld | docker-compose por daemon e topologia de infra | 1.5.5 e imagens antigas; README declara incompatibilidade com pwAdmin que usa processos/SSH; usar como referência de isolamento |

**Recomendação final:** construir painel modular nosso aproveitando **conceitos e requisitos**. **Não** implantar nenhum painel antigo sem pentest/compat, nem copiar código sem licença. Se for acelerar UX/contratos, estudar `pwAdminV2` e `hrace009/pwAdmin` primeiro. Sistemas que funcionam por SSH root ou acesso SQL direto ao WDB não são arquitetura aceitável.

---

## 19. CI/CD, testes, migrações e Definition of Done

### 19.1 Estratégia de testes por camada

| Camada | Testes mínimos |
| --- | --- |
| `panel-types` | schema validation, compatibilidade backward, OpenAPI diff |
| `control-api` | auth/RBAC negative tests, CSRF, sessions, pagination, idempotency, audit |
| `auth-adapter` | registrar → login → logout → ban → re-login → password reset; real GNET wire |
| `game-query-adapter` | GMOnlineNum, GMListOnlineUser, timeouts, cursor, unauthorized, malformed packets |
| `economy-worker` | webhook repetido, timeout após commit, serial fora de ordem, refund, backpressure, DB indisponível |
| `realm-controller` | state machine, dry-run, concurrent operations, map collisions, rollback, ownership |
| `realm-agent` | privilege separation, command allowlist, process identity, mTLS, heartbeat, no arbitrary shell |
| `config-catalog` | parser, comments/encoding, baseline, diff, schemas, bad key, hot/cold classification |
| `gs/map controller` | start with data, announce, login, migration/drain, stop, restart, crash recovery |
| `storage` | backup consistente + restore + checksum + jogador e gold preservados |
| `UI` | acessibilidade, mobile, estados vazios, loading e atualização em tempo real |
| `observability` | readiness semântica, delayed metrics, stale data, alert thresholds |
| `security` | secret scan, dependency audit, DAST, fuzz de GNET, RBAC cross-realm |

### 19.2 Testes de aceitação end-to-end

1. Criar conta real com e-mail e senha aleatória; login PWBR cliente original e browser bridge quando pronto.
2. Impedir login com conta/senha inválidas; verificar que qualquer múltiplo de 16 **não** autentica automaticamente.
3. Criar personagem; sair e voltar; persistir posição/equipamento; listar no portal.
4. Ativar e desativar login do realm com manutenção; usuário logado continua consistente.
5. Observar `gs01` e `is61` separadamente, detectar queda de apenas um e marcar mapa degradado.
6. Criar novo mapa **somente após** teste de hot-start; sem colisão de provider.
7. Não permitir parada de mapa com jogador ativo sem fluxo de drain e aprovação.
8. Trocar parâmetro em revisão, validar, aplicar com health proof e reverter.
9. Comprar gold; duplicar webhook 10 vezes; saldo WDB aumenta **uma única vez**.
10. Falha de rede logo após commit de `AddCash`; retry não duplica; serviço reconcilia.
11. Rodar backup; matar stack; restaurar WDB e identidade; saldo e role permanecem consistentes.
12. Remover `gdeliveryd`; dashboard detecta game unready, abre incidente, recupera coordenadamente.
13. Sem datastore de painel, o jogo já conectado continua; sem `gauthd`, novos logins falham **fechados**.
14. Compilar a source atual com CI e rodar conformance wire (CreateRole 1704 bytes), sem stub falsamente verde.
15. Auth de staff exige MFA; operador de um realm não altera outro; ações geram audit trace.

### 19.3 Gating de release

```text
NOT_STARTED → IMPLEMENTED → UNIT_TESTED → INTEGRATION_TESTED
            → REAL_CLIENT_PROVEN → LOAD_TESTED → SECURITY_REVIEWED
            → RESTORE_DRILL_PASSED → PRODUCTION_READY
```

A execução deve publicar evidência (commit, build, comando, logs sanitizados, métricas, resultado). **NOT RUN ≠ PASS**. Guardar regressões dos bugs de GNET/lock no CI. Os testes atuais documentados não cobrem ainda carga real do painel ou todos esses cenários.

---

## 20. Backlog executável priorizado

### P0 — Bloqueadores (fazer antes de qualquer usuário público)

| ID | Entregável | Critério de aceite |
| --- | --- | --- |
| PANEL-P0-01 | Remover/dev-gate auth insegura; integrar `gauthd` a contas reais | teste negativo de senha errada, usuário inexistente e múltiplos de 16 |
| PANEL-P0-02 | Rotacionar segredos versionados e criar secret manager | nenhum segredo fixo em imagem/script/log; teste de integração |
| PANEL-P0-03 | Definir e medir persistência/backup WDB | restore drill com role e gold, sem `--fresh` |
| PANEL-P0-04 | Criar readiness do realm com `gs` e providers | health NÃO saudável sem mapa jogável |
| PANEL-P0-05 | Corrigir supervisor de processos e shutdown gracioso | consegue parar/reiniciar sem perder personagens |
| PANEL-P0-06 | Unificar geração de configuração local/Cloudflare | artefatos de runtime reprodutíveis com diff e verificação |
| PANEL-P0-07 | Confirmar build/cliente real e estado das provas | evidência commitada de login, create, enter, movimento, relogin |

### P1 — Primeiro painel funcional, útil e seguro

| ID | Entregável | Critério de aceite |
| --- | --- | --- |
| PANEL-P1-01 | `panel-types` + OpenAPI e erros | contract tests entre web/API/agent |
| PANEL-P1-02 | `control-api` + migrations e RBAC | login, roles, session, MFA, negative auth |
| PANEL-P1-03 | `realm-agent` mínimo read-only | CPU/RSS/process/socket, signed heartbeat |
| PANEL-P1-04 | Dashboard de saúde/alertas | oito daemons, `gs01`, `is61`, freshness, incident |
| PANEL-P1-05 | Adaptador GNET `GMOnlineNum`/`GMListOnlineUser` | população e lista paginada testadas |
| PANEL-P1-06 | Portal registro/conta/personagem read-only | cadastro real + lista com ownership |
| PANEL-P1-07 | Logs estruturados e traces | correlação de um login ponta a ponta |
| PANEL-P1-08 | Jobs/audit/approval engine | operação com lifecycle e replay-safe |
| PANEL-P1-09 | Runbooks versionados e alerta storage | diagnóstico, retenção e proteção |
| PANEL-P1-10 | Render mobile/desktop, acessibilidade e i18n | UI robusta a offline/stale |

### P2 — Operações administrativas

| ID | Entregável | Critério de aceite |
| --- | --- | --- |
| PANEL-P2-01 | Inventário de `gs.conf` e `config-catalog` | 316 seções parseadas, cada alteração tipada |
| PANEL-P2-02 | World registry + provider matching | `gameid/worldtag/PID` realmente reconciliados |
| PANEL-P2-03 | Start/stop/drain de instância | tests sem perda de players e sem ID duplicado |
| PANEL-P2-04 | Orquestração de restart de realm | manutenção, ordem, health e rollback |
| PANEL-P2-05 | Editor de configuração com diff e aprovação | apply revision → prova → rollback |
| PANEL-P2-06 | Comandos GM tipados (kick, mute, ban) | permissões/reason/duração/audit e retcode |
| PANEL-P2-07 | Administração de contas e personagem | leituras consistentes e sanções refletidas no jogo |
| PANEL-P2-08 | Backup/restore via painel | restore drill automatizado em sandbox |
| PANEL-P2-09 | Telemetria de `gs` e pressão de fila GNET | dashboards confiáveis com p95/p99 |

### P3 — Economia, loja e experiência

| ID | Entregável | Critério de aceite |
| --- | --- | --- |
| PANEL-P3-01 | Schemas de `gshop` e editor de catálogo | roundtrip 196/220 e compatibilidade v145 |
| PANEL-P3-02 | Ledger/PSP + webhook assinado | não há crédito sem pagamento confirmado |
| PANEL-P3-03 | Adapter `AddCash` idempotente | sem duplicidade em timeout/retry |
| PANEL-P3-04 | Reconciliação ledger↔WDB | alertas e resolução com prova |
| PANEL-P3-05 | VIP, cupons, campanhas/eventos | limites financeiros e aprovação |
| PANEL-P3-06 | Suporte e histórico de compras | rastreio por order_id/role_id autorizado |
| PANEL-P3-07 | Moderação, facções, rankings | integração verificada com GNET/DB |
| PANEL-P3-08 | Editor de quests/NPCs/drops | import/export validado e release seguro |

### P4 — Escala e governança avançada

- Multi-realm/zone com isolamento e controle de capacidade.
- Estratégia de autoscaling de instâncias **após provar** hot-start/state transfer.
- Profiler/tick flamegraphs e otimizações no `gs`, batching DB, locks e serialização.
- Portal de autosserviço de administradores, break-glass e delegação.
- Analytics antifraude, detecção de bots, economia, player churn.
- Incident automation e manutenção programada sem perda de jogadores.
- Telemetria e rollout canário de versão/asset/cliente/browser.
- Restores point-in-time e migração WDB sob requisitos legais e operacionais.
- Export de métricas de negócio e custos por jogador/zone/mundo.
- Integração futura com Forge6 somente por APIs/contratos; **não** colocar orquestrador de IA com acesso irrestrito à economia ou banco do jogo.

### 20.1 Ordem de dependências

```text
P0-01/P0-02 → P1-02 → P1-06
P0-03/P0-05 → P1-03 → P1-04 → P2-02 → P2-03/P2-04
P0-06 → P2-01 → P2-05
P1-02/P1-08 + game adapter → P2-06/P2-07
P0-01/P0-03 + P1-08 → P3-02/P3-03/P3-04
P0-07 → integração GNET e validação browser
```

---

## 21. Critérios de qualidade da experiência de operação

- Cada tela mostra `observed_at`, origem, status de sincronização e refresh; não esconder dados stale.
- Ações destrutivas têm pré-visualização de afetados (jogadores online, mapas dependentes, downtime previsto), confirmação explícita e etapa de aprovação se exigida.
- Nenhuma ação deve apresentar “concluído” ao receber HTTP 202; aguardar verificação do estado.
- Botões devem ser desabilitados quando capacidade não for comprovada, com motivo visível.
- Suporte a auditoria pesquisável por operação, jogador, GM, período, valor, mapa, ID e release.
- Versão mobile foca monitoramento e aprovações; operações de alto risco exigem step-up e UX sem toques acidentais.
- Filtros/consultas paginadas; jamais puxar toda a tabela de `UserContainer` repetidamente para contar online.
- Acessibilidade AA como objetivo e internacionalização PT-BR/EN.
- Logs exportáveis precisam de sanitização automática, redaction e permissão por campo.

---

## 22. Plano de deployment recomendado

### 22.1 Primeiro realm jogável

1. Host Linux com Debian/Ubuntu compatíveis, toolchain i386 **só no build**, runtime mínimo e supervisor.
2. Imagem de jogo imutável + pacote de data files v145 validado (hash/rights), volume WDB persistente com checkpoint, backup off-host.
3. Rede privada para GNET RPC; publicar só gateway de jogo, APIs do portal, health sanitizado conforme política.
4. Banco do painel separado, migrations, backup independente e comunicação autenticada com agent.
5. Workers/CDN podem servir portal/static e gateway WSS quando comprovado; **o container Cloudflare lite atual não deve ser tratado como host de gameplay escalável**.
6. Separar ambiente `dev` (dev-auth explícito), `staging` (auth real sem pagamento, dados fictícios) e `prod` (segredos/PSP reais, sem debug).

### 22.2 Bootstrap proposto — diferentes perfis

- `dev`: um realm local, dados de teste, `gs01+is61`, agent em localhost, API/painéis.
- `staging`: topologia de produção, auth real, PSP sandbox, snapshots, carga sintética, gateway.
- `prod`: deploy com rollback, backups, observabilidade, checklists de segurança, manutenção e rotação de chaves.

Nunca compartilhar contas de teste, secrets ou volume WDB entre perfis.

---

## 23. Perguntas em aberto que exigem experimento, não opinião

1. Quais campos/versões exatos da auth externa presente no `pw.sql` podem ser usados, caso haja compatibilidade com a árvore 1.5.3?
2. Quais RPCs de GM podem ser invocados por uma **conta de serviço** sem passar por login de jogador/estado `gmroleid`? Quais exigem `state.hxx` específico?
3. Quais operações de `gamedbd.cpp` são seguras com daemon ativo versus exigem offline?
4. Qual o procedimento de backup consistente do WDB com todos os storages e uniquenamed, sob write load?
5. Qual consumo real de `gs01`, `is61`, uma nova instância vazia e mundos sob 10/100/1000 jogadores?
6. Qual o conjunto de worldtags, gameids, bases, terrain assets e dependências para cada instância do `gs.conf`?
7. Qual mecanismo de shutdown/drain existe no `gs`, incluindo persistência de players e coordenação com `gdeliveryd`?
8. Quais flags de `GMSetGameAttri` são aplicadas dinamicamente, e quais não sobrevivem a restart?
9. Como a cash shop do PWBR 1.5.3 v145 calcula saldo, oferta, serial, VIP e sincronização exata?
10. Há drift no `cloudflare/entrypoint.sh` quanto a version/edition do glinkd comparado ao `boot-server153.sh`? Confirmar com diff de runtime e cliente real.
11. Os assets e source estão cobertos integralmente pela autorização de distribuição e pela estratégia legal prevista em `docs/LEGAL.md`?
12. O que acontece com sessão de jogador quando o `gdeliveryd` reinicia ou quando `is61` desaparece? Elaborar experimentos de falha.
13. Como impedir spawn abaixo do terreno no `is61` registrado em `08-WORLDS-AND-INSTANCES.md` sem alterar dados de cliente indevidamente?
14. A loja web deve vender apenas gold, itens diretos ou ambos? **O design de ledger suporta ambos**, mas a entrega de item precisa de outro adaptador, não simplesmente `AddCash`.
15. Há necessidade de múltiplos realms por versão de cliente? Fixar chave realm/zone desde v1 para evitar migração dolorosa.

---

## 24. Plano de execução sugerido para um agente implementador

**Objetivo do agente:** implementar módulos de forma progressiva, com testes e PRs menores, sem quebrar a PR #17 do servidor nem confundir documentação com funcionalidade.

1. Ler `AGENTS.md`, `docs/server/`, `SERVER153-REPORT.md`, `LEGAL.md` e este documento; escrever `IMPLEMENTATION-TRACEABILITY` relacionando requisitos com código, tests e evidências.
2. Fazer inventário estático de configs e GM RPCs; produzir matriz `PROTOCOLO | TIPO | HOP | STATE | AUTH | REQUEST | RESPONSE | TEST`.
3. Projetar contrato `panel-types` e decisões de separação control plane/WDB; abrir ADR para auth e para supervisor de `gs`.
4. Criar primeiro o `realm-agent` **read-only** e provas de health/status sem alterar C++.
5. Implementar identidade persistente e auth do jogo em branch independente, com rollback e teste negativo contra dev-auth.
6. Construir `control-api`/dashboard que só lê estado real; jamais chamar scripts destrutivos por endpoint.
7. Introduzir jobs, RBAC e audit antes do primeiro comando mutante.
8. Implementar prova de backup/restore WDB; só depois operações de mapa, restart e gold.
9. Integrar `GMOnlineNum`, `GMListOnlineUser` com teste de estado GNET; não presumir RPC externo.
10. Fazer benchmark de `gs` por instância e carga sintética; adicionar métricas e capacidade verificadas.
11. Implementar config catalog + diff + rollout seguro e then world-controller.
12. Só iniciar cobrança depois de fechar dev-auth, integrar PSP, ledger, reconciliação, idempotência e aprovação.
13. Garantir atualização deste arquivo quando forem descobertas novas configs, RPCs, limites e gaps.
14. Cada PR deve declarar: hipótese, arquivos afetados, testes executados, evidência PASS/FAIL/NOT RUN, riscos, rollback e planos seguintes.

### Definition of Done do primeiro MVP do painel

**Não é “página bonita”**: portal com conta persistente real, login GNET demonstrado, personagem consultável, health semântico com `gs`, dashboard com oito daemons + instâncias, telemetria real, backups restauráveis, MFA/admin RBAC, audit, testes negativos, docs e deploy reprodutível. Controle avançado de mapas e economia **não precisam entrar nesse MVP** enquanto seus invariantes não forem seguros; disponibilizar como roadmap, não botões falsos.

---

## 25. Fontes externas consultadas

- `shadowvzs/pwAdminV2` — https://github.com/shadowvzs/pwAdminV2
- `shadowvzs/pwAdmin` — https://github.com/shadowvzs/pwAdmin
- `hrace009/pwAdmin` — https://github.com/hrace009/pwAdmin
- `tomirons/pw-web` — https://github.com/tomirons/pw-web
- `ilhaaam/pw-web` — https://github.com/ilhaaam/pw-web
- `hrace009/PW-Panel` — https://github.com/hrace009/PW-Panel
- `vir5050/PW-Panel` — https://github.com/vir5050/PW-Panel
- `oss-perfectworld/perfectworld` — https://github.com/oss-perfectworld/perfectworld
- `PW Studio` — https://pwstudio.pw/docs
- Cloudflare Containers — https://developers.cloudflare.com/containers/
- Durable Object Container API — https://developers.cloudflare.com/containers/api/durable-object-container/

Estas referências são informativas. **Nenhum código externo foi copiado neste documento**; compatibilidade, licença, segurança e atualidade dos projetos externos devem ser avaliadas separadamente.

---

## 26. Registro de divergências e descobertas importantes para futuros revisores

1. O PR #17 possui texto inicial que marca tarefas de runtime como pendentes; a documentação posterior, dentro da branch, registra avanços de execução. **Sempre conferir o commit de evidência e não só o texto do PR.**
2. O relatório histórico mais antigo não provava CreateRole/EnterWorld pelo cliente real; o índice mais recente do servidor relata o fluxo completo. **Reexecutar teste antes de anunciar produção.**
3. O Cloudflare atual prova oito daemons e TCP mediation; **não roda `gs`**, portanto **não prova realm jogável**.
4. Há 48 storages WDB em runtime; **MariaDB não é o DB de personagens**.
5. `panel-server153.py` gera credencial de teste — **não cria usuário persistente**.
6. Os limites de `is61` em `gs.conf` não são benchmark.
7. `AddCash` tem mecanismo de serial e transação; `DebugAddCash` existe, mas **não é mecanismo legítimo de pagamento**.
8. `gmrestartserver.hpp` implementa anúncio/forbid e job de reinício dentro do jogo; **não substitui supervisor, checkpoint nem shutdown gracioso de container**.
9. O script `boot-server153.sh` recebe `--no-gs`, mas no snapshot examinado a variável `NOGS` não governa a última etapa de start; corrigir/validar antes de depender dessa opção.
10. `scripts/boot-server153.sh --stop` identifica `gs` pelo nome esperado, enquanto `gs-server153.sh` copia o binário como `gs-ours`; validar remoção de todos os netos, processos órfãos e watchdog antes de expor “parar servidor”.
11. `cloudflare/entrypoint.sh` e `boot-server153.sh` codificam a mesma série de patches manualmente; mudar um **não garante** mudar o outro.
12. `gdeliveryd/gamesys.conf` tem comentários/valores antigos; a prioridade 101 é sobretudo auth, enquanto `EnterWorld` executa inline (`priority=0`), conforme correção documentada em `SERVER153-REPORT.md`. Não reproduzir interpretações antigas dos comentários dos scripts como verdade de scheduling.
13. O portal e o console não devem competir pela porta local 8080 utilizada por outros programas; configurar portas de desenvolvimento por ambiente e evitar hardcode.
14. O código `gs` é 32 bits com dependencies históricas; container e volume devem prever runtime compatível, simbolização e testes de sanitizers quando possível.
15. Números de usuários, latency e RSS devem ser rotulados `MEDIDO`/`ESTIMADO`/`DESCONHECIDO`, com revisão/ambiente/cenário; nada de benchmarks inventados.

---

## 27. Critério final de sucesso do projeto

Teremos o painel certo quando um operador autorizado puder, com segurança:

- descobrir **qual daemon ou mapa está ruim**, quando piorou e por quê;
- distinguir o mundo realmente jogável de um `health 200` enganoso;
- listar online, personagens e saldos sem corromper WDB;
- configurar e publicar uma revisão com validação/rollback;
- colocar realm em manutenção, drenar instâncias e reiniciar sem sumir progresso;
- detectar/recuperar falhas com backup e restore ensaiados;
- permitir contas reais e seguras, inclusive com cliente original 1.5.3 v145 e depois browser;
- vender gold de forma auditável, idempotente e conciliada;
- manter direitos de acesso e auditoria mesmo com equipe grande;
- aumentar capacidade **com base em medições** de CPU/RSS/tick/latência por mapa e jogador.

**Visão-alvo:** transformar o servidor 1.5.3 herdado em uma plataforma de MMO observável, administrável e operacionalmente confiável, preservando protocolo, estado e compatibilidade, sem encobrir as limitações técnicas que hoje ainda existem.
