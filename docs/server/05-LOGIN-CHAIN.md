# 05 — Cadeia de login (cliente → mundo)

**Data:** 2026-10-28 UTC · **Fontes:** traces `[glinkd]`/`[delivery]`/`[auth]`, `docs/SERVER153-REPORT.md` §4.2, memória de sessão

## Visão do fluxo

O login do cliente real PWBR 1.5.3 v145 atravessa quatro processos antes de
abrir a lista de personagens:

```
Cliente ──TCP 29000──▶ glinkd ──RPC──▶ gdeliveryd ──RPC──▶ gauthd (auth terminal)
   ▲                     │                 │
   └─── RoleList ◀───────┴── UserContainer ◀── gamedbd (roles/storage)
```

Passo a passo com os marcos reais de log (evidência 2026-10-27, conta de teste
`1000000`):

1. **Conexão + challenge** — o cliente conecta no glinkd (porta 29000) e
   recebe `challenge`. O cliente compara `challenge.version` com o seu
   `GAME_VERSION` (**0x010503**) e `challenge.edition` com a string que ele
   computa dos próprios data files (`%x%x%x%x%x` de elements/tasks/gshop).
   Divergência → diálogo "servidor suspenso". Fix: `version=10503` +
   `edition` pinada no conf de runtime do glinkd (patch no `glinkd.cpp`;
   o announce do gs respeita o pin).
2. **Response → MatrixPasswd2** — o cliente responde o challenge; o handler
   real é `glinkd/response.hpp` `Process()`, que monta o RPC
   (`RPC_MATRIXPASSWD` se AU version 0, `RPC_MATRIXPASSWD2` se não;
   `RPC_MATRIXTOKEN` quando `use_token`), e envia via
   `GDeliveryClient::SendProtocol`, mudando para `state_GResponseReceive`.
   Trace: `[glinkd] … matrixpasswd … au_version=…`.
3. **gdeliveryd encaminha ao gauthd** — `gdeliveryd/matrixpasswd2.hrp`
   `Delivery()` → `GAuthClient::GetInstance()->SendProtocol(*this)`.
   Trace: `[delivery] matrixpasswd2 forward ok`.
4. **gauthd decide** — `gauthd/matrixpasswd2.hrp` `Process()` (nó terminal
   ProxyRpc — ver [`04-PROTOCOL.md`](04-PROTOCOL.md)) computa o resultado
   com o esquema dev-auth (abaixo) e responde.
   Trace: `[auth] receive matrixpasswd. retcode=0,userid=1000000`.
5. **userlogin2 de volta** — `gdeliveryd/userlogin2.hrp` `PostProcess` →
   `UserContainer::UserLogin` (registra status `_STATUS_READYLOGIN`,
   guarda iseckey/oseckey, avalia GM, faz `KickoutUser` de sessão
   anterior) → `glinkd/userlogin2.hrp` `Client` entrega o resultado ao
   cliente. Trace: `[glinkd] userlogin2 retcode=0 userid=1000000`.
6. **OnlineAnnounce** — `glinkd/onlineannounce.hpp` `Process` faz
   `ChangeState(state_GRoleList)` + `Send` ao cliente; o gdeliveryd
   registra o announce (o `StatusAnnounce` correspondente no gdeliveryd já
   teve seu próprio bug de rwlock — finding em
   [`09-BUGS-AND-FIXES.md`](09-BUGS-AND-FIXES.md)).
7. **RoleList** — o cliente pede a lista de roles; o gdeliveryd consulta o
   gamedbd (storage `base`/`status` por roleid); lista vazia decodifica
   limpo e abre a tela de criação.
8. **CreateRole** — o cliente envia `CreateRole` (type=84, corpo **1704
   bytes** no formato de 23 campos do PWBR 2018; ver
   [`04-PROTOCOL.md`](04-PROTOCOL.md)). O gamedbd cria a role via
   `dbcreaterole.hrp` (worldtag default 1 quando 0) e o template de spawn
   vem do storage `clsconfig` (importado a cada boot do gamedbd de
   `cnet/gamedbd/clsconfig`).
9. **EnterWorld** — o gdeliveryd resolve o gameserver pela worldtag da role
   (`map.cpp` gameservermap), despacha `EnterWorld` (prioridade 101) e
   anuncia o player online. Traces:
   `[enterworld] roleid=1024 Process entered → lock acquired → dispatching
   to gameid=101`. O gs aceita o player no mundo (worldtag 161 → instância
   is61) e o logservice registra em `world2.log`
   (`gameserver 101, 用户数目:1` — "user count: 1", GBK).
10. **Logout / re-login** — ao fechar o cliente, `RoleLogout`/`UserLogout`
    rodam no gdeliveryd; re-login rápido serializa contra eles pelo write
    lock do UserContainer (fix do use-after-free, commit `f6e0abb7`).

## Esquema dev-auth (gauthd desta árvore)

- Contas **numéricas múltiplas de 16**; senha = número da conta.
  Conta de teste: `1000000` / `1000000`.
- Hash armazenado = `MD5(conta‖conta)`; resposta = `HMAC_MD5(desafio, hash)`.
- Implementado no `Process()` terminal de `gauthd/matrixpasswd.hrp`
  (espelha `GQueryPasswd`); **não lê MariaDB** — "criar conta" neste
  esquema é gerar/validar credenciais, sem insert no banco. Substituir por
  auth real contra DB é milestone separado (o painel de contas em
  `scripts/panel-server153.py` é o rascunho).

## Tabela de retcodes e erros observados

| Código / sintoma | Significado | Ação |
| --- | --- | --- |
| userlogin2 `retcode=0` | login OK | — |
| userlogin2 `retcode=10` | multi-login (sessão anterior ainda no usermap) | retry após cleanup do StatusAnnounce; não é erro de credencial |
| sponsor send falha `21` (`ERR_DELIVER_SEND`) | RPC não saiu do glinkd — tipo ausente no state array da sessão | conferir `_state_GDeliverClient` (bug histórico dos IDs 550/8066/8070) |
| `8` (`ERR_COMMUNICATION`) ao cliente | hop gdeliveryd→gauthd falhou | conferir `_state_GAuthClient` do gdeliveryd + chaves iseckey/oseckey |
| precreaterole `retcode=45` (`ERR_DUPLICATRECORD`) | nome de personagem já registrado | escolher outro nome |
| login `result=31`, `gameid=-1` | nenhum gameserver atende a worldtag da role | instância do gs morta (crash is61) ou worldtag 0 na role (fix `dbcreaterole.hrp`) |
| "servidor suspenso" no cliente | version/edition do challenge rejeitados | conferir `version=10503` + `edition` pinada no conf do glinkd |
| "erro de comunicação entre servidores" | gdeliveryd morto → todos os links caem | ver dmesg/segfault; watchdog renasca; investigar em [`09-BUGS-AND-FIXES.md`](09-BUGS-AND-FIXES.md) |
| Decode Error type=84 | wire de CreateRole divergente | teste de conformança (`11-TESTING.md`) |

## Instrumentação de trace

Traces incondicionais `fprintf(stderr)` com prefixos `[glinkd]`, `[delivery]`,
`[auth]` existem nos pontos: `response.hpp` (envio RPC + falha),
`userlogin2.hrp` (retcode), `onlineannounce.hpp` (mudança de estado),
`matrixpasswd2.hrp` (forward), `userlogin2.hrp PostProcess`,
`gauthd/matrixpasswd2.hrp`. No gdeliveryd, `[enterworld]` marca
`Process entered / acquiring / lock acquired / dispatching to gameid=%d`.

Logs são append-only entre restarts — **conte o delta de linhas** antes/depois
de um probe antes de concluir algo (665 linhas `retcode=21` antigas já
confundiram um diagnóstico).

## gclient (cliente virtual) vs cliente real

- `gclient` (in-tree) prova a cadeia de RPCs sem cliente gráfico:
  `scripts/probe-server153.sh` roda `gclient` contra `127.0.0.1:29000` e
  exige `OnAddSession` + processamento no glinkd. Cuidados: `gclient.conf`
  exige `start <= end` no `[Account]` (senão gera zero contas e fica mudo);
  a criação de role via gclient usa `makechoice.h` com prompt interativo.
- **Sucesso do gclient NÃO é sucesso do cliente real**: o gclient não exerce
  o wire do binário PWBR 2018 (foi exatamente o caso do campo `rank` —
  gclient passava, cliente real dava Decode Error). Toda afirmação de
  compatibilidade exige evidência do cliente real ou do teste de
  conformança de wire.
