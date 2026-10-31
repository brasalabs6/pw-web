---
id: OPT-015
title: "Medir buscas lineares e sincronização quadrática dos render slots do terreno"
priority: P2
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 07b8e843acd4a019d2fda991bf14c2f5b63433f1
first_observed: 2026-10-28
validation: static-only
epic: e07-world-terrain-v1
scope: pr-23
---

# OPT-015 — Medir buscas lineares e sincronização quadrática dos render slots do terreno

**Escopo:** Terrain render-slot CPU work. **Candidato preventivo pós-port**, não blocker comprovado. Não implementar tuning até o cliente WASM original executar no navegador desktop e celular e existir baseline de FPS, RAM/VRAM, frame time e estabilidade. Caso um crash/vulnerabilidade grave seja reproduzido, registrar a falha em `.agents/issues` com evidência, sem esperar pelo tuning.

## Evidência exata no SHA auditado
[`src/client153/Angelica2/Angelica3D/Source/A3DTerrain2Render.cpp:270-293`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/src/client153/Angelica2/Angelica3D/Source/A3DTerrain2Render.cpp#L270-L293) procura o slot de cada bloco percorrendo `m_aCurSlots`; [`src/client153/Angelica2/Angelica3D/Source/A3DTerrain2Render.cpp:352-421`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/src/client153/Angelica2/Angelica3D/Source/A3DTerrain2Render.cpp#L352-L421) percorre slots anteriores e atuais em loops aninhados para transferir streams. A rotina `ResetRenderInfo` declara execução por frame ([`src/client153/Angelica2/Angelica3D/Source/A3DTerrain2Render.cpp:320-349`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/src/client153/Angelica2/Angelica3D/Source/A3DTerrain2Render.cpp#L320-L349)).

## Impacto possível e grau de certeza
Se existirem muitos mask areas/slots, a busca por bloco e o pareamento de slots podem elevar tempo CPU e causar jitter de frame; não há prova de que slots sejam numerosos em mapas reais, portanto não afirmar gargalo.

## Estratégia candidata, sem mudança de código nesta rodada
Após baseline, instrumentar quantidade de slots/mask areas, chamadas por frame e tempo de SyncLastRenderSlots; avaliar índice temporário por mask-area (vetor direto quando denso, hash quando esparso) e reutilização com geração/epoch, mantendo semântica de transferência e liberação de streams.

## Testes e critério de aceite pós-port
Mapas com poucas/muitas mask areas, câmera parada/voando/teleporte, transições de slots e reconstrução; comparar custo CPU p50/p95/p99, allocations e GPU streams vivos, sem alterar geometria, ordem visual ou LOD.

**Gate de aceitação:** revalidar contra HEAD, registrar dispositivo/SHA/cena/qualidade, before/after com CPU/GPU frame time p50/p95/p99, memória e regressões funcionais; não prometer ganhos antes dos testes.

## Deduplicação e dependências
Distinto de PW23-027 (streaming/LOD por budget) e OPT-011 (texture warmup); aqui o alvo é busca e reuso de slots no CPU.
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA auditado `07b8e843acd4a019d2fda991bf14c2f5b63433f1`.
- [e07-world-terrain-v1](../epics/e07-world-terrain-v1/README.md).
- Preservar PW23-021…037 e OPT-001…012; não abrir PR, alterar o servidor ou realizar auto-merge.
