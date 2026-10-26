# Relatório do servidor 1.5.3 (`src/server153`) — completude, tamanhos e descobertas de runtime

**Data:** 2026-10-26. **Build revision:** `7dde52fa03d3` (ver `src/server153/PROVENANCE.md`).
**Autorização:** `docs/LEGAL.md` (decisão 2026-10-25, escopo do port contratado).

## 1. Classificação da fonte

- **Derivada comunitária, não oficial.** Origem: `hrace009/PW-Source-1.5.3`
  (fork de `WolfsCore/PW-Source-1.5.3`, raiz `wladsoncedraz/Source-Server`).
  O primeiro commit disponível (25/10/2024) já descreve um servidor adaptado;
  correspondência com a versão oficial 1.5.3 **não é demonstrável** (análise do
  operador). HEAD `c283446` (2025-01-18) contém Makefiles stub não-construíveis
  e **não** é usado; a revisão de build é a `7dde52f` (pós-v145, Makefiles reais).
- **Conteúdo:** apenas o **servidor** em C++ (32-bit). Nada de cliente, nada de
  assets do jogo.

## 2. Completude — o que existe

| Componente | Estado | Observações |
| --- | --- | --- |
| Framework `share/` (io, rpc, storage, threads, perf) | completo | storage embutido (estilo Berkeley DB); **nenhum daemon usa MySQL** |
| `cnet/` — 8 daemons + ferramentas | completo | glinkd, gdeliveryd, gamedbd, gauthd, uniquenamed, gfaction, gacd, logservice; gclient (cliente virtual de teste) e dbtool |
| `cgame/` — servidor de jogo (`gs`) | código completo | binário constrói e inicia; **falha sem os data files** |
| `cskill/` — sistema de skills | completo | libskill.a/.so com 724 arquivos `.hrp` de dados de skill |
| `PWSL/` — linguagem de script | presente | não integrado ao build dos daemons |
| RPC gerado (`rpcdata/`, 668 arquivos) | commitado e consistente | `rpcgen` (Perl) valida igual em CI |
| Configs de runtime (`gamesys.conf`, `*.conf`) | presentes | com mistmatches entre vintages (ver §4) |
| Ferramentas admin (java/php/iweb) | ausentes | o MySQL do readme é para essas ferramentas, não para os daemons |

## 3. Completude — o que falta (para um realm jogável)

1. **Data files do jogo** (não estão nesta fonte): `elements.data`, `tasks.data`,
   `dyn_tasks.data`, `aipolicy.data`, `npcgen.data`, `world_targets.sev`,
   `extra_drops.sev`, `precinct.sev`, `region.sev`, `path.sev`, `gshopsev.data`,
   `gshopsev1.data`, `global_api.lua`, `map.bht`, `wmgj.nmd`, além de
   `movemap/`, `watermap/`, `airmap/` e o diretório `config/` — referenciados por
   `cgame/gs/gs.conf` `[Template]`. O `gs` inicia, carrega substâncias e sai com
   código -300 ao não encontrar `./config/aipolicy.data`. **Realm jogável é
   milestone separado** (assets autorizados pelo gate jurídico, mas não presentes
   nesta fonte).
2. **Ferramentas administrativas** (iweb/java/php) e o schema delas.
3. `sysauctionlist.txt` (leitura opcional do gdeliveryd; loga erro e continua).

## 4. Descobertas de runtime (bugs reais corrigidos por nós)

Três **violações de ODR** — o upstream nunca compilou a árvore inteira com flags
consistentes entre unidades de tradução. Cada uma causava segfault na inicialização:

1. **Locks stub vs. reais** (`share/common/mutex.h`): sem `_REENTRANT_`, Mutex/RWLock
   viram stubs no-op de 1 byte. `matcher.o` (via `gamed`, multi-thread) construía
   `Matcher::instance` com `cinput` em +32; `gdeliveryd.cpp` (SINGLE_THREAD=true)
   lia `cinput` em +4 — dentro do estado do rwlock — e `iconv_close(1)` segfaultava.
   Fix: `-D_REENTRANT_` nos dois branches de `share/mk/gcc.defs.mk`.
2. **`gnet_map` = hash_map vs. std::map** (`share/common/map.h`): daemons definem
   `USE_HASH_MAP`, objetos share (compilados primeiro via logclient) não;
   `PollIO::ionew` era construído como `std::map` e iterado como `hash_map` pela
   thread de poll (buckets NULL). Fix: `-DUSE_HASH_MAP` global.
3. **Wrappers ausentes no gauthd**: `certrequest.hpp`/`certkey.hpp` nunca foram
   commitados (criados seguindo o padrão); 6 esqueletos `.hrp` ProxyRpc
   referenciavam `GAuthClient`/`GameDBClient` inexistentes no gauthd (adaptados
   para o caminho de falha do framework).

Config/ambiente (documentados no `scripts/boot-server153.sh`):
- confs `gamesys.conf` de vintages diferentes discordam em portas (gauthd 9200 vs.
  29200; gacd 29702 vs. 29701/29709) — o boot script gera confs de runtime com os
  fixes documentados; os confs commitados não são alterados.
- `filters` (tabela de palavras sensíveis) é **GBK**, mas os confs declaram
  `table_charset=UCS2` — fix no conf de runtime.
- `gacd` carrega `libxml2.so.2` do próprio diretório se `LD_LIBRARY_PATH` apontar
  para ele (binário antigo commitado) — o boot só define `LD_LIBRARY_PATH` para o `gs`.
- `gs` precisa de `LD_LIBRARY_PATH` no próprio diretório (`libtask.so`, `libskill.so`).

## 5. Tamanhos medidos (2026-10-26)

| Item | Tamanho |
| --- | --- |
| Fonte vendored (working tree, 12.299 arquivos + 31 symlinks) | **141 MB** |
| Por diretório: `cnet` 72 MB, `cskill` 49 MB, `cgame` 13 MB, `share` 7 MB, `PWSL` ~1 MB | — |
| Upstream no GitHub (repo completo, API) | ~37 MB |
| Pacote de build `pw-server153-*.tar.gz` (binários+libs+configs+relatório) | **~122–128 MB** |
| `bin/` (10 binários com debug info `-g`): `gs` 58 MB, `gdeliveryd` 64 MB, `gamedbd` 27 MB, `glinkd` 14 MB, `gacd` 14 MB, `gfactiond` 10 MB, `uniquenamed` 7 MB, `gclient` 12 MB, `gauthd` 3,5 MB, `logservice` 2,5 MB | 203 MB |
| `lib/`: `libskill.so` 82 MB, `libskill.a` 137 MB, `libgsPro2.a` 12 MB, `libdbCli.a` 11 MB, demais ~5 MB | 240 MB |
| `conf/` (confs + xml coletados) | 932 KB |
| Imagem Docker de build (debian:12 + toolchain i386) | ~450–720 MB |
| Imagem Docker de runtime (debian:12 + libs i386) | ~117 MB + pacote |
| Monorepo: pack total após o vendor (`git count-objects -vH`) | 33,4 MiB |

Nota: binários mantêm debug info (`-g -O0`); `strip` reduzirá drasticamente para a
imagem de produção (validação de tamanho a medir na imagem final).

## 6. Reprodução

Ver `scripts/build-server153.sh` (build + pacote) e `scripts/boot-server153.sh`
(boot + verificação de portas). Ambos rodam em container Debian 12 com toolchain
i386 (ver workflow `.github/workflows/server153-build.yml` para a lista exata de
pacotes). Evidências de execução: PR do branch `feat/pw-source-1.5.3-ci-build`.
