# ADR-0006 — Transporte do cliente: gateway WebTransport (HTTP/3) ↔ TCP glinkd

**Status:** proposto (arquitetura-alvo; implementação é milestone futuro do roadmap).
**Data:** 2026-10-25.

## Decisão

O cliente no navegador falará com o servidor PW 1.5.3 através de um **gateway de
transporte** que termina WebTransport (HTTP/3/QUIC) na borda e faz ponte byte a byte
para a porta TCP do `glinkd` (9001 no conf de runtime; 29000 no esquema clássico).
O gateway é um **pipe de bytes puro**: não reencoda, não interpreta e não altera o
framing GNET. O WebSocket seguro (WSS) permanece como transporte de linha de base e
de fallback; o WebTransport é o alvo de desempenho (menos overhead por pacote,
streams múltiplos, datagramas, reconexão mais rápida).

```
Browser (WASM/Emscripten, protocolo GNET original)
   │  WebTransport (H3/QUIC, streams bidirecionais)   ← transporte novo
   │  WSS binário                                      ← linha de base/fallback
   ▼
Gateway de transporte (serviço próprio, Rust ou Go)
   │  TCP raw, 1 sessão TCP por sessão de cliente
   ▼
glinkd (src/server153) → gdeliveryd → gamedbd/gs
```

## Motivação

- O browser não abre TCP raw; todo tráfego do cliente precisa de um terminador.
- O framing GNET (pacotes length-prefixed, camada de segurança com key exchange,
  compressão MPPC opcional) já é definido pelo código 1.5.3 commitado em
  `src/server153/cnet/glinkd/*.hpp` — o gateway apenas transporta esses bytes.
- WebTransport reduz head-of-line blocking e overhead vs. WebSocket sobre TCP+TLS,
  e oferece datagramas (interessante para movimento/posição no futuro).

## Restrições de plataforma (verificadas em 2026-10-25; fontes oficiais consultadas 2026-10-28)

- **Cloudflare Workers não terminam WebTransport server-side**. O índice
  oficial de Runtime APIs do Workers lista Fetch, TCP sockets (`connect`) e
  WebSockets como APIs de transporte do runtime — WebTransport não aparece:
  https://developers.cloudflare.com/workers/runtime-apis/
  A API de WebSocket (a baseline WSS) é documentada em
  https://developers.cloudflare.com/workers/runtime-apis/websockets/
- **Cloudflare Containers recebem tráfego via mediação Worker/DO**
  (`ctx.container.getTcpPort(port).fetch()/connect()`); inbound bruto de
  clientes não é o modelo. Fontes oficiais:
  https://developers.cloudflare.com/containers/ ("Container instances are
  spun up on-demand and controlled by code you write in your Worker") e
  https://developers.cloudflare.com/containers/api/durable-object-container/
  ("getTcpPort() returns a TCP port from the container. Use it to
  communicate with the container over TCP or HTTP."). Portanto: o gateway
  WebTransport roda como **serviço próprio** (VM/origem com UDP 443
  liberado); no Cloudflare, o caminho nativo hoje é WSS via Workers e TCP
  via `getTcpPort().connect()` — validado em produção no deploy deste
  projeto (ver `docs/SERVER153-REPORT.md` §6, endpoint `/tcpprobe`).

## Bibliotecas candidatas (gateway)

| Opção | Justificativa | Risco |
| --- | --- | --- |
| Rust `wtransport` (sobre quinn) | maturidade WebTransport server; casa com o ecossistema Rust já referenciado (`references/server-rust`, MIT) | API em evolução |
| Go `webtransport-go` | alternativa simples, bom para prototipagem | menos manutenção ativa |
| Node `@fails-components/webtransport` | prototipagem rápida | não indicado para produção |

Decisão operacional: **Rust com `wtransport`** como implementação de referência,
mantendo a camada de ponte (WebTransport↔TCP) isolada num crate próprio para permitir
trocar a implementação sem tocar no core.

## Preservação do protocolo e conformidade

1. O gateway NÃO conhece GNET: ponte bidirecional de bytes entre
   stream WebTransport e socket TCP (backpressure propagado).
2. Suíte de conformidade byte a byte (já prevista em `docs/ARCHITECTURE.md`):
   fixtures de pacotes GNET reais gravados do `gclient` (cliente virtual in-tree)
   contra `glinkd` local; o mesmo fixture roda sobre WSS e sobre WebTransport e
   deve produzir bytes idênticos no wire.
3. Critério de aceite: login → lista de personagens → entrada no mundo sobre
   WebTransport com wire byte a byte igual ao caminho TCP nativo do `gclient`.

## Migração e coexistência

- Fase 1 (atual): WSS binário (gateway mínimo) — baseline.
- Fase 2: gateway WebTransport em paralelo; cliente escolhe por capability
  detection (`window.WebTransport` presente + endpoint configurado).
- Fase 3: WebTransport padrão; WSS fallback (redes/proxies que bloqueiam UDP 443).
- O contrato entre cliente WASM e gateway é o mesmo nos dois transportes:
  `send(bytes)` / `onmessage(bytes)`; a camada de transporte é plugável.

## Consequências

- Dois terminadores de transporte a manter até a fase 3 (aceito: WSS é requisito
  de robustez de rede de qualquer forma).
- O gateway precisa de host com UDP/443 (fora do modelo atual de Workers/Containers
  do Cloudflare) — documentado acima; reavaliar quando a plataforma evoluir.
- Nenhuma mudança no protocolo do servidor: `glinkd` permanece intocado.
