# 11 — Testes e validação

**Data:** 2026-10-28 UTC · **Driver:** `scripts/verify.sh` · **Conformance:** `scripts/test-server153-wire.sh`

## Filosofia

- **Evidência antes de promessa**: toda afirmação de sucesso tem comando +
  output real. O que não rodou é `NOT RUN`; o que falhou é `FAIL`; sem
  ferramenta é `BLOCKED`; **zero casos nunca é PASS**.
- Testes de comportamento exercitam o código real (marshal/unmarshal da
  source, sockets reais, daemons reais) — sem mocks do protocolo.

## Suite atual

### 1. Conformância de wire — CreateRole (a mais importante)

`scripts/test-server153-wire.sh` compila `tests/server153_createrole_wire.cpp`
com `g++ -m32` contra os headers **reais** da source
(`share/io`, `share/common`, `cnet/rpcdata`, `cnet/inl`) e prova:

1. Um pacote no estilo do cliente PWBR 1.5.3 v145 (Build 2555) marshal para
   **exatamente 1704 bytes** (formato de 23 campos, sem `rank`).
2. Os **primeiros 64 bytes** são idênticos à captura real do cliente
   (dump `[decode]` do glinkd, 2026-10-27).
3. O **unmarshal do servidor consome o pacote limpo** (sem exceção, sem
   leitura além do fim).

Guarda contra a reintrodução do 24º campo (`Octets rank`, upstream `8aa72d3`)
que este cliente não envia (bug B09 em
[`09-BUGS-AND-FIXES.md`](09-BUGS-AND-FIXES.md)).

Execução: roda no `verify.sh`; procura primeiro `g++ -m32` no host, senão
dentro do container `pw153-runtime`. Sem toolchain 32-bit em lugar nenhum:
**exit 2 = BLOCKED** (não é PASS por skip).

### 2. Probe de handshake GNET

`scripts/probe-server153.sh` — sobe o cliente virtual in-tree `gclient`
contra o glinkd em `127.0.0.1:29000` e exige:

- porta 29000 escutando (via `/proc/net/tcp`);
- `OnAddSession` no log do gclient (sessão GNET estabelecida);
- processamento de protocolo no glinkd (`receive response from client` /
  `receive matrixpasswd` no log).

### 3. Health do stack (runtime)

- Local: `boot-server153.sh` reporta ALIVE/DEAD por daemon + portas esperadas.
- Cloudflare: `GET /health` (estático), `GET /cgi-bin/health.sh`
  (`daemons_alive:8`), `GET /tcpprobe` (TCP de mediação ao glinkd) — ver
  [`10-CLOUDFLARE.md`](10-CLOUDFLARE.md).

### 4. Verificação geral do repo

`scripts/verify.sh` (o guardião):

```
== shell syntax ==            bash -n em todos os scripts/*.sh
== standard-library Python == unittest em tests/test_*.py
== portable C++ core ==       engine/core (nativo, -Werror)
== server153 CreateRole wire conformance ==
```

O wire test trata `rc=2` como SKIP (BLOCKED documentado — a imagem CI do
server tem gcc-multilib, então lá ele roda de verdade) e qualquer outro
rc≠0 como FAIL do verify.

## O que a suite NÃO cobre (honestidade)

- Não há teste automatizado da cadeia de login inteira (precisa do stack
  vivo; o probe cobre só o handshake).
- Não há teste de movimento/persistência (provado manualmente com o cliente
  real; evidência em `world-entry-proof.txt` no scratch do goal).
- Builds TSan/ASan ainda não fazem parte da suite (milestone separado).

## Reprodução

```bash
./scripts/verify.sh                        # suite geral (host)
./scripts/test-server153-wire.sh           # wire isolado
./scripts/probe-server153.sh                # probe contra o stack vivo
docker exec pw153-runtime bash -c 'for p in /proc/[0-9]*/; do c=$(cat $p/comm 2>/dev/null); case $c in gauthd|gdeliveryd|gamedbd|uniquenamed|gfactiond|gacd|logservice|glinkd) echo "$c ALIVE";; esac; done'
```
