---
id: PW23-037
title: "Validar pesos, índices, normals e finitude antes do blend"
priority: P1
status: open
kind: security-hardening
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e17-security-deployment-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-037 — Validar pesos, índices, normals e finitude antes do blend

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/Angelica2/Angelica3D/port/ski.cpp:109-135`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/port/ski.cpp#L109-L135) decodifica quatro pesos e bone indices sem checar soma normalizada/valores negativos; [`src/client153/Angelica2/Angelica3D/port/skin_binding.cpp:42-68`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/port/skin_binding.cpp#L42-L68) usa `1-total` no quarto peso, e `n.Normalize()` mesmo que todas as influências sejam zero.

## Impacto e hipótese técnica
SKI estranho pode produzir deformação incorreta, normais degeneradas ou cálculos inúteis; não é uma vulnerabilidade de execução de código demonstrada.

## Implementação proposta
Estabelecer contrato de pesos válidos conforme formato de origem (sem quebrar assets reais), checar ranges/índices por mesh, fallback robusto para zero-weight e normal degenerada, log/fail explícitos, fuzz + differential testing contra leitura original.

## Testes / plano de medição
SKI com weights zeros, negativas, >1, quarto negativo implícito, vetor normal zero, índices limites; testes de posição/normal finitos, sem crash.

## Critério de aceite
Nenhum output NaN/Inf e política de fallback explícita com corpus real.

## Dependências e governança
- Épico: [e17-security-deployment-v1](../epics/e17-security-deployment-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
