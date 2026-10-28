---
id: OPT-030
title: "Evitar extração de DDS irmãs não referenciadas pela cena MOX"
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

# OPT-030 — Evitar extração de DDS irmãs não referenciadas pela cena MOX

**Gate:** hipótese estática para **depois** que o cliente Angelica 1.5.3 rodar funcionalmente via WASM no navegador de PC e mobile; nenhuma alteração de código-fonte de tuning foi executada. Não alegar ganho de MB, taxa de compressão, FPS ou carga de browser real.

## Área
Staging / tamanho de bundle de demonstração

## Evidência no HEAD auditado
[`scripts/prepare-client153-scene.py:22-45`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/scripts/prepare-client153-scene.py#L22-L45) escolhe o MOX e **todas** as DDS sob `directory + '/textures/'`, faz `package.read` e escreve cada arquivo. [`web/client153/app.mjs:30-36`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/web/client153/app.mjs#L30-L36) consulta somente as texturas referenciadas pelas meshes, portanto pode haver DDS exportadas e jamais solicitadas.

## Impacto possível (hipótese)
O conjunto produzido pode ter payload/manifest maiores do que a dependência efetiva da cena e custar disco/cook/controle de versão; não prova que o navegador baixe as DDS excedentes, pois ele usa um manifest. É estágio do **viewer diagnóstico**, não do game completo.

## Experimento / estratégia posterior
Extrair apenas nomes DDS efetivamente mencionados no MOX e dependências de materiais, com resolução GBK/casefold e caminho relativo; testar casos de atlas, animação, skin, DDS com mesmo basename, detail/lightmap. Limpar outputs obsoletos com manifest transacional, sem alterar originais.

## Plano de benchmark / validação
Cenas sintéticas com 2 DDS usadas e 20 irmãs extras; comparar bytes no output e SHA das texturas realmente referenciadas, detectar assets órfãos e restaurar modo all-siblings por debug. Validar telas e efeitos para evitar missing dependencies.

**Critério de aceite:** demonstrar equivalência de bytes e hashes de arquivos, comportamento de loading e de gameplay, compatibilidade com formato legado; medir TTFP/TTI, bytes transferidos, compressão real, p50/p95/p99 de CPU/decode/I/O e pico RAM/VRAM quando disponível. Registrar SHA, dispositivo, rede/RTT, tamanho da amostra e before/after na mesma cena.

## Deduplicação e dependências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), fonte inspecionada `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`.
- Épico [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md).
- Checar PW23-020 (entrega/cloud), PW23-024 (cópias JS/WASM), PW23-027 (streaming do terreno), PW23-032 (política de confiança do manifest), OPT-002 (dedup de textura), OPT-003 (inflate do payload), OPT-014 (DDS legado) e OPT-024 (SGC de skills). Este registro trata exclusivamente o recorte descrito.
- Se confirmar vulnerabilidade crítica, crash ou falha impeditiva de port, reclassificar com reprodutor em `.agents/issues`; esta pasta não é autorização de adiamento de incidente real.
- Não abrir terceiro PR, não tocar PR #17 do servidor, não fazer auto-merge.
