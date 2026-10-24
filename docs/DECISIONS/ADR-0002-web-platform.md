# ADR-0002 — Host browser com WebGL2 e C++/WASM

**Status:** aceito para prova técnica, revisável por medições.
**Data:** 2026-10-24.

## Decisão
Host estático JavaScript sem dependências de build nesta fase, prova de WebGL 2 e módulo C++ em Emscripten opcional. Renderização roda no cliente/browser; não é streaming de vídeo. Futuro backend WebGPU depende de benchmarks/compatibilidade.

## Motivação
Separar capacidade demonstrada da plataforma das dificuldades do código proprietário. Bootstrap rápido, reprodutível e testável em CI sem assets ou binários do jogo.

## Não decidido
Cliente final, renderer da Angelica, protocolo e server realm. Não extrapolar a demo para um port jogável.
