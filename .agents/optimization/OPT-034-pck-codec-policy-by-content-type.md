---
id: OPT-034
title: "Comparar compressão seletiva por tipo de asset sem alterar assets originais"
priority: P2
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c
first_observed: 2026-10-28
validation: static-only
epic: e05-asset-pipeline-v1
scope: PR-23
---

# OPT-034 — Comparar compressão seletiva por tipo de asset sem alterar assets originais

**Gate:** hipótese estática para **depois** que o cliente Angelica 1.5.3 rodar funcionalmente via WASM no navegador de PC e mobile; nenhuma alteração de código-fonte de tuning foi executada. Não alegar ganho de MB, taxa de compressão, FPS ou carga de browser real.

## Área
Compression / CDN cooker

## Evidência no HEAD auditado
[`src/client153/Angelica2/AngelicaFile/Source/afilepackage.cpp:1007-1053`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Angelica2/AngelicaFile/Source/afilepackage.cpp#L1007-L1053) tenta comprimir com zlib cada arquivo gravado com `bCompress`, reserva buffer com tamanho original e só usa saída compactada quando menor. [`src/client153/Angelica2/AngelicaFile/Source/afilepackage.cpp:1752-1765`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Angelica2/AngelicaFile/Source/afilepackage.cpp#L1752-L1765) usa `compress2(...,1)`.

## Impacto possível (hipótese)
Textures DDS comprimidas, áudio e arquivos já compactados podem ter compressibilidade diferente de texto/UI/dados. A tentativa global tem custo offline, enquanto a escolha de codec no CDN pode reduzir bytes e aumentar CPU de decode; **nenhuma taxa real de redução foi medida**.

## Experimento / estratégia posterior
Usar inventário OPT-029 para ranking por extensão/entropia e comparar zlib level1 vs codec de entrega como Brotli/Zstd (quando o runtime de entrega suportar), KTX2/BasisU para texturas com QA visual e transporte sem recomprimir conteúdo já comprimido; preservar PCK original read-only e checksums por asset. Nunca substituir cegamente DXT no caminho legado.

## Plano de benchmark / validação
Amostra autorizada representativa de texturas, malhas, UI e arquivos de configuração. Relatar bytes comprimidos/decompressão CPU, cold/warm start, hash após decode, custo de catálogo e compatibilidade Chrome/Android/Safari.

**Critério de aceite:** demonstrar equivalência de bytes e hashes de arquivos, comportamento de loading e de gameplay, compatibilidade com formato legado; medir TTFP/TTI, bytes transferidos, compressão real, p50/p95/p99 de CPU/decode/I/O e pico RAM/VRAM quando disponível. Registrar SHA, dispositivo, rede/RTT, tamanho da amostra e before/after na mesma cena.

## Deduplicação e dependências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), fonte inspecionada `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`.
- Épico [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md).
- Checar PW23-020 (entrega/cloud), PW23-024 (cópias JS/WASM), PW23-027 (streaming do terreno), PW23-032 (política de confiança do manifest), OPT-002 (dedup de textura), OPT-003 (inflate do payload), OPT-014 (DDS legado) e OPT-024 (SGC de skills). Este registro trata exclusivamente o recorte descrito.
- Se confirmar vulnerabilidade crítica, crash ou falha impeditiva de port, reclassificar com reprodutor em `.agents/issues`; esta pasta não é autorização de adiamento de incidente real.
- Não abrir terceiro PR, não tocar PR #17 do servidor, não fazer auto-merge.
