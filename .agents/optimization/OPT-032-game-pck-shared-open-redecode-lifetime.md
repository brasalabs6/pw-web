---
id: OPT-032
title: "Avaliar reabertura e descompressão repetida de arquivos compartilhados no AFilePackGame"
priority: P2
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

# OPT-032 — Avaliar reabertura e descompressão repetida de arquivos compartilhados no AFilePackGame

**Gate:** hipótese estática para **depois** que o cliente Angelica 1.5.3 rodar funcionalmente via WASM no navegador de PC e mobile; nenhuma alteração de código-fonte de tuning foi executada. Não alegar ganho de MB, taxa de compressão, FPS ou carga de browser real.

## Área
PCK / repetição de IO e decompression

## Evidência no HEAD auditado
[`src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp:805-866`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp#L805-L866) mostra `OpenSharedFile` sempre alocando novo `pFileData`, chamando `ReadFile` e criando `SHAREDFILE`; não consulta tabela de compartilhamento nesse método. [`src/client153/Angelica2/AngelicaFile/Source/AFileImage.cpp:95-110`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Angelica2/AngelicaFile/Source/AFileImage.cpp#L95-L110) o invoca para assets empacotados.

## Impacto possível (hipótese)
Se o mesmo asset é aberto simultaneamente, a implementação game pode repetir IO/zlib e buffers por referência, aumentando latência e pico de heap. **Não assumir vazamento ou múltiplas aberturas sem tracing**; esse caminho é distinto do cache de texturas GPU OPT-002.

## Experimento / estratégia posterior
Medir overlap temporal do mesmo path e opção de borrowed buffer com contagem de referências/copy-on-write segura; pool cache por SHA/versão/pack com teto por bytes. Não reusar ponteiros após CloseSharedFile e não confundir AFilePackGame com variante de editor AFilePackage.

## Plano de benchmark / validação
Abrir 1/10/100 vezes a mesma entrada sob leitura concorrente/serial; validar bytes, refcounts, lifespan e ASan; medir IO, inflate calls, RAM residente e tempo até modelo/scene.

**Critério de aceite:** demonstrar equivalência de bytes e hashes de arquivos, comportamento de loading e de gameplay, compatibilidade com formato legado; medir TTFP/TTI, bytes transferidos, compressão real, p50/p95/p99 de CPU/decode/I/O e pico RAM/VRAM quando disponível. Registrar SHA, dispositivo, rede/RTT, tamanho da amostra e before/after na mesma cena.

## Deduplicação e dependências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), fonte inspecionada `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`.
- Épico [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md).
- Checar PW23-020 (entrega/cloud), PW23-024 (cópias JS/WASM), PW23-027 (streaming do terreno), PW23-032 (política de confiança do manifest), OPT-002 (dedup de textura), OPT-003 (inflate do payload), OPT-014 (DDS legado) e OPT-024 (SGC de skills). Este registro trata exclusivamente o recorte descrito.
- Se confirmar vulnerabilidade crítica, crash ou falha impeditiva de port, reclassificar com reprodutor em `.agents/issues`; esta pasta não é autorização de adiamento de incidente real.
- Não abrir terceiro PR, não tocar PR #17 do servidor, não fazer auto-merge.
