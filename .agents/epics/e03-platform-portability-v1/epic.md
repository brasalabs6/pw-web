# Epic Charter: Desacoplamento de Win32, SDK e Direct3D

- **Identificador:** `e03-platform-portability-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Criar fronteiras C++ portáveis para relógio, janela, threads, arquivos, eventos, memória, locale, áudio e render, minimizando dependências Windows no núcleo.
- **Principal dependência:** `e02-native-client-baseline-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **dependências ocultas em headers transversais; ABI packing e UTF/GBK; semântica de threads/cancelamento divergente**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- engine/core
- engine/platform
- engine/render
- tests/

## Contratos de saída obrigatórios
- IClock
- IFileSystem
- IWindowInput
- IRenderDevice
- INetworkTransport

## Critérios executivos de sucesso
- zero includes Windows no core
- testes de equivalência nativa e browser
- matriz APIs substituídas por módulo

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
