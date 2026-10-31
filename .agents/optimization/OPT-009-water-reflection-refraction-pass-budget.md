---
id: OPT-009
title: "Avaliar budgets de passes de reflexão/refração de água"
priority: P1
status: candidate
kind: static-optimization
phase: post-browser-port
blocking: false
source_sha: 87a7748491f94bdf16e455c303218393a039de94
first_observed: 2026-10-28
validation: static-only
epic: e07-world-terrain-v1
scope: PR-23
---

# OPT-009 — Avaliar budgets de passes de reflexão/refração de água

**Classificação:** inspeção estática; candidato pós-port, não blocker de compilação ou gameplay. Não implementar antes do cliente realmente rodar no navegador PC e no mobile com baseline de performance. Problemas reais de segurança/correção comprovados devem ser promovidos a `.agents/issues` com reprodução; a classificação aqui não os isenta de correção.

## Subsistema
Água / custo GPU

## Evidência observada
[`src/client153/Angelica2/Angelica3D/Source/A3DTerrainWater.cpp:562-587`](https://github.com/brasalabs6/pw-web/blob/87a7748491f94bdf16e455c303218393a039de94/src/client153/Angelica2/Angelica3D/Source/A3DTerrainWater.cpp#L562-L587) percorre `m_WaterAreas` para chamar `RenderReflect` e `RenderRefract` por grupo de altura. [`src/client153/Angelica2/Angelica3D/Source/A3DTerrainWater.cpp:590-622`](https://github.com/brasalabs6/pw-web/blob/87a7748491f94bdf16e455c303218393a039de94/src/client153/Angelica2/Angelica3D/Source/A3DTerrainWater.cpp#L590-L622) já implementa frustum culling de áreas, portanto **não** se presume que todos os passes sempre sejam efetivamente executados.

## Hipótese / impacto possível
Água com múltiplos níveis pode multiplicar render targets/passes, custo de GPU e bandwidth, sobretudo mobile. Sem counters no renderer interno não há quantificação nem prova de trabalho redundante.

## Estratégia candidata (não implementada)
Instrumentar número de render passes efetivamente executados, resolução e pixels desenhados; comparar half-resolution/reflection quality tiers, atualização temporal escalonada, pooling de FBOs e fallback sem reflexão em low tier. Preservar clipping e efeitos em locais próximos.

## Plano de teste e aceite
Cenas com 0/1/4/16 planos de água visíveis e offscreen; capturas GPU de reflexo/refração, screenshot diff, latency p95, FBO bytes e estabilidade térmica. Não reduzir qualidade sem declarar tier.

**Gate de aceite:** comparar mesma cena, conteúdo, qualidade, SHA, hardware e perfil de dispositivo; reportar p50/p95/p99 de tempo CPU/GPU (quando disponível), RAM/VRAM e regressões visuais/funcionais. Ganho não quantificado nesta revisão.

## Dependências
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23); baseline inspecionado `87a7748491f94bdf16e455c303218393a039de94`.
- [e07-world-terrain-v1](../epics/e07-world-terrain-v1/README.md).
- Consultar `README.md`, PW23-021…037 e OPT-001…006 antes de executar; documentar dedup e revalidar contra HEAD.
- Não tocar no servidor PR #17, não abrir terceiro PR, não fazer merge ou tuning do código nesta fase.
