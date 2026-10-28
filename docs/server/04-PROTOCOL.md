# 04 — Protocolo GNET

**Data:** 2026-10-28 UTC · **Fontes:** `share/io/protocol.h`, `share/common/marshal_i386.h`, `share/io/security.h`, `cnet/rpc.h`, `share/io/proxyrpc.h`, capturas reais de pacotes

## Framing de pacote

Todo pacote GNET no wire é:

```
CompactUINT(type) + CompactUINT(size) + corpo
```

- `Protocol::Encode` (`share/io/protocol.h:101-107`) serializa o protocolo em
  um `OctetsStream` interno (`data << *this`) e escreve
  `CompactUINT(type) << data`; o marshal de `Octets` prefixa o próprio
  tamanho (`CompactUINT(size)` — `marshal_i386.h:469`).
- O decode (`protocol.h:523-529`) lê `type` e `size` primeiro (com
  `Marshal::Begin/Rollback` para não consumir em caso de erro) e valida o
  tamanho antes de decodificar o corpo — pacote maior que o esperado lança
  `Marshal::Exception` → **"Decode Error"** no log (foi a assinatura do bug
  B09: `Protocol(type=84,size=1704) Decode Error`).
- O stub do tipo (`Protocol::GetStub(type)` → `Clone()`) reconstrói o
  protocolo; tipo desconhecido é descartado.

## CompactUINT / CompactSINT (encoding compacto)

Implementação real (`share/common/marshal_i386.h`, `compact_uint32`):

| Valor | Wire | Regra |
| --- | --- | --- |
| `x < 0x80` | 1 byte `x` | bit alto 0 |
| `x < 0x4000` | 2 bytes `byteorder_16(x \| 0x8000)` | prefixo `10` |
| `x < 0x20000000` | 4 bytes `byteorder_32(x \| 0xc0000000)` | prefixo `110` |
| else | 5 bytes `0xe0` + `byteorder_32(x)` | escape |

Unmarshal (`uncompact_uint32`): olha os 3 bits altos do primeiro byte
(`& 0xe0`): `0xe0` → escape de 5 bytes; `0xc0` → 4 bytes (máscara
`~0xc0000000`); `0xa0`/`0x80` → 2 bytes (máscara `~0x8000`); senão 1 byte.

`CompactSINT` (síntese de signed): positivos como acima com limites
`0x40`/`0x2000`/`0x10000000`; negativos usam os marcadores `0x40` (1 byte),
`0xa000` (2), `0xd0000000` (4), `0xf0`+4 bytes, com o valor absoluto.

**Implicação para o port browser**: o framing é byte a byte — qualquer
gateway (WebSocket/WebTransport → glinkd) preserva esses bytes sem
reinterpretar nada (ADR-0006).

## Handshake de sessão (cliente ↔ glinkd)

1. **Challenge** — o glinkd envia nonce + `version` + `edition`
   (`glinkserver.cpp:39-51`). O cliente PWBR 2555 exige
   `version == 0x010503` (conf: `version=10503`, lido como hex em
   `glinkd.cpp:64`) e `edition` igual à string que ele computa dos próprios
   data files (`%x%x%x%x%x` de elements/tasks/gshop — a mesma string que o
   gs anuncia via `AnnounceProviderID`). A edition é pinável no conf
   (`glinkd.cpp:69-81`); se vazia, o glinkd adota a do gs.
2. **Response** — handler real: `glinkd/response.hpp` `Process()`; monta o
   RPC de senha (`RPC_MATRIXPASSWD` se `GetAUVersion()==0`, senão
   `RPC_MATRIXPASSWD2`; `RPC_MATRIXTOKEN` quando `use_token`) e envia ao
   gdeliveryd (`state_GResponseReceive`).
3. **KeyExchange / KeyReestablish** — chaves de sessão do link.
4. **userlogin2 / OnlineAnnounce** — resultado do auth + mudança para
   `state_GRoleList` (passo a passo em [`05-LOGIN-CHAIN.md`](05-LOGIN-CHAIN.md)).

`challenge_algo` (conf `[GDeliveryServer]`): `0` = MD5, `1` = SHA — decide
resposta vs. challenge no protocolo de senha.

## RPC e ProxyRpc

- **`Rpc`** (`share/io/rpc.h`): chamada com `xid` (transaction id), argumento
  e resultado; `Server()` roda no nó que executa, `Client()` no sponsor.
  `Rpc::Call` enfileira; a resposta volta pelo mesmo caminho.
- **`ProxyRpc`** (`share/io/proxyrpc.h`): estende `Protocol`, **não** `Rpc` —
  um `Server()` override nunca roda para ele. O nó terminal precisa
  implementar `Process()` com o bookkeeping completo:
  1. guard `xid.IsRequest()`, senão `xid.ClrRequest()`;
  2. setar `proxy_manager`/`proxy_sid`/`proxy_xid`; `xid.SetRequest()`;
  3. desmarshal do argumento de `proxy_data`; computar;
  4. `SetResult(res)`; `SendToSponsor()`.
  Sem isso, `SendToSponsor` é no-op (`proxy_manager==NULL`) e a resposta
  some em silêncio. A base `Process()` só encaminha via `Delivery()` (falha
  → `ERR_DELIVER_SEND` = 21).
- **Cadeia de login** é ProxyRpc em dois hops: glinkd (sponsor) →
  gdeliveryd (proxy) → gauthd (terminal: `UserLogin2`, `MatrixPasswd2`).

## Despacho e prioridades

- Cada protocolo tem um `PriorPolicy` (`protocol.h:573`, default **0**). O
  `Dispatch` (`protocol.h:455-468`) é binário: prioridade **0** → a task roda
  **inline na thread de poll (I/O)**; prioridade ≠ 0 → entra na fila do
  `Thread::Pool` com essa prioridade.
- No gdeliveryd, quem sobrescreve `PriorPolicy() { return 101; }` são os RPCs
  de autenticação: `userlogin2.hrp:30`, `userlogin.hrp`,
  `matrixpasswd2.hrp:29`, `matrixpasswd.hrp`, `gquerypasswd.hrp:27`,
  `matrixtoken.hrp`, `playerpositionresetrqst.hrp`.
- `EnterWorld`, `CreateRole`, `RoleList` (classes `GNET::Protocol`) herdam
  prioridade 0 → rodam inline na thread de poll. Com o rwlock corrompido
  (bug B12), a thread de poll travava no lock do EnterWorld e o worker
  único da fila 101 travava no mesmo lock acumulando os RPCs de auth
  (TaskQueueSize=7 observado ao vivo). O runtime conf
  `(1,3)(100,1)(101,4)(0,1)` dá headroom na fila de auth (4 workers).

## Criptografia de stream (isec/osec)

`share/io/security.h` define os algoritmos de sessão por direção:

| ID | Algoritmo |
| --- | --- |
| 0 | RANDOM |
| 1 | NULLSECURITY |
| 2 | ARCFOURSECURITY (RC4) |
| 3 | MD5HASH |
| 4 | HMAC_MD5HASH |
| 5+ | (HMAC variants, ver enum) |

Cada seção de conexão nos confs carrega `isec`/`iseckey` (entrada) e
`osec`/`oseckey` (saída) — os dois lados precisam de chaves cruzadas:
`delivery.oseckey` = `gauthd.iseckey`, `delivery.iseckey` =
`gauthd.oseckey`. Chaves erradas produzem lixo decodificado
("Protocol Unknown type=…" e loop de reconnect — foi o bug das chaves
placeholder 456/123 no `gauthd.conf` upstream). Primitivas (AES, MD5, SHA1,
RC4, MPPC) vêm do `share/perf` em assembly i386.

## Compressão

- `compress = 0` no `[GLinkServer1]` (desligada para clientes).
- MPPC (`share/io/mppc.h`, `ppp_mppe_mppc.c`) existe no framework para
  streams comprimidos entre daemons.

## Strings e encoding

- Dados do jogo e logs usam **GBK**; nomes de player vão no wire como
  octets com charset controlado por `name_charset` (UCS2 no gs.conf).
- A tabela de palavras sensíveis `filters` é GBK sem BOM — os confs
  upstream declaram `table_charset=UCS2` (bug: conversão produzia lixo e os
  daemons não subiam); o runtime corrige para `GBK`.
- Conversão: `CharsetConverter` (`cnet/common/conv_charset.h`).

## Pacotes conhecidos com evidência real

| type | Protocolo | Evidência |
| --- | --- | --- |
| — | Challenge/Response/KeyExchange | traces `[glinkd]` do login real |
| 550/8066/8070 | RPC_MATRIXPASSWD/2/TOKEN | IDs que faltavam nos state arrays (bug B05) |
| 84 | CreateRole | captura real 1704 bytes (23 campos) — teste de conformança reproduz byte a byte |
| — | EnterWorld | traces `[enterworld] roleid=1024 … dispatching to gameid=101` |
| — | AnnounceProviderID | `gameserver 1 … worldtag is 1` / `gameserver 101 … worldtag is 161` no gdeliveryd.log |
