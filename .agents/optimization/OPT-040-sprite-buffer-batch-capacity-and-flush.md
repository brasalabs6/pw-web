---
id: OPT-040
title: "Limite de lote e flush de sprites A2D no renderer legado"
priority: P2
status: candidate
kind: static-optimization
phase: post-browser-port
blocking: false
source_sha: c86bb95a0cb37d45d31b81a6c7a27d3afe1f6a24
first_observed: 2026-10-28
validation: static-only
epic: e09-interface-input-audio-v1
scope: PR-23
---

# OPT-040 — Limite de lote e flush de sprites A2D

**Fase: somente após o port WebAssembly funcionar em navegador desktop e mobile.** Inspeção estática da Angelica 1.5.3 no SHA `c86bb95a0cb37d45d31b81a6c7a27d3afe1f6a24`; nenhuma execução, medição de FPS, dispositivo físico ou reprodução de defeito nesta auditoria. Não alterar código para tuning agora.

## Evidências precisas

- [`A2DSpriteBuffer.cpp:86-108`](https://github.com/brasalabs6/pw-web/blob/c86bb95a0cb37d45d31b81a6c7a27d3afe1f6a24/src/client153/Angelica2/Angelica3D/Source/A2DSpriteBuffer.cpp#L86-L108): o flush percorre slots de textura, calcula `nNumBlades = nVertsCount / 6` e limita **uma única vez** a `GetMaxBlades()` antes de `LockStream`.
- [`A2DSpriteBuffer.cpp:113-134`](https://github.com/brasalabs6/pw-web/blob/c86bb95a0cb37d45d31b81a6c7a27d3afe1f6a24/src/client153/Angelica2/Angelica3D/Source/A2DSpriteBuffer.cpp#L113-L134): copia e desenha somente esse lote, depois zera `nVertsCount` e remove todos os vértices do slot, sem loop para o restante. Se um slot acumular mais quads que `nMaxBlades`, a leitura estática sugere descarte silencioso do excedente; **não está reproduzido** e não classificar como blocker do port.
- [`A2DSpriteBuffer.cpp:150-175`](https://github.com/brasalabs6/pw-web/blob/c86bb95a0cb37d45d31b81a6c7a27d3afe1f6a24/src/client153/Angelica2/Angelica3D/Source/A2DSpriteBuffer.cpp#L150-L175): `AddToBuffer` acrescenta seis vértices por quad e acumula no slot, sem limite local de quads. Os atributos de blend/filtro/stereo do slot são sobrescritos pela última inserção, o que requer teste de fidelidade visual antes de qualquer mudança de batching.
- [`A2DSpriteTexMan.cpp:285-305`](https://github.com/brasalabs6/pw-web/blob/c86bb95a0cb37d45d31b81a6c7a27d3afe1f6a24/src/client153/Angelica2/Angelica3D/Source/A2DSpriteTexMan.cpp#L285-L305): flush por textura cacheada; [`A2DSpriteTexMan.cpp:311-329`](https://github.com/brasalabs6/pw-web/blob/c86bb95a0cb37d45d31b81a6c7a27d3afe1f6a24/src/client153/Angelica2/Angelica3D/Source/A2DSpriteTexMan.cpp#L311-L329): ring de vértices com reinício e `NOOVERWRITE`.

## Hipótese e impacto potencial

Com UI de muitas sprites/quads por atlas, o limite de lote pode reduzir a quantidade efetivamente desenhada e o desenho atual exige lock/cópia/draw por slot. Há risco de overhead CPU/WASM↔WebGL2, chamadas GPU e pressão de alocações em telas mobile. **Não inferir FPS, RAM/VRAM, energia nem frequência de ocorrência a partir deste código.** A observação sobre excedente é uma condição estática verificável, não uma regressão reproduzida.

## Experimento pós-browser-port

1. Primeiro comprovar renderização Angelica, HUD, input, login e gameplay em Chrome desktop e Android/iOS reais. Manter o código original até haver baseline funcional.
2. Instrumentar por slot: `nVertsCount/6`, `nMaxBlades`, número de quads submetidos, desenhados e eventualmente excedentes, `LockStream`, bytes copiados, draw calls e alterações de estado.
3. Reproduzir fixtures de 100/1.000/10.000 quads, incluindo muitos quads de uma textura e mistura de filtros/blends/ordens de transparência. Comparar screenshots/golden frames com referência nativa.
4. Apenas se medido, testar flush em múltiplos lotes e batches que respeitem ordem e estado, com streaming de vértices por ring e buffers reutilizáveis. Não agrupar elementos transparentes de forma que altere sobreposição.
5. Medir frame time CPU/GPU p50/p95/p99, FPS 1% low, draw calls, heap WASM, pico RAM/VRAM quando observável, tempo de upload e consumo energético/temperatura em soak mobile, mantendo SHA, cena, viewport, DPR e qualidade comparáveis.

**Aceite:** nenhum quad perdido, mesma semântica visual e controles, nenhuma regressão funcional, e melhoria de custo comprovada por benchmark before/after. Se a condição de perda de sprites for reproduzida e impedir o port, criar **nova issue** com reprodutor e deduplicação; não promover esta hipótese automaticamente.

## Deduplicação

Distinto de [PW23-026](PW23-026-per-frame-uniform-state-and-draw-calls.md) (uniforms/draws do viewer MOX WebGL2), [OPT-023](OPT-023-crowd-combat-nameplate-ui-overdraw.md) (visibilidade/overdraw de placas em multidões) e [OPT-001](OPT-001-texture-manager-linear-lookup.md) (cache A3DTextureMan). Este candidato trata especificamente do limite e do flush do `A2DSpriteBuffer` original.

Não modificar fonte, PR #17 ou abrir terceiro PR.