---
id: OPT-011
title: "Substituir precache de todas as texturas por aquecimento incremental medido"
priority: P2
status: candidate
kind: static-optimization
phase: post-browser-port
blocking: false
source_sha: 87a7748491f94bdf16e455c303218393a039de94
first_observed: 2026-10-28
validation: static-only
epic: e05-asset-pipeline-v1
scope: PR-23
---

# OPT-011 — Substituir precache de todas as texturas por aquecimento incremental medido

**Classificação:** inspeção estática; candidato pós-port, não blocker de compilação ou gameplay. Não implementar antes do cliente realmente rodar no navegador PC e no mobile com baseline de performance. Problemas reais de segurança/correção comprovados devem ser promovidos a `.agents/issues` com reprodução; a classificação aqui não os isenta de correção.

## Subsistema
Texturas / cold start e GPU

## Evidência observada
[`src/client153/Angelica2/Angelica3D/Source/A3DTextureMan.cpp:276-321`](https://github.com/brasalabs6/pw-web/blob/87a7748491f94bdf16e455c303218393a039de94/src/client153/Angelica2/Angelica3D/Source/A3DTextureMan.cpp#L276-L321) define `PrecacheAllTexture`, altera estado/projeção do device e percorre toda `m_ListTexture`, chamando `Appear`, `DrawPrimitiveUP` e `Disappear` sob mutex.

## Hipótese / impacto possível
Se essa rotina for usada no cliente browser, uma única fase de aquecimento pode causar spike de GPU/CPU e bloquear o carregamento; pode não estar no hot path atual. Não confundir com OPT-006 (tick da animação) ou OPT-002 (deduplicação).

## Estratégia candidata (não implementada)
Confirmar primeiro os call sites e eficácia do precache; testar uploads sob demanda com fila incremental por orçamento ms/frame, prioridades por visibilidade, invalidar na perda do contexto e restaurar estado explicitamente. Não fazer draws invisíveis de todos os recursos se não necessário.

## Plano de teste e aceite
Cold start com 100/1.000/10.000 texturas, frames até primeira interação, long tasks, GPU upload stall, frame time p99 e textura faltante; preservar warm-cache correctness.

**Gate de aceite:** comparar mesma cena, conteúdo, qualidade, SHA, hardware e perfil de dispositivo; reportar p50/p95/p99 de tempo CPU/GPU (quando disponível), RAM/VRAM e regressões visuais/funcionais. Ganho não quantificado nesta revisão.

## Dependências
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23); baseline inspecionado `87a7748491f94bdf16e455c303218393a039de94`.
- [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md).
- Consultar `README.md`, PW23-021…037 e OPT-001…006 antes de executar; documentar dedup e revalidar contra HEAD.
- Não tocar no servidor PR #17, não abrir terceiro PR, não fazer merge ou tuning do código nesta fase.
