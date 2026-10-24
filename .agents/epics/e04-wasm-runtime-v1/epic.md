# Epic Charter: Toolchain C++/Emscripten e runtime WebAssembly

- **Identificador:** `e04-wasm-runtime-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Compilar componentes C++ portáveis para wasm32, configurar startup, heaps, memória, tratamento de erros, frame loop e interop JavaScript com reprodução determinística.
- **Principal dependência:** `e03-platform-portability-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **builds diferentes por emsdk; OOM e pointers inválidos; browser bloqueia SharedArrayBuffer**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- engine/core
- web/wasm
- scripts/build-wasm.sh
- tests/

## Contratos de saída obrigatórios
- module factory async
- API exportada C estável
- memory lifecycle
- JS↔WASM typed buffers

## Critérios executivos de sucesso
- paridade de unit tests
- tempo de inicialização medido
- peak linear memory sob orçamento definido

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
