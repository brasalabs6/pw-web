---
id: OPT-043
title: "A3DHorizonCull: custo de varredura por coluna e hardening de segmentos degenerados"
priority_after_port: P2
status: hypothesis-static
phase: post-browser-port
scope: pr-23-client153-angelica
inspected_sha: c4eb8fe8cb8262a217bdae69789174cae334db64
observed_at: 2026-10-28
---

# OPT-043 — Horizon culling por coluna de tela

## Evidência estática (não é defeito reproduzido)

A unidade original `A3DHorizonCull.cpp` foi adicionada à compilação de objetos Angelica em [`src/client153/CMakeLists.txt:45-50`](https://github.com/brasalabs6/pw-web/blob/c4eb8fe8cb8262a217bdae69789174cae334db64/src/client153/CMakeLists.txt#L45-L50). Isso **não prova** que o cliente WASM a invoque no gameplay, nem que o módulo completo esteja linkado ou executando.

- [`PrepareToWork:113-121`](https://github.com/brasalabs6/pw-web/blob/c4eb8fe8cb8262a217bdae69789174cae334db64/src/client153/Angelica2/Angelica3D/Source/A3DHorizonCull.cpp#L113-L121) redimensiona e zera um vetor `float` com uma entrada por coluna da viewport; [`Resize:141-161`](https://github.com/brasalabs6/pw-web/blob/c4eb8fe8cb8262a217bdae69789174cae334db64/src/client153/Angelica2/Angelica3D/Source/A3DHorizonCull.cpp#L141-L161) realoca quando a largura muda.
- [`BlockVisibility:279-350`](https://github.com/brasalabs6/pw-web/blob/c4eb8fe8cb8262a217bdae69789174cae334db64/src/client153/Angelica2/Angelica3D/Source/A3DHorizonCull.cpp#L279-L350) testa até quatro arestas de cada quad e insere arestas oclusoras, com contadores já existentes (`m_iNumTest`, `m_iNumInsert`).
- [`TestLine:438-508`](https://github.com/brasalabs6/pw-web/blob/c4eb8fe8cb8262a217bdae69789174cae334db64/src/client153/Angelica2/Angelica3D/Source/A3DHorizonCull.cpp#L438-L508) percorre colunas `x` para comparar alturas até encontrar uma coluna visível; [`InsertLine:511-560`](https://github.com/brasalabs6/pw-web/blob/c4eb8fe8cb8262a217bdae69789174cae334db64/src/client153/Angelica2/Angelica3D/Source/A3DHorizonCull.cpp#L511-L560) percorre colunas para atualizar o horizonte. O custo é proporcional ao comprimento horizontal projetado das arestas, multiplicado pela quantidade de blocos/arestas testados.
- [`TestLine:461-464`](https://github.com/brasalabs6/pw-web/blob/c4eb8fe8cb8262a217bdae69789174cae334db64/src/client153/Angelica2/Angelica3D/Source/A3DHorizonCull.cpp#L461-L464) e [`InsertLine:523-526`](https://github.com/brasalabs6/pw-web/blob/c4eb8fe8cb8262a217bdae69789174cae334db64/src/client153/Angelica2/Angelica3D/Source/A3DHorizonCull.cpp#L523-L526) dividem `dy / dx` sem guarda explícita de `dx == 0`; o impacto de segmentos verticais/degenerados não foi reproduzido. Não classificar como vulnerabilidade, crash ou blocker.

## Hipótese de impacto

Em cenas de terreno com muitos blocos e grande largura de render (DPR elevado), o algoritmo pode consumir tempo de CPU/WASM proporcional à largura da tela e ao número de arestas visíveis, com potencial de energia e latência de frame em mobile. A memória residente desse vetor é linear na largura (aproximadamente 4 bytes por coluna, fora do overhead), **não** há medição de RAM, FPS, GPU ou consumo real. O culling também pode poupar draw calls; não substituir sem medir ganho líquido nem sem preservar a mesma visibilidade.

## Plano de benchmark após browser port funcional

1. Gate obrigatório: primeiro cliente Angelica 1.5.3 real com login, mundo, render, câmera e controles no Chrome desktop e Android físico. Fixar SHA, cena, câmera, resolução CSS, DPR e qualidade.
2. Instrumentar tempo inclusivo/exclusivo de `PrepareToWork`, `BlockVisibility`, `TestLine`, `InsertLine`; contar arestas, colunas percorridas, blocos aceitos/rejeitados e draw calls poupadas. Medir frame time CPU/GPU p50/p95/p99, FPS 1% low, memória/heap e energia/temperatura durante soak, com e sem culling quando tecnicamente seguro.
3. Matriz: 720p/1080p/1440p físicos, DPR 1/2/3, terreno aberto, cidade e horizonte complexo, câmera parada/em movimento; golden frames para identificar falsos positivos de oclusão.
4. Somente se custo medido justificar: avaliar varredura por spans/coarse tiles, resolução reduzida do buffer de horizonte, early-out e guarda de segmentos degenerados, comparando antes/depois na mesma cena. Preservar correção visual e benefício de culling; testar casos verticais, largura 0/1, context loss e resize.

## Deduplicação e prioridade

Distinto de [PW23-036](PW23-036-scene-entity-lod-culling-ps2-style.md), que aborda política geral de LOD/culling de cena, e de [OPT-015](OPT-015-terrain-render-slot-lookup-reuse.md), que trata busca/sincronização de render slots; aqui o foco é o rasterizador de horizonte por coluna de `A3DHorizonCull`. Nenhuma alteração de código ou issue é autorizada com esta hipótese. **Fase: post-browser-port**, após evidência de execução real em PC e celular. Sem subagentes, compilação local, profiling ou testes mobile nesta inspeção.
