---
id: OPT-029
title: "Medir peso e compressibilidade de cada asset dentro dos PCK/PKX"
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

# OPT-029 — Medir peso e compressibilidade de cada asset dentro dos PCK/PKX

**Gate:** hipótese estática para **depois** que o cliente Angelica 1.5.3 rodar funcionalmente via WASM no navegador de PC e mobile; nenhuma alteração de código-fonte de tuning foi executada. Não alegar ganho de MB, taxa de compressão, FPS ou carga de browser real.

## Área
Inventory / bytes / codecs

## Evidência no HEAD auditado
[`scripts/inventory-client153-assets.py:21-42`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/scripts/inventory-client153-assets.py#L21-L42) registra apenas contagem, extensões e bytes/hash dos arquivos de contêiner, embora [`tools/assets/pck153.py:82-88`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/tools/assets/pck153.py#L82-L88) já forneça `Entry.size` e `Entry.stored_size`. Não aparece relatório de custo por recurso, razão de compressão, p50/p95/p99 ou top N por extensão nesse fluxo.

## Impacto possível (hipótese)
Sem distribuição real de tamanhos, o projeto não sabe quais modelos, texturas, efeitos, mapas e UI justificam pré-carregamento, mudança de codec ou precache. **Nenhum tamanho de asset foi medido nesta auditoria: não estão disponíveis pacotes autorizados de cliente.**

## Experimento / estratégia posterior
Estender o inventário offline somente de metadata, sem extração: tamanho bruto, armazenado, razão, extensão, contagem, p50/p95/p99, top arquivos, categoria e duplicatas de caminhos; opcionalmente usar hash de payload somente em amostra controlada. Respeitar confidencialidade e licença dos arquivos. Combinar com telemetria de cold start e zona.

## Plano de benchmark / validação
Rodar em cliente 1.5.3 autorizado com e sem PKX; validar soma dos armazenados, percentis, entradas zero-byte e arquivo grande; publicar números apenas após execução. Comparar com logs de bytes HTTP e RAM na cena.

**Critério de aceite:** demonstrar equivalência de bytes e hashes de arquivos, comportamento de loading e de gameplay, compatibilidade com formato legado; medir TTFP/TTI, bytes transferidos, compressão real, p50/p95/p99 de CPU/decode/I/O e pico RAM/VRAM quando disponível. Registrar SHA, dispositivo, rede/RTT, tamanho da amostra e before/after na mesma cena.

## Deduplicação e dependências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), fonte inspecionada `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`.
- Épico [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md).
- Checar PW23-020 (entrega/cloud), PW23-024 (cópias JS/WASM), PW23-027 (streaming do terreno), PW23-032 (política de confiança do manifest), OPT-002 (dedup de textura), OPT-003 (inflate do payload), OPT-014 (DDS legado) e OPT-024 (SGC de skills). Este registro trata exclusivamente o recorte descrito.
- Se confirmar vulnerabilidade crítica, crash ou falha impeditiva de port, reclassificar com reprodutor em `.agents/issues`; esta pasta não é autorização de adiamento de incidente real.
- Não abrir terceiro PR, não tocar PR #17 do servidor, não fazer auto-merge.
