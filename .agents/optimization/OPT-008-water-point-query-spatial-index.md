---
id: OPT-008
title: "Indexar consultas IsUnderWater/GetWaterHeight por célula espacial"
priority: P2
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

# OPT-008 — Indexar consultas IsUnderWater/GetWaterHeight por célula espacial

**Classificação:** inspeção estática; candidato pós-port, não blocker de compilação ou gameplay. Não implementar antes do cliente realmente rodar no navegador PC e no mobile com baseline de performance. Problemas reais de segurança/correção comprovados devem ser promovidos a `.agents/issues` com reprodução; a classificação aqui não os isenta de correção.

## Subsistema
Água / teste de imersão

## Evidência observada
[`src/client153/Angelica2/Angelica3D/Source/A3DTerrainWater.cpp:365-410`](https://github.com/brasalabs6/pw-web/blob/87a7748491f94bdf16e455c303218393a039de94/src/client153/Angelica2/Angelica3D/Source/A3DTerrainWater.cpp#L365-L410) usa dois loops por grupo de altura e área para `IsUnderWater` e `GetWaterHeight`, testando o AABB de cada área. Diferentemente de OPT-007, essas consultas não percorrem todas as células internas.

## Hipótese / impacto possível
Busca O(número de áreas de água) por chamada pode consumir CPU se colisão, áudio, partículas ou câmeras consultarem muitas posições por frame; sem perfil de call sites não afirmar gargalo.

## Estratégia candidata (não implementada)
Avaliar indexação espacial por tile/célula + buckets de altura, consulta AABB e cache temporal por entidade; tratar áreas sobrepostas e preservar a ordem e o valor de retorno original. Coordenar índice com OPT-007 sem implementar duas estruturas redundantes.

## Plano de teste e aceite
Medir calls/frame, zonas com áreas sobrepostas, bordas exatas e diferentes alturas; 1/10/1.000 áreas; verificar equivalência de bool/altura, p95 tempo de consulta e memory overhead.

**Gate de aceite:** comparar mesma cena, conteúdo, qualidade, SHA, hardware e perfil de dispositivo; reportar p50/p95/p99 de tempo CPU/GPU (quando disponível), RAM/VRAM e regressões visuais/funcionais. Ganho não quantificado nesta revisão.

## Dependências
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23); baseline inspecionado `87a7748491f94bdf16e455c303218393a039de94`.
- [e07-world-terrain-v1](../epics/e07-world-terrain-v1/README.md).
- Consultar `README.md`, PW23-021…037 e OPT-001…006 antes de executar; documentar dedup e revalidar contra HEAD.
- Não tocar no servidor PR #17, não abrir terceiro PR, não fazer merge ou tuning do código nesta fase.
