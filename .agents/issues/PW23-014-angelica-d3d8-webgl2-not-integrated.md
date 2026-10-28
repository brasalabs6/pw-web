---
id: PW23-014
title: "Backend da Angelica original ainda não desenha os frames do jogo"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e06-render-webgl2-v1
scope: pr-23
---

# PW23-014 — Backend da Angelica original ainda não desenha os frames do jogo

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`platform/web/runtime.cpp` implementa viewer GLSL MOX/DDS e câmera orbital; target `client153_engine_original` OBJECT não está linkado ao browser game; docs evidenciam rotas PURESERVER/HWI com falso sucesso sem desenho.

## Impacto / por que importa
Não há equivalência da pipeline D3D8 Angelica: buffers/FVF, estados, blending, fog, iluminação, skinning e render de mundo.

## Solução proposta
Conectar A3DEngine/A3DDevice/A3DStream/A3DSkinModel ao backend WebGL2 sem duplicar engine; implementar estados usados e comparar frames com baseline Win32/Wine.

## Testes e critérios de aceite
Original Angelica produz pixels de cena/modelo animado e mundo via seu call graph; testes excluem caminhos que retornam sucesso sem draw.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e06-render-webgl2-v1`](../epics/e06-render-webgl2-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
