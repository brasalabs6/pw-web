# Epic Charter: Renderer WebGL2 da engine Angelica

- **Identificador:** `e06-render-webgl2-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Substituir estado Direct3D8/9 da Angelica por backend WebGL2 verificável, com recursos, vertex formats, depth/cull/blend, fixed function em shaders e gestão de contexto.
- **Principal dependência:** `e05-asset-pipeline-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **semântica D3D↔GL divergente; shaders D3DX indisponíveis; perda do contexto GPU e leaking buffers**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- engine/render
- web/renderer
- tests/render

## Contratos de saída obrigatórios
- IRenderDevice
- ITexture
- IMeshBuffer
- IPipelineState
- IMaterial

## Critérios executivos de sucesso
- golden images com tolerância registrada
- frametime e draw calls
- recursos GPU liberados sem vazamento

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
