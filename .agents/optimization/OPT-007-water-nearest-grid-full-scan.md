---
id: OPT-007
title: "Evitar varredura completa de grids na busca de água mais próxima"
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

# OPT-007 — Evitar varredura completa de grids na busca de água mais próxima

**Classificação:** inspeção estática; candidato pós-port, não blocker de compilação ou gameplay. Não implementar antes do cliente realmente rodar no navegador PC e no mobile com baseline de performance. Problemas reais de segurança/correção comprovados devem ser promovidos a `.agents/issues` com reprodução; a classificação aqui não os isenta de correção.

## Subsistema
Água / consultas espaciais

## Evidência observada
[`src/client153/Angelica2/Angelica3D/Source/A3DTerrainWater.cpp:91-135`](https://github.com/brasalabs6/pw-web/blob/87a7748491f94bdf16e455c303218393a039de94/src/client153/Angelica2/Angelica3D/Source/A3DTerrainWater.cpp#L91-L135) implementa `GetNearestWaterArea` percorrendo cada grupo, área e **cada célula x/y do grid**, mesmo quando a consulta busca apenas uma área próxima.

## Hipótese / impacto possível
Complexidade O(total de células de todas as áreas) por consulta. Pode ficar cara com lagos grandes, mas a frequência real e a distribuição espacial ainda precisam ser rastreadas; nenhuma medição foi feita.

## Estratégia candidata (não implementada)
Comparar busca exaustiva com estrutura espacial por AABB / buckets e distância mínima a cada célula ativa (quadtree ou distance field quando útil), com cache por tile do mundo. Manter comportamento de desempate, minor-water, sobreposição e resolução espacial do código original.

## Plano de teste e aceite
Instrumentar call count em GetNearestWaterArea; testar 1/10/100 áreas, grades pequenas/grandes, buracos, bordas, minor water, jogador voando/entrando e saindo. Verificar mesma área/mesma distância e p95/p99 antes-depois.

**Gate de aceite:** comparar mesma cena, conteúdo, qualidade, SHA, hardware e perfil de dispositivo; reportar p50/p95/p99 de tempo CPU/GPU (quando disponível), RAM/VRAM e regressões visuais/funcionais. Ganho não quantificado nesta revisão.

## Dependências
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23); baseline inspecionado `87a7748491f94bdf16e455c303218393a039de94`.
- [e07-world-terrain-v1](../epics/e07-world-terrain-v1/README.md).
- Consultar `README.md`, PW23-021…037 e OPT-001…006 antes de executar; documentar dedup e revalidar contra HEAD.
- Não tocar no servidor PR #17, não abrir terceiro PR, não fazer merge ou tuning do código nesta fase.
