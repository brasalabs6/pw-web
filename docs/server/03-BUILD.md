# 03 — Build do servidor

**Data:** 2026-10-28 UTC · **Driver:** `scripts/build-server153.sh` · **CI:** `.github/workflows/server153-build.yml`

## Toolchain

- **Compilador:** `g++ -m32 -std=gnu++11 -fpermissive` (32-bit; ABI i386).
- **Imagem de build:** Debian 12 (`debian:12`) com:
  ```
  dpkg --add-architecture i386
  apt-get install -y build-essential gcc-multilib g++-multilib \
      libssl-dev:i386 libpcre3-dev:i386 libxml2-dev:i386 perl git
  ```
  (`libssl-dev:i386` e `libpcre3-dev:i386` são exigidos no link final;
  `libxml2-dev:i386` cobre o gacd.)
- Flags efetivas incluem `-g -O0` (debug info mantida; `strip` é aplicado só
  na imagem Cloudflare — ver [`10-CLOUDFLARE.md`](10-CLOUDFLARE.md)).

## Estágios (ordem obrigatória)

O driver `scripts/build-server153.sh --jobs 2` executa estágios críticos
(falha aborta) e soft (falha conta no relatório):

| # | Estágio | O que produz |
| --- | --- | --- |
| 1 | `setup` | symlinks de libs esperadas pelos Makefiles (`iolib/libgsio.a → ../cnet/io/...`, `cskill/libskill.a → skill/...`) |
| 2 | `perf` | `share/perf/libperf.a` |
| 3 | `share-objs` | objetos base de `share/io` e `share/common` (`pollio.o`, `protocol.o`, `security.o`, `rpc.o`, `proxyrpc.o`, `octets.o`, `thread.o`, `conf.o`, `timer.o`, `itimer.o`) |
| 4 | `share-io` | framework `share/io` |
| 5 | `logclient` | `cnet/logclient/liblogCli.a` (`make -f Makefile.gs lib`) |
| 6 | `gamed` | `cnet/gamed/libgsPro2.a` |
| 7 | `gdbclient` | `cnet/gdbclient/libdbCli.a` |
| 8 | `cskill` | `libskill.a` + `libskill.so` (ver abaixo) |
| 9 | `libcommon` | `cgame/libcommon.a` |
| 10 | `collision` | `cgame/collision/libTrace.a` |
| 11 | `cgame-lib` | libs do `cgame/` |
| 12 | `tasklib` | `cgame/gs/task/libtask.so` (`make lib` — o alvo `all` só gera `.o`) |
| 13 | `gs` | binário `cgame/gs/gs` |
| 14 | `daemons` | glinkd, gdeliveryd, gamedbd, gauthd, uniquenamed, gfaction, gacd, logservice (soft — um daemon pode falhar sem abortar os outros) |
| 15 | `gclient` | cliente virtual de teste (soft) |
| 16 | `rpcgen` | valida que o `cnet/rpcdata` commitado bate com a saída do `rpcgen rpcalls.xml` (soft; pula sem Perl/XML::DOM) |
| 17 | `package` | `dist/server153/pw-server153-<sha>.tar.gz` (binários + libs + confs + BUILD-REPORT) |

Relatório por estágio: `dist/server153/BUILD-REPORT.txt`; log por estágio:
`dist/server153/logs/<estágio>.log`.

## Workarounds de Makefile (empíricos, obrigatórios)

Os Makefiles upstream **não são confiáveis**; estas regras foram descobertas
na prática e estão codificadas no driver:

1. **`cskill`**: o Makefile da raiz (`Makefilelib`/`Makefile2`) referencia
   caminhos inexistentes. O build real é
   `make -C cskill/skill -f Makefilelib lib dlib` + symlinks de
   `libskill.a/.so` na raiz de `cskill` (iolib e cgame/gs esperam lá).
2. **`libtask.so`**: vem de `make -C cgame/gs/task lib` — o alvo `all`
   daquele diretório só compila `.o`.
3. **`cnet/makefile`** (minúsculo) é o makefile real dos daemons; os
   `execdirs` são glinkd, gdeliveryd, gauthd, gamed, gamedbd, gdbclient,
   gfaction, uniquenamed.
4. **`cgame/io/Makefile`** constrói `pollio.o`.
5. **Preflight do driver** rejeita árvores erradas: `cgame/Rules.make` deve
   ter `RULES_DIR` (fix de caminho portável) e `cnet/glinkd/Makefile` deve
   ter `TOP_SRCDIR` (Makefile stub da revisão pós-2025 indica revisão errada).

## ⚠️ Makefiles não rastreiam dependências de headers

**Depois de editar qualquer header (`rpcdata/`, `mutex.h`, `hashtab.h`, …),
reconstrua com `make -B`** os daemons afetados. O `make` comum não suja os
`.o` — o estágio "passa" em 0 s com o binário antigo rodando. Isso já causou
falsa conclusão de fix aplicada (o cliente real continuou com Decode Error
após a correção de `RoleInfo`; o glinkd tinha mtime de 6 h antes).

Verifique sempre o mtime do binário após rebuild:

```bash
ls -l --time-style=full-iso src/server153/cnet/glinkd/glinkd
```

Exemplos de rebuild dirigido (dentro do container `pw153-runtime`, que tem o
toolchain i386):

```bash
cd /work/src/server153
make -B -C cnet/glinkd       # glinkd (consome rpcdata/roleinfo)
make -B -C cnet/gdeliveryd    # gdeliveryd (consome mutex.h, map.cpp, enterworld.hpp)
make -B -C cnet/gamedbd       # gamedbd (consome clsconfig, dbcreaterole.hrp)
```

Rebuild dos três daemons principais leva ~520 s no container. Após rebuild,
reinicie na ordem (ver [`06-OPERATIONS.md`](06-OPERATIONS.md)).

## Fixes de compilação que a árvore exige (ODR)

O upstream nunca compilou a árvore inteira com flags consistentes entre
unidades de tradução; cada violação abaixo causava segfault na inicialização
e foi corrigida nos `share/mk/gcc.defs.mk`:

1. **`-D_REENTRANT_`** nos dois branches: sem ele, `Mutex`/`RWLock` viram
   stubs no-op de 1 byte; unidades compiladas com layouts divergentes
   segfaultavam (ex.: `gamed` multi-thread construía `Matcher::instance` com
   `cinput` em +32; `gdeliveryd.cpp` single-thread lia em +4).
2. **`-DUSE_HASH_MAP`** global: daemons definiam, objetos de `share/`
   (via logclient) não; `gnet_map` era `std::map` de um lado e `hash_map` do
   outro — `PollIO::ionew` iterava buckets NULL.
3. **Wrappers ausentes no gauthd**: `certrequest.hpp`/`certkey.hpp` nunca
   foram commitados upstream; criados seguindo o padrão dos vizinhos, e os 6
   esqueletos `.hrp` ProxyRpc que referenciavam `GAuthClient`/`GameDBClient`
   foram completados.

## CI (GitHub Actions)

`.github/workflows/server153-build.yml` — dispara em push/PR que tocam
`src/server153/**`, `scripts/build-server153.sh` ou o próprio workflow:

1. Container `debian:12` + toolchain multilib (lista acima).
2. `./scripts/build-server153.sh --jobs 2`.
3. Artifacts: `pw-server153-report` (BUILD-REPORT, sempre),
   `pw-server153-package` (tar.gz, sempre), `pw-server153-logs` (só em falha).

Estado: workflow verde no branch (5/5 checks no commit final do goal;
pacote ~120 MiB).

## Tamanhos medidos (2026-10-27 20:32 UTC)

| Item | Tamanho |
| --- | --- |
| Árvore `src/server153` com artefatos de build | 520 MB (`cnet` 279 MB, `cgame` 119 MB, `cskill` 111 MB, `share` 12 MB) |
| Source pura (sem build) | 141 MB |
| Pacote `pw-server153-*.tar.gz` (CI/local) | ~120 MiB |
| Binários com `-g`: `gs` 33 MB, `gdeliveryd` 37 MB, `libskill.so` 34 MB | — |
| Imagem Docker de runtime (debian:12 + libs i386 + árvore) | 480 MB |
| Repo completo no GitHub (upstream) | ~37 MB |

## Reprodução do build completo

```bash
# no host (com docker), ou dentro de qualquer Debian 12 com multilib:
./scripts/build-server153.sh --jobs 2        # build + package + BUILD-REPORT
cat dist/server153/BUILD-REPORT.txt           # todos os estágios devem ser PASS
ls dist/server153/pw-server153-*.tar.gz       # pacote
```
