# 12 — Segurança

**Data:** 2026-10-28 UTC · **Escopo:** chaves de sessão, autenticação, exposição de rede, segredos

## Chaves de criptografia entre daemons (isec/osec)

- Cada conexão daemon↔daemon declara `isec`/`iseckey` (entrada) e
  `osec`/`oseckey` (saída) — algoritmos do `share/io/security.h`
  (ARCFOUR/RC4, MD5, HMAC-MD5…; primitivas do `share/perf`).
- **As chaves são cruzadas entre pares**: `gdeliveryd.oseckey` =
  `gauthd.iseckey` e `gdeliveryd.iseckey` = `gauthd.oseckey` (seção
  `[GAuthServer]` do `gauthd.conf` ↔ `[GAuthClient]` do gamesys.conf).
- O `gauthd.conf` upstream carrega placeholders (456/123/123456) — o
  gdeliveryd tem chaves reais → stream decodifica lixo ("Protocol Unknown
  type=…" + loop de reconnect). O `scripts/boot-server153.sh` casa as
  chaves do gauthd às do gdeliveryd no conf de runtime.
- `shared_key` (seção `[GAuthServer]`) é a chave compartilhada
  gdeliveryd↔gauthd.
- **Regra**: nunca reproduzir valores de chave completos em doc/log/commit;
  os valores vivem nos confs de runtime e no boot script. Chaves de um
  servidor de teste local, mas a regra vale para qualquer segredo.

## Autenticação (dev-auth desta árvore)

- gauthd é o nó terminal de `UserLogin2`/`MatrixPasswd2` (ProxyRpc).
- Esquema atual (implementado em `gauthd/matrixpasswd.hrp`): contas
  **numéricas múltiplas de 16**, senha = número da conta;
  hash = `MD5(conta‖conta)`; resposta = `HMAC_MD5(desafio, hash)`;
  `algorithm=0` (sem cartão/handset/usbkey).
- **Limitações conhecidas** (aceitas para o estágio de validação):
  - sem verificação contra banco (MariaDB importado não é lido);
  - qualquer múltiplo de 16 com senha=conta autentica;
  - sem rate limiting, lockout ou 2FA.
- Substituir por auth real contra DB é milestone separado (o painel
  `scripts/panel-server153.py` é o rascunho de contas).
- `challenge_algo` (conf): 0 = MD5, 1 = SHA.

## Exposição de rede

| Superfície | Estado |
| --- | --- |
| glinkd 29000 | publicado só em `127.0.0.1` (container local) — cliente de teste via Wine |
| Cloudflare | container com `enableInternet: false`; sem TCP inbound público — todo acesso via mediação Worker/DO (ver [`10-CLOUDFLARE.md`](10-CLOUDFLARE.md)) |
| Portas internas (9200, 29100, 29300/29301, 29400/29401, 29500/29600, 29702/29712, 11100/11101) | bind 127.0.0.1 dentro do container/host — não expor publicamente |
| TLS | inexistente no TCP de jogo (o gateway ADR-0006 trará WSS/WebTransport na borda) |

## Anti-cheat (gacd)

- `gacd` escuta 29702 (do gdeliveryd) e 29712 (iweb/gacdclient); config em
  `gacd.xml` (punishmanager, statmanager, códigos, padrões de
  memória/processo/mouse).
- Protocolos: `ACQuestion`/`ACAnswer` (verificação ativa), `ACKickoutUser`,
  `ACReport(Cheater)`, `ACForbidProcess`, `ACReloadConfig`…
- Estado em memória; sem storage.

## GM e administração

- Comandos GM trafegam pelo glinkd (`GMKickoutUser`, `GMShutUp`,
  `GMListOnlineUser`, `GMRestartServer`, `GMForbidRole`…);
  `ProcessUserPrivilege` no gdeliveryd faz broadcast de privilégios quando
  `blIsGM != 0`.
- O binário gamedbd tem subcomandos administrativos locais (`query`,
  `listrole`, `merge`…) — executar só dentro do container/host confiável.
- A conta de teste `1000000`/`1000000` é de conhecimento público do time;
  não é segredo, mas não deve virar credencial de produção.

## Regras operacionais de segredos

1. Credencial, token ou chave que aparecer no caminho é sinalizado **sem
   reproduzir o valor**; nunca mover/copiar/imprimir segredos.
2. Não expor IPs internos, `lan_ip_list`, ou a topologia em artifacts
   públicos (a imagem CF não carrega IPs internos — tudo 127.0.0.1).
3. Novos segredos de produção (auth DB, chaves de gateway) entram via
   variável de ambiente/secret manager, nunca em conf commitado.
4. Logs de trace não registram senhas (só userid/retcode) — manter assim ao
   instrumentar.
