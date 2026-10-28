---
id: OPT-038
title: "Incluir assets fora de element/*.pck na análise de peso e prioridades de download"
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

# OPT-038 — Incluir assets fora de element/*.pck na análise de peso e prioridades de download

**Gate:** hipótese estática para **depois** que o cliente Angelica 1.5.3 rodar funcionalmente via WASM no navegador de PC e mobile; nenhuma alteração de código-fonte de tuning foi executada. Não alegar ganho de MB, taxa de compressão, FPS ou carga de browser real.

## Área
Inventory / scope/coverage

## Evidência no HEAD auditado
[`scripts/inventory-client153-assets.py:21-43`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/scripts/inventory-client153-assets.py#L21-L43) percorre apenas `source_root/element/*.pck` (com continuação PKX), enquanto [`src/client153/Angelica2/AngelicaFile/Source/AFileImage.cpp:95-167`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Angelica2/AngelicaFile/Source/AFileImage.cpp#L95-L167) possui fallback para arquivo avulso via `fopen` quando não encontra pacote. Logo o inventário atual não cobre explicitamente todos os arquivos avulsos que o cliente pode abrir.

## Impacto possível (hipótese)
Tamanho total do cliente e primeira área podem ser subestimados caso arquivos avulsos, scripts, áudios, UI ou outros pacotes estejam fora da raiz indexada. Não foi quantificada a parcela efetiva sem diretório cliente autorizado.

## Experimento / estratégia posterior
Construir inventário de filesystem com allowlist/denylist de áreas do cliente, ignorando exe/bin privados e conteúdos não destinados ao browser; separar PCK/PKX, assets avulsos, runtime/wasm e dados por versão; evitar dupla contagem e preservar permissões de uso.

## Plano de benchmark / validação
Fixture com DDS avulsa, script, áudio e PCK/PKX; validar cobertura, exceções, soma total e relatório separado por área/versão/arquivo; não publicar paths sensíveis ou conteúdo não autorizado.

**Critério de aceite:** demonstrar equivalência de bytes e hashes de arquivos, comportamento de loading e de gameplay, compatibilidade com formato legado; medir TTFP/TTI, bytes transferidos, compressão real, p50/p95/p99 de CPU/decode/I/O e pico RAM/VRAM quando disponível. Registrar SHA, dispositivo, rede/RTT, tamanho da amostra e before/after na mesma cena.

## Deduplicação e dependências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), fonte inspecionada `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`.
- Épico [e15-assets-streaming-perf-v1](../epics/e15-assets-streaming-perf-v1/README.md).
- Checar PW23-020 (entrega/cloud), PW23-024 (cópias JS/WASM), PW23-027 (streaming do terreno), PW23-032 (política de confiança do manifest), OPT-002 (dedup de textura), OPT-003 (inflate do payload), OPT-014 (DDS legado) e OPT-024 (SGC de skills). Este registro trata exclusivamente o recorte descrito.
- Se confirmar vulnerabilidade crítica, crash ou falha impeditiva de port, reclassificar com reprodutor em `.agents/issues`; esta pasta não é autorização de adiamento de incidente real.
- Não abrir terceiro PR, não tocar PR #17 do servidor, não fazer auto-merge.
