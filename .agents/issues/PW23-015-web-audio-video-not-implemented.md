---
id: PW23-015
title: "AngelicaMedia compila mas não há saída WebAudio comprovada"
priority: P1
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e09-interface-input-audio-v1
scope: pr-23
---

# PW23-015 — AngelicaMedia compila mas não há saída WebAudio comprovada

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`client153_engine_original` adiciona AMSoundEngine/AMSoundBufferMan/AMEngine em CMake; `dsound.h` é shim de assinatura. Nenhum E2E de áudio do jogo no browser.

## Impacto / por que importa
DirectSound/DirectShow originais não são APIs do browser; objetos compilados não garantem playback.

## Solução proposta
Implementar adaptadores WebAudio/HTMLMediaElement necessários, respeitar gesto para desbloqueio e mapear 3D/volume/loop/stop e lifecycle.

## Testes e critérios de aceite
Teste browser com gesto do jogador e verificação de saída/estado do AudioContext e efeitos de gameplay quando integrados.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e09-interface-input-audio-v1`](../epics/e09-interface-input-audio-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
