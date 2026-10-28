# 08 — Mundos e instâncias (gs)

**Data:** 2026-10-28 UTC · **Fontes:** `cgame/gs/start.cpp`, `global_world_stubs.cpp`, `worldmanager.cpp`, `gs.conf` do pacote v145, relatório de exploração da sessão

## Boot do gs: fork por instância

Linha de comando (pacote v145, via `scripts/gs-server153.sh`):

```
./gs-ours gs01 gs.conf gmserver.conf gsalias.conf is61
```

- `argv[1]` = servername (`gs01`), `argv[2]` = `gs.conf`, `argv[3]` =
  `gmserver.conf`, `argv[4]` = `gsalias.conf`, `argv[5+]` = instâncias extras
  (`is61`).
- `main()` em `cgame/gs/start.cpp:138`. **FirstStepInit carrega TODOS os
  dados globais antes do fork** (`worldmanager.cpp:824-990`: elements,
  tasks, aipolicy, skills, drops, mall, NPC templates, colisão, matcher) —
  os processos filhos herdam via copy-on-write.
- **Double-fork por instância** (`start.cpp:178-219`): o pai cria um filho
  por instância; cada filho faz o segundo fork e o neto vira o processo da
  instância; os intermediários saem. O processo pai também sai ao final —
  **a saída do pai com SIGFPE é cosmética** (destrutores de `ClassInfo` do
  `libskill.so` apagando do registro destruído — bug B11); os processos das
  instâncias sobrevivem.
- Servernames começando com `ms` marcam `g_mobile_server` (variante mobile).

## Seleção do tipo de mundo

`InitWorld()` (`global_world_stubs.cpp:47-127`) classifica o servername:

1. Se está na lista `instance_servers` do `[General]` (gs.conf): lê a seção
   `[Instance_<nome>]` e as flags (`faction_server`,
   `battleground_server`, `countrybattle_server`,
   `countryterritory_server`, **`parallelworld_server`**,
   `trickbattle_server`, `mnfaction_server`) → tipo correspondente.
2. Se está em `world_servers`: mundo aberto (`INIT_BIG_WORLD`) ou mobile.

| Tipo | World manager | Exemplo |
| --- | --- | --- |
| `INIT_BIG_WORLD` | `global_world_manager` | **gs01** |
| `INIT_INSTANCE` | `instance_world_manager` | instâncias genéricas |
| `INIT_PARALLEL_WORLD` | `parallel_world_manager` | **is61** |
| `INIT_FACTION` / `INIT_BATTLEGROUND` / `INIT_COUNTRYBATTLE` / … | variantes | cidades de guilda, batalhas |

## gs01 (mundo principal)

- Seção `[World_gs01]`: `index = 1`, `tag = 1` (**worldtag 1**).
- Anuncia ao gdeliveryd: `gameserver 1's region is (-4096.000, 4096.000,
  5632.000,-5632.000), worldtag is 1`.

## is61 (mundo paralelo — onde personagens novos nascem)

- Seção `[Instance_is61]` (gs.conf do pacote): `index = 101`, `tag = 161`
  (**worldtag 161**), `base_path = a61/`, `parallelworld_server = 1`,
  `player_per_instance = 1000`, `instance_capacity = 10`,
  `pool_threshold_low = 1`, `pool_threshold_high = 10`.
- `parallel_world_manager` (`cgame/gs/instance/`): mantém **até 10 planos
  paralelos** (`_cur_planes`) do mesmo mapa — auto-aloca planos novos
  (`AutoAllocWorld`) quando a capacidade esgota e libera planos vazios após
  `_idle_time` (300 s default); `PlayerSwitchWorld()` troca o player de
  plano; vidas infinitas (`_life_time = -1`).
- Anuncia ao gdeliveryd: `gameserver 101 … worldtag is 161` → **gameid 101**.
- **Por que personagens novos nascem aqui**: o template de spawn da classe
  vem do storage `clsconfig` (importado de `cnet/gamedbd/clsconfig` a cada
  boot do gamedbd) — tanto o arquivo vendored quanto o original do pacote
  PWBR definem o spawn no is61. É **design do reino original**, não bug
  (bug B10). Substituímos pelo arquivo original do pacote (spawns corretos
  para todas as classes).

## Worldtags e roteamento

- O worldtag é anunciado no `AnnounceProviderID` e guardado no
  `gameservermap` do gdeliveryd; `EnterWorld` resolve o gs por
  posição + worldtag (`FindGameServer(point, worldtag)`).
- Role com worldtag 0 não acha servidor (login `result=31, gameid=-1`) — o
  fix `dbcreaterole.hrp` força `worldtag = 1` na criação quando 0 (bug B16).
- Outros mundos paralelos no gs.conf: `is62` (index 102, tag 162),
  `is42` (index 82, tag 142).

## Mapas disponíveis vs. subindo

- O gs.conf do pacote define **dezenas de instâncias** (`[Instance_is05]`
  … is61+, com `index`/`tag`/`base_path` próprios) e os mundos em
  `world_servers`.
- **Só sobem as instâncias passadas na linha de comando** — nosso boot sobe
  `gs01` + `is61` (2 processos gs-ours, listeners 10901/11971). Cada
  instância extra é um processo com seu mapa carregado; não subir mapas
  sem uso é a economia de recursos correta.
- Para subir mais instâncias: acrescentar o nome na invocação (o
  `gs-server153.sh` e o watchdog usam a linha fixa atual).

## gmserver.conf (conexões do gs)

- `[ProviderServers] count = 10` → um `GProviderClient` por provider:
  `GProviderClient0` = gdeliveryd (29300), `GProviderClient1` = gfaction
  (29600), `GProviderClient2-9` = link servers (29301+).
- `[GamedbClient]` = gamedbd (29400); `[LogclientClient]`/`[LogclientTcpClient]`
  = logservice (11100/11101).
- `[ThreadPool]` do gs: `threads=(1,5)(100,1)(101,1)(0,1)`,
  `max_queuesize=1048576`, `prior_strict=1`.

## gsalias.conf

Mapeia nomes lógicos → endereços (`[AddrAlias]`: `WORLD01 = localhost`,
`INSTANCE61 = localhost`, …), apendido ao Conf no boot
(`ONET::Conf::AppendConfFile`). Os `MsgReceiverTCP_XXXX` do gs.conf usam
esses aliases nos `listen_addr`.

## Subsistemas por processo

Cada world manager carrega: pools de NPC/matter/player, terreno + movemap
(`movemap/`, `watermap/`, `airmap/`), colisão (`map.bht` + `wmgj.nmd` via
`trace_manager2`), NPC generator (`npcgen.data`), templates de player
(`ptemplate.conf`), regiões (`precinct.sev`, `region.sev`), paths
(`path.sev`), quests públicas, thread pool, timer, poll, e registra-se nos
providers (`GMSV::InitGSP` → `GProviderClient::Connect` →
`AnnounceProviderID`).

## Limites e limitações conhecidas

- **Spawn Y abaixo do terreno** no is61 (issue em aberto; contorno: teleport
  GM).
- O crash de saída do pai (SIGFPE) é cosmético, mas o guard do hashtab
  (`find`/`erase`) existe para os caminhos equivalentes em runtime (bug B15).
