---
id: OPT-035
title: "Substituir política de loading de cena baseada só em número de blocos"
priority: P1
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c
first_observed: 2026-10-28
validation: static-only
epic: e07-world-terrain-v1
scope: PR-23
---

# OPT-035 — Substituir política de loading de cena baseada só em número de blocos

**Gate:** hipótese estática para **depois** que o cliente Angelica 1.5.3 rodar funcionalmente via WASM no navegador de PC e mobile; nenhuma alteração de código-fonte de tuning foi executada. Não alegar ganho de MB, taxa de compressão, FPS ou carga de browser real.

## Área
World/scene / hitch / loading progress

## Evidência no HEAD auditado
[`src/client153/Element/CElementClient/EC_Scene.cpp:547-573`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Element/CElementClient/EC_Scene.cpp#L547-L573) carrega blocos ativos em loop no início e contabiliza 45% fixos de progresso; [`src/client153/Element/CElementClient/EC_Scene.cpp:606-636`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Element/CElementClient/EC_Scene.cpp#L606-L636) limita candidatos por `m_iBlkLoadSpeed`, não por bytes nem ms; [`src/client153/Element/CElementClient/EC_World.cpp:1075-1105`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Element/CElementClient/EC_World.cpp#L1075-L1105) define velocidade inicial 2 e pode chamar LoadInMainThread(true) na troca do centro.

## Impacto possível (hipótese)
Blocos de cena com tamanhos heterogêneos podem causar hitchs/loading irregular, especialmente ao entrar em cidades/povoamentos. Complementa PW23-027 (streaming do **terreno Angelica**), mas foca objetos CECScene e progresso real de mundo.

## Experimento / estratégia posterior
Instrumentar tempo/bytes de scene block e recursos dependentes; depois avaliar fila priority com budget por frame e bytes/VRAM, cancelamento por câmera/teleporte, coreografia do loading e progresso ponderado pelos bytes verificáveis; não eliminar colisão/objetos antes do ready gate.

## Plano de benchmark / validação
Travessia cidade/campo, teleporte rápido e retorno, blocos com 1/1000 objetos, velocidades 1/2/8 blocos: p95/p99 long task, TTFP, progresso monotônico, consistência de cenário e objetos interativos.

**Critério de aceite:** demonstrar equivalência de bytes e hashes de arquivos, comportamento de loading e de gameplay, compatibilidade com formato legado; medir TTFP/TTI, bytes transferidos, compressão real, p50/p95/p99 de CPU/decode/I/O e pico RAM/VRAM quando disponível. Registrar SHA, dispositivo, rede/RTT, tamanho da amostra e before/after na mesma cena.

## Deduplicação e dependências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), fonte inspecionada `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`.
- Épico [e07-world-terrain-v1](../epics/e07-world-terrain-v1/README.md).
- Checar PW23-020 (entrega/cloud), PW23-024 (cópias JS/WASM), PW23-027 (streaming do terreno), PW23-032 (política de confiança do manifest), OPT-002 (dedup de textura), OPT-003 (inflate do payload), OPT-014 (DDS legado) e OPT-024 (SGC de skills). Este registro trata exclusivamente o recorte descrito.
- Se confirmar vulnerabilidade crítica, crash ou falha impeditiva de port, reclassificar com reprodutor em `.agents/issues`; esta pasta não é autorização de adiamento de incidente real.
- Não abrir terceiro PR, não tocar PR #17 do servidor, não fazer auto-merge.
