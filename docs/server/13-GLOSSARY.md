# 13 — Glossário

Termos e siglas usados nesta documentação e no código do servidor.

## Daemons e processos

| Termo | Definição |
| --- | --- |
| **glinkd** | Daemon de entrada do cliente (porta 29000): handshake, login, roles, roteamento. |
| **gdeliveryd** | Orquestrador central: UserContainer, roteamento entre daemons, subsistemas de gameplay globais. |
| **gauthd** | Servidor de autenticação (nó terminal dos ProxyRpcs de login/senha). |
| **gamedbd** | Banco do jogo (storage WDB): roles, inventário, mail, leilões, facções, transações. |
| **uniquenamed** | Unicidade de nomes (roles/factions/families) + alocação de LogicUID. |
| **gfactiond** | Servidor de facções (membros, alianças, fortalezas). |
| **gacd** | Anti-cheat (punições, códigos de verificação, padrões). |
| **logservice** | Coletor central de logs (UDP 11100 / TCP 11101 → arquivos `world2.*`). |
| **gs** | Game server (`cgame/gs`): um processo por mundo/instância (gs01, is61). |
| **gclient** | Cliente virtual de teste in-tree (probe de handshake/carga). |
| **gs-ours** | Nome do binário gs copiado para o pacote de dados v145 pelo `gs-server153.sh`. |

## Protocolo e framework

| Termo | Definição |
| --- | --- |
| **GNET** | Framework de rede do servidor: protocolo, sessões, RPC, threads, marshal (`share/io`, `share/common`). |
| **CompactUINT/SINT** | Encoding compacto de inteiros no wire (1/2/4/5 bytes com marcadores de bits altos — ver [`04-PROTOCOL.md`](04-PROTOCOL.md)). |
| **Octets / OctetsStream** | Buffer de bytes e stream de serialização (`Marshal::OctetsStream`); base de todo marshal. |
| **Protocol (type)** | Pacote GNET identificado por `type` (ex.: 84 = CreateRole); framing `CompactUINT(type)+CompactUINT(size)+corpo`. |
| **Rpc** | Chamada remota com `xid`, argumento e resultado; `Server()` no executor, `Client()` no sponsor. |
| **ProxyRpc** | RPC com hop intermediário (sponsor → proxy → terminal); o terminal precisa implementar `Process()` com o bookkeeping de `SendToSponsor`. |
| **Sponsor / terminal** | Nó que origina o RPC / nó que o executa de fato (gauthd é terminal dos RPCs de auth). |
| **State array** | Lista de tipos de protocolo aceitos por sessão em cada estado (`state.hxx`); tipo ausente = envio falha em silêncio. |
| **PriorPolicy / pool 101** | Prioridade de fila no `Thread::Pool` de cada daemon; 101 = alta (EnterWorld/CreateRole/RoleList). |
| **Challenge / Response** | Handshake inicial cliente↔glinkd (nonce + version + edition). |
| **version 0x10503** | `GAME_VERSION` do cliente PWBR 1.5.3 v145 (Build 2555); o glinkd precisa anunciar igual. |
| **edition** | String que o cliente computa dos próprios data files (`%x%x%x%x%x` de elements/tasks/gshop); precisa bater com a do servidor. |
| **MatrixPasswd / MatrixPasswd2 / MatrixToken** | RPCs de validação de senha (v1/v2/token). |
| **userlogin2** | RPC de resultado de login (gdeliveryd `PostProcess` → glinkd `Client`). |
| **OnlineAnnounce** | Anúncio pós-login que muda o cliente para `state_GRoleList`. |
| **AnnounceProviderID** | Protocolo de registro do gs nos providers (carrega gid, região, worldtag, edition). |
| **gameservermap** | Mapa `id → {sid, region, worldtag}` no gdeliveryd/glinkd com os gs registrados. |
| **halfloginset** | Conjunto de logins parciais no glinkd; estourar `ExceedHalfloginLimit` para o listener. |
| **Decode Error** | Falha de unmarshal de pacote (lança `Marshal::Exception`) — assinatura de divergência de wire. |
| **rpcgen / rpcalls.xml** | Gerador Perl que produz `rpcdata/` (668 estruturas) e `inl/` (1.010 protocolos) a partir do XML. |
| **.hrp** | Arquivo de implementação de RPC (724 em `cnet/` — NÃO são dados de skill). |
| **PWSL** | Perfect World Serverside Library — biblioteca io/log/rpc (não é linguagem de script). |

## Mundos e dados

| Termo | Definição |
| --- | --- |
| **worldtag** | Identificador do mundo no roteamento (1 = gs01 principal; 161 = is61 paralelo). |
| **gs01 / is61** | Mundo principal / instância de mundo paralelo (onde personagens novos nascem por design do clsconfig). |
| **parallelworld_server** | Flag do gs.conf que instancia `parallel_world_manager` (até 10 planos do mesmo mapa). |
| **world manager** | Classe que implementa um tipo de mundo (`global_world_manager`, `parallel_world_manager`, `instance_world_manager`…). |
| **provider** | Conexão registrada do gs no gdeliveryd/glinkd/gfaction (GProviderServer/GProviderClient). |
| **WDB** | Engine de storage embutido (estilo Berkeley DB) do gamedbd/uniquenamed — `dbhomewdb/`, `unamewdb/`. |
| **storage / tabela** | Cada "tabela" é um arquivo no storage (48 no gamedbd: `user`, `base`, `status`, `clsconfig`…). |
| **clsconfig** | Storage+arquivo com templates de spawn por classe (importado a cada boot do gamedbd). |
| **LogicUID** | ID lógico de usuário alocado pelo uniquenamed. |
| **elements.data / tasks.data** | Dados centrais do jogo (itens/NPCs/skills; quests) — v145/v125, byte-idênticos entre cliente e servidor. |
| **gshopsev** | Dados do mall de cash (formato 196 bytes no pacote v145; a source espera 220 — conversor no script). |
| **dev-auth** | Esquema de auth de validação: contas múltiplas de 16, `MD5(conta‖conta)`, `HMAC_MD5(desafio, hash)`. |
| **GBK / UCS2** | Encodings: dados e logs em GBK; nomes de player em UCS2 no wire. |

## Operação

| Termo | Definição |
| --- | --- |
| **pw153-runtime** | Container Docker Debian 12 do runtime local (bind `/work`, `/gamedata`, `/home/pwserver`). |
| **--fresh** | Flag do boot script que APAGA todo o storage (personagens somem) — validação do zero. |
| **watchdog** | `scripts/watchdog-server153.sh`: renasce daemons mortos na ordem, ignora zombies. |
| **zombie** | Processo em estado Z (o PID 1 do container não colhe órfãos) — não conta como vivo. |
| **make -B** | Rebuild total — obrigatório após editar headers (Makefiles não rastreiam dependências). |
| **BUILD-REPORT** | `dist/server153/BUILD-REPORT.txt` — resultado por estágio do build. |
| **wire conformance** | Teste que reproduz pacotes reais byte a byte (`tests/server153_createrole_wire.cpp`). |
| **mediation TCP** | Camada de acesso a portas do container Cloudflare via `ctx.container.getTcpPort(port)`. |
| **DO / Durable Object** | Entidade do Workers que hospeda/controla o container Cloudflare. |
| **lite (instância)** | Tamanho de container CF: 1/16 vCPU, 256 MiB (nosso uso medido: 26 MiB). |
