---
id: OPT-010
title: "Evitar pesquisa repetida do hanger em modelos-filho"
priority: P2
status: candidate
kind: static-optimization
phase: post-browser-port
blocking: false
source_sha: 87a7748491f94bdf16e455c303218393a039de94
first_observed: 2026-10-28
validation: static-only
epic: e08-avatar-animation-v1
scope: PR-23
---

# OPT-010 — Evitar pesquisa repetida do hanger em modelos-filho

**Classificação:** inspeção estática; candidato pós-port, não blocker de compilação ou gameplay. Não implementar antes do cliente realmente rodar no navegador PC e no mobile com baseline de performance. Problemas reais de segurança/correção comprovados devem ser promovidos a `.agents/issues` com reprodução; a classificação aqui não os isenta de correção.

## Subsistema
Animação / vínculos de modelo

## Evidência observada
[`src/client153/Angelica2/Angelica3D/Source/A3DSkinModel.cpp:926-938`](https://github.com/brasalabs6/pw-web/blob/87a7748491f94bdf16e455c303218393a039de94/src/client153/Angelica2/Angelica3D/Source/A3DSkinModel.cpp#L926-L938) chama `UpdateAsChild` no caminho de atualização de filhos quando autoupdate está desligado; [`src/client153/Angelica2/Angelica3D/Source/A3DSkinModel.cpp:1026-1042`](https://github.com/brasalabs6/pw-web/blob/87a7748491f94bdf16e455c303218393a039de94/src/client153/Angelica2/Angelica3D/Source/A3DSkinModel.cpp#L1026-L1042) procura o hanger em todo `m_pParent->GetHangerNum()` a cada chamada.

## Hipótese / impacto possível
Com vários avatares equipados e muitos hangers no pai, o custo da procura pode crescer com itens anexados. Não há perfil indicando que seja mais caro que o trabalho dos ossos.

## Estratégia candidata (não implementada)
Na associação pai-filho, registrar handle/índice estável de hanger e invalidar em remoção/reattach/despawn; manter validação para mudanças dinâmicas. Analisar caso em que GetAutoUpdateChildFlag muda durante frame.

## Plano de teste e aceite
Avatares com 0/5/20/50 anexos por personagem e 1/50/200 personagens, alternância de pai e slot; validar posições de armas/itens, lifecycle, allocs e custo CPU.

**Gate de aceite:** comparar mesma cena, conteúdo, qualidade, SHA, hardware e perfil de dispositivo; reportar p50/p95/p99 de tempo CPU/GPU (quando disponível), RAM/VRAM e regressões visuais/funcionais. Ganho não quantificado nesta revisão.

## Dependências
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23); baseline inspecionado `87a7748491f94bdf16e455c303218393a039de94`.
- [e08-avatar-animation-v1](../epics/e08-avatar-animation-v1/README.md).
- Consultar `README.md`, PW23-021…037 e OPT-001…006 antes de executar; documentar dedup e revalidar contra HEAD.
- Não tocar no servidor PR #17, não abrir terceiro PR, não fazer merge ou tuning do código nesta fase.
