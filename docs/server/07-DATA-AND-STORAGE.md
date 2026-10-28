# 07 — Dados e storage

**Data:** 2026-10-28 UTC · **Fontes:** `cnet/storage/storagewdb.h`, `cnet/gamedbd/` (accessdb.cpp, gamedbmanager), `scripts/boot-server153.sh`, `scripts/gs-server153.sh`, pacote v145

## Visão

O servidor **não usa MariaDB/MySQL**. Toda a persistência de jogo é o
storage embutido (engine proprietário **WDB** + Berkeley DB legado) nos
daemons gamedbd e uniquenamed. MariaDB roda na máquina com o schema `pw.sql`
importado, mas **nenhum daemon o lê** — o MySQL citado no readme upstream é
para as ferramentas administrativas (iweb/java/php), que não estão nesta
source.

## Engines de storage

| Engine | Conf | Usado por | Observações |
| --- | --- | --- | --- |
| **WDB** (`storagewdb.h`) | `[storagewdb]` | gamedbd, uniquenamed | engine principal; `homedir=dbhomewdb`, `datadir=dbdata`, `logdir=dblogs`, `checkpoint_interval=60`, `times_incbackup=24` (a cada N backups, completo em vez de incremental) |
| BDB (`storagebdb.h`) | `[storage]` | gamedbd, uniquenamed, gfacion (cache) | legado; `cachesize`, `compress=1`, `checkpoint_interval=300` |

- `StorageEnv::Open()` lê `tables=` da conf e cria um `Storage` por tabela
  com compressor; cache LRU por tabela
  (`cache_high_default`/`cache_low_default`/`<tabela>_cache_high`).
- **BackupThread** (`storagewdb.h:695-755`): loop periódico de checkpoint +
  backup incremental/completo + limpeza de logs antigos, controlado por
  `backup_lockfile`/`quit_lockfile`.
- **Sem recuperação de estado CORRUPT**: `truncate()` retorna CORRUPT e não
  há caminho de repair — um shutdown sujo pode travar o boot do gamedbd
  permanentemente. Nesses casos o `--fresh` do boot script (que apaga TODO o
  storage) é o caminho, com a perda de dados que implica.
- Shutdown gracioso: `DbPolicy::OnQuit()` faz checkpoint e close (os scripts
  upstream `killgame`/`stopgame` usam `killall -SIGUSR1` nos daemons de DB).

## As 48 tabelas do gamedbd (runtime)

O conf upstream declara 26; o código abre ~48 via `GetStorage()` —
`GetStorage("syslog")` retornava NULL e o `InitGUID` segfaultava. O conf de
runtime lista todas (tabelas ausentes são criadas vazias sob demanda):

`auction, base, city, clsconfig, config, crslogicuid, equipment,
factionfortress, factioninfo, factionname, factionrelation, force,
friendext, friends, globalcontrol, gtask, inventory, kingelection, mailbox,
mappassword, messages, mndomainbonus, mndomaininfo, mnfactionapplyinfo,
mnfactioninfo, order, playerprofile, playershop, recalluser, rolename,
rolenamehis, sellpoint, serverdata, shoplog, solochallengerank, status,
storehouse, syslog, task, translog, uniquedata, user, userfaction,
userstore, waitdel, weborderitem, webtrade, webtradesold`

As centrais para login/roles:

| Storage | Conteúdo |
| --- | --- |
| `user` | UserInfo: conta, rolelist, cash |
| `base` | GRoleBase: race, classe, gênero, level, posição… |
| `status` | GRoleStatus: hp/mp/exp/propriedades/posição persistida |
| `equipment` / `inventory` / `storehouse` | equipados / mochila / armazém |
| `task` | quests do personagem |
| `clsconfig` | templates de spawn por classe (importado a cada boot de `cnet/gamedbd/clsconfig`) |
| `rolename` | unicidade de nomes (espelhada no uniquenamed) |
| `config` | DBConfig: init_time, create_count… |
| `waitdel` | roles marcadas para deleção agendada |

Subcomandos administrativos do binário gamedbd (`gamedbd.cpp`):
`importclsconfig`, `exportclsconfig`, `query`, `listrole`, `merge`,
`tablestat`, `listsyslog`, `deletewaitdel`…

## uniquenamed (unamewdb)

Tabelas: `config, uidrole, unamefaction, unamerole` (storage WDB próprio,
`uniquenamed/unamewdb/`). Responsável por unicidade de nomes de
roles/factions/families e alocação de LogicUID (`LogicuidManager` +
thread `LogicuidSeeker` que busca IDs livres periodicamente). RPCs:
`precreaterole`/`postcreaterole` (reserva/consolida nome), variantes para
rename/faction/family, `rolenameexists`, `userrolecount`, `moverolecreate`
(merge de DB).

## clsconfig — templates de spawn por classe

- Arquivo fonte: `cnet/gamedbd/clsconfig` — importado para o storage
  `clsconfig` a cada boot do gamedbd (`importclsconfig`).
- Usamos o **original do pacote PWBR** (61 KB, md5 `dd08cebb…`) — o vendorado
  upstream (90 KB) spawnava no is61 com posições quebradas para algumas
  classes.
- Define atributos iniciais por ocupação (8 classes: SWORDSMAN, MAGE, MONK,
  HAG, ORGE, GENIE, ARCHER, ANGEL) — `race`, `posx/posy/posz`, `money`, HP/MP,
  atributos, velocidades. O `ptemplate.conf` do gs carrega a versão de
  runtime dos mesmos atributos.

## Data files do jogo (v145)

**Não estão na source** — vêm do pacote do servidor `pwserver_153v145`
(bind `/home/pwserver/gamed`) e do cliente real instalado (bind `/gamedata`).
O `gs-server153.sh` liga os dois no `config/` do gs:

| Arquivo | Origem | Papel |
| --- | --- | --- |
| `elements.data` (v145) | cliente | itens, NPCs, skills, equipamentos — **deve ser byte-idêntico ao do cliente** |
| `tasks.data` (v125) / `dyn_tasks.data` | cliente | quests (binário com header/magic/version) |
| `aipolicy.data`, `task_npc.data` | cliente | IA, NPCs de quest |
| `gshop.data`, `gshop1/2.data` | cliente | lojas |
| `domain*.data` | cliente | domínios |
| `gshopsev.data`, `gshopsev1.data` | pacote | mall de cash — **formato 196→220 bytes convertido pelo script** (bug B07; originais preservados `.old`) |
| `npcgen.data`, `world_targets.sev`, `extra_drops.sev`, `precinct.sev`, `region.sev`, `path.sev` | pacote | spawns de NPC, drops, regiões |
| `map.bht`, `wmgj.nmd` | pacote | colisão (brush + elementos) |
| `movemap/`, `watermap/`, `airmap/` | pacote | pathfinding por NPC |
| `global_api.lua` | pacote | versão de scripts Lua (só a versão é lida no boot) |

O `[Template]` do gs.conf referencia tudo isso (`Root=./config/`); sem os
arquivos o gs inicia, carrega substâncias e sai com código **-300**
(falta `./config/aipolicy.data`).

## Encoding de dados

- GBK em strings de dados e logs (`iconv -f GBK -t UTF-8` para ler).
- `name_charset=UCS2`/`table_charset=GBK` no gs.conf (nomes UCS2; tabelas GBK).
- `filters` (palavras sensíveis) é GBK — os confs upstream declaram UCS2
  (bug corrigido no runtime conf).

## MariaDB (estado atual)

- Rodando com `pw.sql` importado; **não consumido por nenhum daemon**.
- Uso futuro: auth real de contas (substituindo o dev-auth) e ferramentas
  administrativas quando existirem. O painel `scripts/panel-server153.py`
  é o rascunho do caminho de contas.
