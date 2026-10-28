---
id: OPT-039
title: "Medir cache LRU fixo e dupla aquisição de referência de textura GFX"
priority: P2
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 4bd108b30650647d1c510db252442de764b27dd1
first_observed: 2026-10-28
validation: static-only
epic: e06-render-webgl2-v1
scope: PR-23
---

# OPT-039 — Cache GFX: LRU de 16 entradas, duas aquisições e lock durante load

**Somente candidato pós-browser-port.** Não alterar o algoritmo antes de ter o jogo Angelica 1.5.3 funcionando no navegador PC e mobile, com cenas/ativos autorizados e métricas repetíveis. O viewer MOX/DDS atual não prova que este gerenciador participe do frame loop ou do carregamento final.

## Evidência estática no SHA auditado

- [`A3DGFXTexMan.cpp:5-22`](https://github.com/brasalabs6/pw-web/blob/4bd108b30650647d1c510db252442de764b27dd1/src/client153/Common/Output/source/Gfx/A3DGFXTexMan.cpp#L5-L22): cache GFX limitado a **16 entradas** e primeira chamada a `A3DTextureMan::LoadTextureFromFileInFolder` antes de entrar no lock.
- [`A3DGFXTexMan.cpp:25-52`](https://github.com/brasalabs6/pw-web/blob/4bd108b30650647d1c510db252442de764b27dd1/src/client153/Common/Output/source/Gfx/A3DGFXTexMan.cpp#L25-L52): no miss, `RemoveLRUItem`, `ReleaseTexture`, `AddItem` e **segunda chamada** a `LoadTextureFromFileInFolder` ainda sob `LockTex`; no hit, `TouchItem`. O retorno da segunda aquisição não é checado aqui.
- [`A3DGFXCachedItemMan.h:38-86`](https://github.com/brasalabs6/pw-web/blob/4bd108b30650647d1c510db252442de764b27dd1/src/client153/Common/Output/include/Gfx/A3DGFXCachedItemMan.h#L38-L86): `RemoveLRUItem` varre todas as entradas do mapa para escolher a mais antiga; o limite 16 torna esse scan pequeno, mas a frequência de churn é desconhecida.
- [`A3DTextureMan.cpp:97-120`](https://github.com/brasalabs6/pw-web/blob/4bd108b30650647d1c510db252442de764b27dd1/src/client153/Angelica2/Angelica3D/Source/A3DTextureMan.cpp#L97-L120): a aquisição encaminha ao gerenciador global, que pesquisa cache existente. **Não presumir dois uploads de GPU**: a segunda aquisição pode existir deliberadamente para manter uma referência do cache além da referência do chamador.

## Hipótese e impacto a validar

Sob diversidade alta de texturas de efeitos, o cache de 16 entradas pode sofrer evicções frequentes; o miss pode fazer duas pesquisas/aquisições globais e manter o mutex do cache GFX durante a segunda chamada. Investigar custo de CPU, contenção, refcount, alocações, RAM/VRAM, uploads reais e estabilidade em desktop/mobile. Não foi demonstrado gargalo, leak, crash, bug visual ou aumento de FPS. Não inferir que 16 é inadequado sem medir distribuição de reutilização.

## Benchmark pós-port e critério de aceite

1. Instrumentar separadamente hit/miss, evicção, tempo de `LockTex`, tempo das duas aquisições, refcounts e criação/upload GPU, sem mudar o comportamento. Rodar cenas reais com 8, 16, 32, 128 e 512 texturas GFX distintas, controlando repetição e ordem.
2. Medir frame time p50/p95/p99 CPU/GPU quando disponível, número de draw calls, bytes de textura, RAM/VRAM (se mensurável), hitches e consumo/temperatura no Android físico; comparar com desktop no mesmo SHA/cena/qualidade e documentar limitações de instrumentação.
3. Só se houver gargalo real: experimentar cache por orçamento de bytes/recência, mover IO/decode para fora da região crítica ou coalescer lookups, **preservando exatamente as duas responsabilidades de lifetime/refcount**; validar context-loss, concorrência, misses e flags `bNoDownSample`.
4. Aceite condicionado a redução medida de custo sem regressão de textura/material, sem refs perdidas, sem aumento descontrolado de memória e sem afetar login/gameplay. Nunca otimizar antes de completar o port.

## Deduplicação e limites

- [OPT-001](OPT-001-texture-manager-linear-lookup.md) trata o **gerenciador global** `A3DTextureMan::FindTexture`, não a política de cache GFX nem sua seção crítica.
- [OPT-002](OPT-002-dedup-texture-fetch-upload.md) trata **viewer JS/WASM** e dedup entre meshes, não esta dupla aquisição do gerenciador original.
- [OPT-011](OPT-011-texture-precache-bounded-scheduling.md) trata aquecimento global `PrecacheAllTexture`, não LRU de efeitos.
- [OPT-016](OPT-016-combat-gfx-stop-cache-retention-60000s.md) trata lifetime de **instâncias de efeitos**, não referências de texturas.

Análise de código apenas no commit `4bd108b30650647d1c510db252442de764b27dd1` do [PR #23](https://github.com/brasalabs6/pw-web/pull/23); não houve benchmark, teste de dispositivo, teste de gameplay ou subagentes. Nenhuma mudança de código proposta foi aplicada.
