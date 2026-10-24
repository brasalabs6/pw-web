# Epic Charter: Pipeline de recursos PCK/PKX, formatos e proveniência

- **Identificador:** `e05-asset-pipeline-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Especificar parsers e transformações seguras de recursos permitidos, incluindo PCK/PKX, nomes GBK, offsets 64-bit, materiais, modelos, texturas, checksum e manifestos.
- **Principal dependência:** `e04-wasm-runtime-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **arquivos >2GB e offsets truncados; bomba de descompressão e path traversal; incompatibilidade de versões e redistribuição**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- engine/formats
- tools/assets
- web/storage
- tests/fixtures

## Contratos de saída obrigatórios
- IArchiveIndex
- AssetManifest
- IAssetDecoder
- resource version map

## Critérios executivos de sucesso
- fixtures sintéticas com limites validados
- 0 path escapes
- 100% arquivos emitidos com hash e versão

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
