---
id: PW23-003
title: "RoleInfo difere na serialização entre cliente e servidor v145"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e10-network-transport-v1
scope: pr-23
---

# PW23-003 — RoleInfo difere na serialização entre cliente e servidor v145

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`scripts/compare-client153-role-wire.py`, `tests/client153/roleinfo_wire.cpp`, `docs/CLIENT153-HANDOFF.md`: fixture default observada com 60 bytes no cliente e 59 no servidor; byte extra `rank` no cliente.

## Impacto / por que importa
Há divergência concreta de esquema para a fixture, embora ainda não prove quebra de todo o protocolo. Pode comprometer lista de personagens, criação de role, seleção ou outros envelopes.

## Solução proposta
Mapear as versões/edition e fazer golden fixtures não default e encapsuladas, bilateralmente, para RoleInfo/CreateRole/GetRoleList/SelectRole/EnterWorld. Adaptar no boundary se necessário, sem alterar arbitrariamente a source.

## Testes e critérios de aceite
Script comparador verde para casos representativos e handshake/lista/seleção com servidor PR #17; registrar decisão e schema compatível.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e10-network-transport-v1`](../epics/e10-network-transport-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).
