# Rede e servidor

## Estado
PR #17 declara código C++ 1.5.3 derivado comunitário e build 32-bit, mas ainda requer validação de runtime e handshake. Não tratá-lo como fonte oficial comprovada. Não reescrever em Rust neste estágio.

## Rede
- Gateway browser encapsula protocolo do jogo preservando semântica e validação.
- WebSocket como baseline confiável; WebTransport é hipótese futura, condicionada a compatibilidade de infraestrutura, browser, transporte e ganho medido.
- Instrumentar RTT, jitter, packet size, bytes/s, filas, perdas percebidas, reconexões e timeouts.
- Delta compression e interest management somente onde não alterem semântica ou segurança.
- Interpolação/extrapolação visual limitadas; servidor mantém autoridade.
- Backpressure, limites de payload, rate limiting, autenticação e proteção contra replay.

## Servidor
- Primeiro validar processo completo, daemons, DB, realm, gauthd/glinkd, handshake e cenário jogável.
- Profiling CPU, lock contention, I/O, serialização, memória, tick, mapa e número de entidades.
- Otimizar queries, estruturas espaciais, cópias e filas somente após medir.
- Serviços novos podem usar Rust se houver benefício operacional; manter o core C++ até benchmark justificar migração.

## Aceite
Testes de protocolo golden, reconexão, carga concorrente, p50/p95/p99 de latência e integridade de estado. Comparar transportes em condições idênticas.
