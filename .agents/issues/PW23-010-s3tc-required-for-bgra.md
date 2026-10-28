---
id: PW23-010
title: "Não exigir S3TC para renderizar DDS BGRA sem compressão"
priority: P1
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e06-render-webgl2-v1
scope: pr-23
---

# PW23-010 — Não exigir S3TC para renderizar DDS BGRA sem compressão

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`platform/web/webgl_caps.cpp` exige `WEBGL_compressed_texture_s3tc` durante init, embora `port/dds.cpp` aceite BGRA8 e converta para RGBA; check acontece antes da DDS selecionada.

## Impacto / por que importa
Bloqueia GPUs/browser sem S3TC mesmo quando a textura não exige compressão, principalmente dispositivos móveis.

## Solução proposta
Tratar WebGL2 como requisito e S3TC como capacidade opcional testada no momento de carregar DXT; implementar fallback/transcode documentado ou erro específico.

## Testes e critérios de aceite
Testar BGRA num contexto sem S3TC (deve renderizar) e DXT com/sem extensão (fallback ou falha explícita); pixel check e mipmaps.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e06-render-webgl2-v1`](../epics/e06-render-webgl2-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
