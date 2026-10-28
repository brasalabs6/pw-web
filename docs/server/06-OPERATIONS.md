# 06 — Operação do servidor

**Data:** 2026-10-28 UTC · **Scripts:** `scripts/boot-server153.sh`, `gs-server153.sh`, `watchdog-server153.sh`, `probe-server153.sh`

## Container de runtime

O stack roda no container Docker **`pw153-runtime`** (Debian 12, toolchain
i386 embutido — usado também para rebuilds):

| Bind mount | Uso |
| --- | --- |
| `~/workspace/pw/pw-source-153 → /work` | a árvore do repo (source, scripts, dist) |
| `~/workspace/pw/game/client/element/data → /gamedata` | data files do cliente real (elements/tasks/gshop — devem ser byte-idênticos aos do cliente) |
| `~/workspace/pw/clients/pwserver_153v145/home/pwserver → /home/pwserver` | pacote de dados do servidor v145 (`gamed/` com confs, `.sev`, npcgen, movemap) |

glinkd publicado em `127.0.0.1:29000` (porta do cliente). O container foi
criado com `--cap-add SYS_PTRACE` para permitir `gdb attach` em diagnóstico
de crashes.

## Ordem de boot (obrigatória)

```
gauthd → gdeliveryd → gamedbd → uniquenamed → gfaction → gacd
→ logservice → glinkd → gs
```

Dependências primeiro: o gdeliveryd só sobe depois do gauthd (auth), o
gamedbd depois do gdeliveryd, o glinkd por último (registra os linkservers no
gdeliveryd) e o gs depois de tudo (anuncia providers ao gdeliveryd).

```bash
# dentro do container:
/work/scripts/boot-server153.sh            # boot completo (gera confs de runtime)
/work/scripts/gs-server153.sh              # gs a partir do pacote v145
# no host:
docker exec -d pw153-runtime bash /work/scripts/watchdog-server153.sh --interval 10
```

O `boot-server153.sh` gera confs de runtime em `dist/server153/runtime/`
(cópias dos confs commitados com os fixes de localhost documentados no
próprio script) e reporta processos + portas ao final (`--wait N`, default 20 s).

Opções: `--stop` (derruba tudo), `--fresh` (**apaga storage** — ver abaixo),
`--no-gs` (gs sobe só via `gs-server153.sh`), `--src/--out` (dirs).

## Estado saudável

- **Processos:** 8 daemons (1 de cada) + gs-ours (pai sai após o fork; as
  instâncias gs01/is61 são os netos) + watchdog.
- **Portas escutando** (medição ao vivo 2026-10-28 02:55 UTC, dono por inode
  de socket):

| Porta | Dono | Papel |
| --- | --- | --- |
| 9200 | gauthd | auth terminal (gdeliveryd conecta aqui) |
| 29000 | glinkd | **entrada dos clientes** |
| 29100 | gdeliveryd | GDeliveryServer (glinkd conecta) |
| 29300 | gdeliveryd | GProviderServer (os gs anunciam providers aqui) |
| 29301 | glinkd | linkserver interno |
| 29400 | gamedbd | GameDBServer (RPCs de storage) |
| 29401 | uniquenamed | nomes únicos |
| 29500 | gfactiond | facções |
| 29600 | gfactiond | segunda porta (vintage do conf) |
| 29702 / 29712 | gacd | anti-cheat |
| 11101 | logservice | logs do gs (world2.log) |
| 10901 / 11971 | gs-ours | listeners das instâncias (gs01 / is61) |

- **Announce de providers no gdeliveryd** (`gdeliveryd.log`):
  `gameserver 1's region is (-4096.000, 4096.000, 5632.000,-5632.000),
  worldtag is 1` e `gameserver 101 … worldtag is 161`.
- Probe de handshake: `scripts/probe-server153.sh` (gclient → exige
  `OnAddSession` + processamento no glinkd).

## Watchdog

`scripts/watchdog-server153.sh --interval 10` (roda no container):

- A cada ciclo checa os 8 daemons + `gs-ours` via `/proc` (comun por nome);
  **zombies (state Z) não contam como vivos** — o PID 1 do container não
  colhe órfãos, e 39 zombies chegaram a enganar um `alive()` antigo.
- Reinicia daemons mortos **na ordem de dependência** (gauthd primeiro,
  glinkd com o argumento de instância `1` — sem ele o glinkd morre na hora).
- gs morto → reexecuta `gs-server153.sh`.
- Log: `dist/server153/runtime/logs/watchdog.log`.
- Validado ao vivo: glinkd morto → renasce em ~10 s → probe GNET OK.

O watchdog é a resposta de curto prazo a "como ter milhares de jogadores se
crasha fácil": um crash vira um blip de segundos. A caça estrutural continua
com builds TSan/ASan (milestone separado).

## Logs

| Log | Onde | Conteúdo |
| --- | --- | --- |
| `glinkd.log`, `gdeliveryd.log`, `gauthd.log`… | `dist/server153/runtime/logs/` | stderr dos daemons (traces `[glinkd]`/`[delivery]`/`[auth]`/`[enterworld]`) |
| `watchdog.log` | idem | reinícios |
| `gs.log` | idem | boot do gs (substâncias, sessões, provider) |
| `world2.log` | `src/server153/cnet/logservice/logs/` (no container) | **runtime detalhado do gs** (spawns, contagem de players) — GBK: ler com `iconv -f GBK -t UTF-8` (用户数目 = contagem de usuários) |

- Logs são **append-only entre restarts**: conte o delta de linhas antes de
  atribuir um erro a uma tentativa nova.
- Grep útil de login: `[glinkd]|[delivery]|[auth]|matrixpasswd|userlogin|
  OnlineAnnounce|enterworld|createrole` (excluir ruído `logclient`).
- O stdout do gs só tem mensagens de boot; o runtime vai para o logservice.

## Restart seguro (regra de ouro)

**Nunca reinicie um daemon isoladamente** (exceto glinkd em manutenção
dirigida). Links entre daemons (glinkd↔gdeliveryd, gs↔gdeliveryd) não
reconectam sozinhos de forma confiável: um gdeliveryd restartado solto faz o
login do cliente falhar com "servidor suspenso" até o restart completo.

Procedimento de restart completo (dentro do container):

```bash
/work/scripts/boot-server153.sh --stop     # kill -9 por comm, todos os daemons
/work/scripts/boot-server153.sh --fresh    # ⚠ APAGA STORAGE (ver abaixo)
/work/scripts/gs-server153.sh              # gs
```

`--fresh` **apaga os storages** (`dbhome/dbdata/dblogs/backup/dbhomewdb/
unamewdb/backupwdb` de gauthd, gamedbd, uniquenamed, gfaction) — **personagens
somem**. Use apenas para validação do zero. Para preservar personagens,
reinicie **sem** `--fresh`.

Nota: o WDB do gamedbd não tem recuperação de estado CORRUPT — um shutdown
sujo pode travar o boot dele; nesses casos o `--fresh` é o caminho (com a
perda de dados que ele implica).

## Troubleshooting rápido

| Sintoma | Causa provável | Ação |
| --- | --- | --- |
| Cliente: "servidor suspenso" | version/edition do challenge erradas, ou glinkd↔gdeliveryd quebrado por restart individual | conferir `version=10503`+`edition` no runtime conf; restart completo ordenado |
| Cliente: "erro de comunicação entre servidores" | gdeliveryd morto | `dmesg` (segfault?), watchdog renasca; se crash novo: [`09-BUGS-AND-FIXES.md`](09-BUGS-AND-FIXES.md) |
| Fica em "conectando" sem resposta | halfloginset estourou `ExceedHalfloginLimit` no glinkd | restart completo; investigar sessões presas |
| Login OK, criação com Decode Error | wire CreateRole divergente (ex.: campo `rank`) | teste de conformança `scripts/test-server153-wire.sh` |
| Criação OK, entrada falha `result=31 gameid=-1` | instância do gs morta (is61) ou worldtag 0 na role | `world2.log` do logservice; confirme os dois providers no `gdeliveryd.log` |
| glinkd morre ao renascer pelo watchdog | faltou o argumento de instância `1` | o script atual já passa; não iniciar glinkd à mão sem `1` |
| gamedbd: "Initialize storage environment failed" | storage copiado pela metade / estado sujo | limpar TODO o storage (staging CF fez isso — ver [`10-CLOUDFLARE.md`](10-CLOUDFLARE.md)) ou `--fresh` |
| gs sai com código -300 | falta `./config/aipolicy.data` (data files) | rodar via `gs-server153.sh` com o pacote v145 |
| Rebuild "passa" mas comportamento não muda | Makefile não viu a mudança de header | `make -B` + conferir mtime do binário ([`03-BUILD.md`](03-BUILD.md)) |

## Diagnóstico de crash (ferramentas)

```bash
# segfaults recentes no host:
sudo dmesg | grep -E "segfault" | tail
# backtrace de um core (binário com -g):
gdb -batch -ex "bt" /work/src/server153/cnet/gdeliveryd/gdeliveryd /path/core
# mapear endereço de crash no código (binário + addr2line):
addr2line -e /work/src/server153/cnet/gdeliveryd/gdeliveryd -f -C 0x<addr>
# attach em daemon travado (container precisa de SYS_PTRACE):
docker exec -it pw153-runtime gdb -p <pid> -batch -ex "thread apply all bt"
```
