---
id: PW23-025
title: "Impor limites globais e fuzz para SKI não confiável"
priority: P1
status: open
kind: security-hardening
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e17-security-deployment-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-025 — Impor limites globais e fuzz para SKI não confiável

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/Angelica2/Angelica3D/port/ski.cpp:76-132`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/Angelica2/Angelica3D/port/ski.cpp#L76-L132) permite até 4096 meshes e até 1.000.000 vértices por mesh, com `reserve(nv)`. Checa se bytes remanescentes cobrem um mesh, mas não um orçamento global de vértices/memória ao longo de todos os meshes. `AnimatedSkin::load` limita arquivo a 64 MiB, porém `load_ski` também é API pública de parser e não exige esse limite.

## Impacto e hipótese técnica
Arquivos maliciosos/grandes podem provocar consumo excessivo de memória ou falha de alocação, degradando sessão ou derrubando aba. É risco de exaustão; não há exploit reproduzido.

## Implementação proposta
Limites cumulativos de bytes, meshes, vértices, índices, ossos e strings; overflow-safe checked multiplication; validar compatibilidade de índices e pesos; limite da API mesmo fora de AnimatedSkin; catch de bad_alloc; fuzz libFuzzer/AFL com ASan+UBSan.

## Testes / plano de medição
Corpus mínimo, header/counts na borda, truncamento em cada offset, casos count cumulativo, NaN, números extremos, timeout e limite de RSS.

## Critério de aceite
Parsers rejeitam entradas fora do orçamento de forma determinística sem crash/hang e todas as regressões passam.

## Dependências e governança
- Épico: [e17-security-deployment-v1](../epics/e17-security-deployment-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
