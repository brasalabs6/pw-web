# Epic Charter: Terreno, cidades, vegetação, água e visibilidade do mundo

- **Identificador:** `e07-world-terrain-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Reconstruir a apresentação dos mapas de Perfect World com streaming de setores, terreno, lightmap, vegetação, água, objetos estáticos, neblina e culling.
- **Principal dependência:** `e06-render-webgl2-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **coordenadas, escala e eixos divergentes; costuras de terreno; carregamento de chunks gera stutter**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- engine/world
- engine/terrain
- engine/render
- web/storage

## Contratos de saída obrigatórios
- WorldChunk
- HeightField
- VisibilitySet
- SceneGraph
- IChunkLoader

## Critérios executivos de sucesso
- alinhamento de posição e altura
- p95 frame time ao trocar setor
- memória limitada por janela de mundo

## Fora do escopo
- Distribuir código/assets de terceiros sem permissão documentada.
- Substituir testes por declarações verbais ou prints sem contexto.
- Liberar alterações diretamente em produção; workorders entram em PR, com revisão e CI.
- Tratar fixtures sintéticas como prova da funcionalidade integral do Perfect World.

## Dependências e desbloqueio
- Referência de cliente/engine/protocolo explicitamente identificada em testes.
- Contratos e controles em `AGENTS.md` seguidos integralmente.
- Ambientes de desenvolvimento isolados e artefatos reproduzíveis.
- Decisões de mudança de stack, versão ou direitos documentadas em ADR.

## Definition of done
Cinco workorders com evidência verificável; aceitação e limites documentados; matriz de riscos atualizada; testes unitários/integrados pertinentes executados; revisão técnica e, quando aplicável, revisão jurídica.
