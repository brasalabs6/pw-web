---
id: OPT-013
title: "Validar dimensões, contagem de ondas e orçamento de alocação de A3DWaterArea"
priority: P1
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

# OPT-013 — Validar dimensões, contagem de ondas e orçamento de alocação de A3DWaterArea

**Escopo:** Water area binary deserialization. **Candidato preventivo pós-port**, não blocker comprovado. Não implementar tuning até o cliente WASM original executar no navegador desktop e celular e existir baseline de FPS, RAM/VRAM, frame time e estabilidade. Caso um crash/vulnerabilidade grave seja reproduzido, registrar a falha em `.agents/issues` com evidência, sem esperar pelo tuning.

## Evidência exata no SHA auditado
[`src/client153/Angelica2/Angelica3D/Source/A3DWaterArea.cpp:310-375`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/src/client153/Angelica2/Angelica3D/Source/A3DWaterArea.cpp#L310-L375) lê `m_nWidth` e `m_nHeight` como `int`, aloca `new BYTE[m_nWidth * m_nHeight]` e lê o produto em bytes sem validação explícita de limites nem overflow. O mesmo padrão reaparece nas versões antigas ([`src/client153/Angelica2/Angelica3D/Source/A3DWaterArea.cpp:385-445`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/src/client153/Angelica2/Angelica3D/Source/A3DWaterArea.cpp#L385-L445)). `nNumWaves` dirige um loop de alocação por onda sem limite explícito. [`src/client153/Angelica2/Angelica3D/Source/A3DWaterArea.cpp:762-780`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/src/client153/Angelica2/Angelica3D/Source/A3DWaterArea.cpp#L762-L780) pode ainda criar muitos mesh blocks em função das dimensões.

## Impacto possível e grau de certeza
Entradas inválidas ou corrompidas podem levar a alocação exagerada, estouro aritmético, loops prolongados ou OOM do heap WASM; não há crash/exploração reproduzidos. O tamanho e o custo de malhas precisam ser medidos.

## Estratégia candidata, sem mudança de código nesta rodada
Em fase posterior, definir limites por formato e budget RAM/CPU por mapa/área, validar dimensões positivas, multiplicação checked em size_t, limite de ondas e bytes restantes antes de alocar, limpeza transacional de objetos parcialmente lidos; rejeitar arquivos inválidos sem deixar estado inconsistente. Validar contra os assets originais para não rejeitar mapas legítimos.

## Testes e critério de aceite pós-port
Corpus de arquivos de água reais e mutações de largura/altura negativas, zero, INT_MAX, produto overflow, nNumWaves negativo/alto, truncamento e falha de alocação; ASan/UBSan nativo e Emscripten bounds, medir peak heap e tempo de parse, assegurar compatibilidade byte a byte de mapas válidos.

**Gate de aceitação:** revalidar contra HEAD, registrar dispositivo/SHA/cena/qualidade, before/after com CPU/GPU frame time p50/p95/p99, memória e regressões funcionais; não prometer ganhos antes dos testes.

## Deduplicação e dependências
Distinto de OPT-007/008 (consultas de água), OPT-012 (roteamento de caminhos PCK), PW23-025 (SKI) e PW23-033 (budget geral WASM).
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA auditado `07b8e843acd4a019d2fda991bf14c2f5b63433f1`.
- [e07-world-terrain-v1](../epics/e07-world-terrain-v1/README.md).
- Preservar PW23-021…037 e OPT-001…012; não abrir PR, alterar o servidor ou realizar auto-merge.
