---
id: OPT-012
title: "Auditar limites de caminho e roteamento por prefixo no AFilePackMan"
priority: P1
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

# OPT-012 — Auditar limites de caminho e roteamento por prefixo no AFilePackMan

**Classificação:** inspeção estática; candidato pós-port, não blocker de compilação ou gameplay. Não implementar antes do cliente realmente rodar no navegador PC e no mobile com baseline de performance. Problemas reais de segurança/correção comprovados devem ser promovidos a `.agents/issues` com reprodução; a classificação aqui não os isenta de correção.

## Subsistema
PCK / lookup e hardening preventivo

## Evidência observada
[`src/client153/Angelica2/AngelicaFile/Source/AFilePackMan.cpp:212-229`](https://github.com/brasalabs6/pw-web/blob/87a7748491f94bdf16e455c303218393a039de94/src/client153/Angelica2/AngelicaFile/Source/AFilePackMan.cpp#L212-L229) copia `szPath` com `strncpy(szLowPath,szPath,MAX_PATH)` e em seguida itera até `\0` sem assegurar terminador caso o caminho tenha comprimento >= MAX_PATH. Em seguida percorre os pacotes e retorna o primeiro cujo prefixo corresponde ao caminho, sem ordem pelo prefixo mais específico.

## Hipótese / impacto possível
Há risco estático de leitura fora do buffer para caminho longo e possível roteamento ambíguo com roots aninhadas. Não foi reproduzido bug em browser, nem provada exploração. A busca linear por pacotes também pode custar CPU em assets numerosos.

## Estratégia candidata (não implementada)
Antes de alterar, definir um contrato de tamanho e canonicalização de caminhos, testar truncamento/Unicode/GBK/separadores e roots sobrepostos. Avaliar lookup em prefix trie/mapa de raízes normalizadas preservando compatibilidade e ordem original quando exigida. **Se crash/corrupção for demonstrado, transferir imediatamente para .agents/issues** como bug de correção prioritário.

## Plano de teste e aceite
Testes nativos ASan/UBSan e WASM com path comprimento 0, MAX_PATH-1, MAX_PATH, MAX_PATH+100, prefixo compartilhado, pacote shadowing, caracteres GBK e pastas com/sem separador. Medir correctness e p95 lookup antes-depois.

**Gate de aceite:** comparar mesma cena, conteúdo, qualidade, SHA, hardware e perfil de dispositivo; reportar p50/p95/p99 de tempo CPU/GPU (quando disponível), RAM/VRAM e regressões visuais/funcionais. Ganho não quantificado nesta revisão.

## Dependências
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23); baseline inspecionado `87a7748491f94bdf16e455c303218393a039de94`.
- [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md).
- Consultar `README.md`, PW23-021…037 e OPT-001…006 antes de executar; documentar dedup e revalidar contra HEAD.
- Não tocar no servidor PR #17, não abrir terceiro PR, não fazer merge ou tuning do código nesta fase.
