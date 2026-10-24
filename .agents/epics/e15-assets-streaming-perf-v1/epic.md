# Epic Charter: Streaming, CDN, cache OPFS e desempenho do navegador

- **Identificador:** `e15-assets-streaming-perf-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Baixar assets necessários sob demanda com manifesto e Range requests, cache local, budget de RAM/VRAM, política de versões e frames estáveis.
- **Principal dependência:** `e14-items-economy-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **pacote monolítico multigigabyte; cache corrompido; queda de FPS em streaming e memória estourada**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- web/storage
- web/streaming
- tools/assets
- engine/resources

## Contratos de saída obrigatórios
- ManifestVersion
- ChunkRequest
- AssetPriorityQueue
- CacheEntry
- PerfSample

## Critérios executivos de sucesso
- bytes até primeiro frame
- p95 frame time durante streaming
- taxa hit cache, espaço consumido

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
