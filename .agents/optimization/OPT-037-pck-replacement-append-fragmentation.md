---
id: OPT-037
title: "Evitar crescimento histórico de PCK por substituições append-only no pipeline de build"
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

# OPT-037 — Evitar crescimento histórico de PCK por substituições append-only no pipeline de build

**Gate:** hipótese estática para **depois** que o cliente Angelica 1.5.3 rodar funcionalmente via WASM no navegador de PC e mobile; nenhuma alteração de código-fonte de tuning foi executada. Não alegar ganho de MB, taxa de compressão, FPS ou carga de browser real.

## Área
PCK / tamanho de release

## Evidência no HEAD auditado
[`src/client153/Angelica2/AngelicaFile/Source/afilepackage.cpp:1176-1208`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Angelica2/AngelicaFile/Source/afilepackage.cpp#L1176-L1208) descreve e implementa `ReplaceFile` anexando uma nova cópia no final da parte de dados e atualizando entrada. O método não faz compactação dos blocos físicos antigos nesse trecho.

## Impacto possível (hipótese)
Se o pipeline de preparação editar PCK diretamente, múltiplas substituições podem deixar bytes não referenciados e aumentar download/armazenamento. **Não inferir que os PCK originais distribuídos hoje estejam fragmentados**; cooker atual deve manter pacote fonte intocado.

## Experimento / estratégia posterior
Preferir cook reprodutível imutável a partir das entradas ativas e bundles content-addressed; se manutenção de PCK for necessária, compactor offline validado por SHA de todas as entradas e política segura de PKX split/rollback. Evitar patch binário sem manifesto e integridade.

## Plano de benchmark / validação
Criar fixture sintética com 1 arquivo substituído 10/100 vezes, medir tamanho físico vs soma de entradas vivas, roundtrip SHA e rollback; comparar rebuild canônico sem alterar pacote autorizado original.

**Critério de aceite:** demonstrar equivalência de bytes e hashes de arquivos, comportamento de loading e de gameplay, compatibilidade com formato legado; medir TTFP/TTI, bytes transferidos, compressão real, p50/p95/p99 de CPU/decode/I/O e pico RAM/VRAM quando disponível. Registrar SHA, dispositivo, rede/RTT, tamanho da amostra e before/after na mesma cena.

## Deduplicação e dependências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), fonte inspecionada `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`.
- Épico [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md).
- Checar PW23-020 (entrega/cloud), PW23-024 (cópias JS/WASM), PW23-027 (streaming do terreno), PW23-032 (política de confiança do manifest), OPT-002 (dedup de textura), OPT-003 (inflate do payload), OPT-014 (DDS legado) e OPT-024 (SGC de skills). Este registro trata exclusivamente o recorte descrito.
- Se confirmar vulnerabilidade crítica, crash ou falha impeditiva de port, reclassificar com reprodutor em `.agents/issues`; esta pasta não é autorização de adiamento de incidente real.
- Não abrir terceiro PR, não tocar PR #17 do servidor, não fazer auto-merge.
