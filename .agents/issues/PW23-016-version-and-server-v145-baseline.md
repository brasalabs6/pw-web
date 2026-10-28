---
id: PW23-016
title: "Revisão do código-fonte não comprovadamente idêntica ao PW 1.5.3 v145"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e02-native-client-baseline-v1
scope: pr-23
---

# PW23-016 — Revisão do código-fonte não comprovadamente idêntica ao PW 1.5.3 v145

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`docs/evidence/first-draw-closure.md`: referências contraditórias a 1.5.2 BR, VS2005 libs 2.2 e source custom 1.5.2–1.5.5; servidor #17 derivado comunitário. RoleInfo confirma divergência pontual.

## Impacto / por que importa
Compatibilidade de assets/protocolo/gameplay pode falhar mesmo que o cliente compile e renderize.

## Solução proposta
Construir baseline autorizada Windows/Wine com hash, edition, config e handshake; matriz source×client-v145×server-v145×assets; ADR se alterar versão-alvo.

## Testes e critérios de aceite
Protocolo e dados fundamentais comprovadamente compatíveis, com logs/golden tests e divergências documentadas.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e02-native-client-baseline-v1`](../epics/e02-native-client-baseline-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
