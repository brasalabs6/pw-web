---
id: OPT-031
title: "Reduzir custo de abertura do índice PCK com milhares de entradas"
priority: P1
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

# OPT-031 — Reduzir custo de abertura do índice PCK com milhares de entradas

**Gate:** hipótese estática para **depois** que o cliente Angelica 1.5.3 rodar funcionalmente via WASM no navegador de PC e mobile; nenhuma alteração de código-fonte de tuning foi executada. Não alegar ganho de MB, taxa de compressão, FPS ou carga de browser real.

## Área
PCK catalog / load latency

## Evidência no HEAD auditado
[`src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp:209-273`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp#L209-L273) carrega o índice iterando arquivo a arquivo: duas leituras de inteiro, uma terceira leitura, alocação temporária e possível inflate por item; há caminho semelhante em [`src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp:328-389`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp#L328-L389) para versão com offsets 64-bit. O leitor Python [`tools/assets/pck153.py:138-165`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/tools/assets/pck153.py#L138-L165) também percorre cada entrada na inicialização.

## Impacto possível (hipótese)
Abrir muitos pacotes com dezenas/milhares de entradas pode causar latência inicial e trabalho de zlib em pequenos registros, mesmo sem carregar texturas ou mapas. Não é o mesmo que OPT-003, que aborda leitura/descompressão do **payload** de arquivos.

## Experimento / estratégia posterior
Instrumentar tempo de indexação por arquivo e branch/version; estudar leitura sequencial em chunks e parsing em batch, cache offline versionado de metadados e lazy catalog sharding, preservando offsets, versões, nomes GBK e collision tables. Validar que pré-indexado não é autoridade maior que SHA do pacote.

## Plano de benchmark / validação
Fixtures PCK com 100, 10 mil e 100 mil registros, 0/50/100% comprimidos, PCK/PKX e índice truncado; startup p95, bytes lidos, chamadas read, mem allocations e igualdade do catálogo.

**Critério de aceite:** demonstrar equivalência de bytes e hashes de arquivos, comportamento de loading e de gameplay, compatibilidade com formato legado; medir TTFP/TTI, bytes transferidos, compressão real, p50/p95/p99 de CPU/decode/I/O e pico RAM/VRAM quando disponível. Registrar SHA, dispositivo, rede/RTT, tamanho da amostra e before/after na mesma cena.

## Deduplicação e dependências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), fonte inspecionada `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`.
- Épico [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md).
- Checar PW23-020 (entrega/cloud), PW23-024 (cópias JS/WASM), PW23-027 (streaming do terreno), PW23-032 (política de confiança do manifest), OPT-002 (dedup de textura), OPT-003 (inflate do payload), OPT-014 (DDS legado) e OPT-024 (SGC de skills). Este registro trata exclusivamente o recorte descrito.
- Se confirmar vulnerabilidade crítica, crash ou falha impeditiva de port, reclassificar com reprodutor em `.agents/issues`; esta pasta não é autorização de adiamento de incidente real.
- Não abrir terceiro PR, não tocar PR #17 do servidor, não fazer auto-merge.
