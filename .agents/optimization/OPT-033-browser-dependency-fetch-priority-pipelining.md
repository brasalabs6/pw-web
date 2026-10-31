---
id: OPT-033
title: "Pipelinar dependências da cena Web com concorrência e prioridades limitadas"
priority: P1
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c
first_observed: 2026-10-28
validation: static-only
epic: e15-assets-streaming-perf-v1
scope: PR-23
---

# OPT-033 — Pipelinar dependências da cena Web com concorrência e prioridades limitadas

**Gate:** hipótese estática para **depois** que o cliente Angelica 1.5.3 rodar funcionalmente via WASM no navegador de PC e mobile; nenhuma alteração de código-fonte de tuning foi executada. Não alegar ganho de MB, taxa de compressão, FPS ou carga de browser real.

## Área
Browser / network waterfall / first frame

## Evidência no HEAD auditado
[`web/client153/app.mjs:16-37`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/web/client153/app.mjs#L16-L37) espera o download e parse do MOX e depois percorre cada mesh sequencialmente com `await fetchAsset(asset)` e upload para GPU antes de reportar READY. [`web/client153/app.mjs:7-14`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/web/client153/app.mjs#L7-L14) faz SHA de cada asset e passagem para WASM.

## Impacto possível (hipótese)
A waterfall serial pode aumentar o tempo de primeiro frame em latência alta mesmo quando imagens independentes poderiam ser baixadas/preparadas em paralelo. Não duplicar OPT-002 (cache/dedup por hash): este item trata **escalonamento e prioridade** após resolução de dependências.

## Experimento / estratégia posterior
Após o port funcional e dedup de OPT-002, testar fila de fetch/verify/decode/upload com limite de concorrência por rede/memória, prioridade ao avatar/UI/área visível e cancelamento de scene switch; executar uploads sob budget CPU/GPU por frame, jamais exibir recursos não verificados.

## Plano de benchmark / validação
20/100 DDS distintas sob RTT 25/100/300 ms e rede 1/5/30 Mbps; waterfall, TTFP, long tasks, heap/VRAM, retries e cancel; comparar serial/concurrency 2/4/8 evitando OOM mobile.

**Critério de aceite:** demonstrar equivalência de bytes e hashes de arquivos, comportamento de loading e de gameplay, compatibilidade com formato legado; medir TTFP/TTI, bytes transferidos, compressão real, p50/p95/p99 de CPU/decode/I/O e pico RAM/VRAM quando disponível. Registrar SHA, dispositivo, rede/RTT, tamanho da amostra e before/after na mesma cena.

## Deduplicação e dependências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), fonte inspecionada `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`.
- Épico [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md).
- Checar PW23-020 (entrega/cloud), PW23-024 (cópias JS/WASM), PW23-027 (streaming do terreno), PW23-032 (política de confiança do manifest), OPT-002 (dedup de textura), OPT-003 (inflate do payload), OPT-014 (DDS legado) e OPT-024 (SGC de skills). Este registro trata exclusivamente o recorte descrito.
- Se confirmar vulnerabilidade crítica, crash ou falha impeditiva de port, reclassificar com reprodutor em `.agents/issues`; esta pasta não é autorização de adiamento de incidente real.
- Não abrir terceiro PR, não tocar PR #17 do servidor, não fazer auto-merge.
