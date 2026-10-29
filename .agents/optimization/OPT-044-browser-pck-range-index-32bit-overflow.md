---
id: OPT-044
title: "Validar aritmética de offsets HTTP Range acima de 2 GiB no probe browser PCK/PKX"
priority: P1
status: candidate
phase: post-browser-port
blocking: false
kind: static-hardening
source_sha: 0d8f802bea0603ebeb2723bbc886dc9555f12341
first_observed: 2026-10-28
validation: static-arithmetic-check-only
epic: e15-assets-streaming-perf-v1
scope: PR-23
---

# OPT-044 — Aritmética de offsets HTTP Range acima de 2 GiB

**Gate:** achado estático de robustez no *probe* recém-adicionado; **não há prova de falha no boot real, nem tamanho de PCK/PKX >= 2 GiB confirmado** nesta auditoria. Registrar como candidato pós-port, sem alteração de código. Se a instalação efetivamente exigir leitura acima do limite e o boot falhar, reproduzir com arquivo/offset/erro e promover para `.agents/issues` como blocker funcional (não como FPS).

## Evidência reproduzível de código

- [`scripts/client153-entry-browser-pre.js:26-28`](https://github.com/brasalabs6/pw-web/blob/0d8f802bea0603ebeb2723bbc886dc9555f12341/scripts/client153-entry-browser-pre.js#L26-L28): `BLOCK_SHIFT=20`, blocos de 1 MiB.
- [`scripts/client153-entry-browser-pre.js:45-49`](https://github.com/brasalabs6/pw-web/blob/0d8f802bea0603ebeb2723bbc886dc9555f12341/scripts/client153-entry-browser-pre.js#L45-L49): `const start = index << BLOCK_SHIFT` é coerção JavaScript **signed int32**, não multiplicação de inteiro seguro.
- [`scripts/client153-entry-browser-pre.js:73-85`](https://github.com/brasalabs6/pw-web/blob/0d8f802bea0603ebeb2723bbc886dc9555f12341/scripts/client153-entry-browser-pre.js#L73-L85): `>>> BLOCK_SHIFT` reduz posição a uint32 e o cálculo de `blockOffset` reutiliza o deslocamento signed.
- [`scripts/client153-entry-server.py:91-101`](https://github.com/brasalabs6/pw-web/blob/0d8f802bea0603ebeb2723bbc886dc9555f12341/scripts/client153-entry-server.py#L91-L101): a rota Range espera `bytes=N-M` com índices não negativos.

**Checagem aritmética independente (JavaScript, sem browser nem assets):** em `position=2147483648` (2 GiB), `index=2048`, `index << 20 = -2147483648`, mas o início correto é `2147483648`. Em `position=4294967296` (4 GiB), `position >>> 20 = 0`, mas o índice correto é `4096`. Portanto o código não representa corretamente offsets altos; **não foi demonstrado que a rota do cliente atinge esses offsets em pacotes reais**.

## Impacto e hipótese

Se um pacote montado tiver bytes necessários a partir de 2 GiB, o Range construído poderá ser inválido ou a leitura poderá usar offset errado, produzindo erro HTTP/short read, dados incorretos ou falha de carregamento. O uso de até 64 blocos por pacote também merece orçamento de RAM por dispositivo, já coberto pelos registros de memória existentes; **não atribuir FPS, throughput, consumo real ou crashes a este achado**.

## Plano de validação e benchmark (fase pós-browser-port)

1. Inventariar tamanhos reais de `*.pck/*.pkx` autorizados e os offsets efetivamente lidos no boot; não publicar assets. Se houver falha impeditiva acima de 2 GiB, antecipar **apenas a correção funcional**, com repro, para o port.
2. Criar fixture sintética esparsa com sentinelas antes/depois de 2 GiB e 4 GiB; testar `read(position,length)`, Range, bytes retornados, short read, limites de EOF, arquivo menor que 2 GiB e múltiplos blocos. Não alocar 4 GiB de RAM para o fixture.
3. Avaliar aritmética de `Math.floor(position / BLOCK_SIZE)` e `index * BLOCK_SIZE` com `Number.isSafeInteger`/limites explícitos, mantendo compatibilidade com os offsets suportados pelo formato e pelo FS.
4. Após o cliente funcional, comparar no **mesmo SHA/cena/dispositivo/rede** p50/p95/p99 de leitura, requisições HTTP, bytes transferidos, tempo até primeira tela, pico JS/WASM RAM e cache residente; testar Chrome desktop e Android físico. Não há baseline executado nesta rodada.

## Deduplicação

Não duplica [PW23-024](PW23-024-asset-copy-amplification-wasm.md) (cópias RAM), [PW23-033](PW23-033-wasm-heap-budget-and-memory-growth.md) (budget), [OPT-032](OPT-032-game-pck-shared-open-redecode-lifetime.md) (reabertura/decompressão) ou [OPT-036](OPT-036-pck-read-short-read-integrity-contract.md) (leitura do PCK legado). O recorte aqui é **aritmética de offsets HTTP Range no novo probe browser**. [PW23-020](../issues/PW23-020-assets-streaming-cache-cloudflare-pending.md) já cobre a ausência de pipeline de entrega completo, não este limite específico.

**Restrições:** nenhum tuning de código, PR adicional, auto-merge, mudança no PR #17 ou alegação de teste mobile. O gate principal continua o boot/render/gameplay real da Angelica 1.5.3 em desktop e celular.
