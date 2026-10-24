# Epic Charter: UI do jogo, HUD, entrada, câmera e áudio

- **Identificador:** `e09-interface-input-audio-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Criar apresentação de HUD, janelas, ações por mouse/teclado/toque, câmera, atalhos, chat e mixagem de som sem bloquear o loop da engine.
- **Principal dependência:** `e08-avatar-animation-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **web browser consome teclas/scroll; latência de UI; recurso sonoro com direitos incertos**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- web/ui
- web/input
- web/audio
- engine/ui

## Contratos de saída obrigatórios
- InputAction
- HUDState
- CameraController
- AudioEvent
- UIMessageBus

## Critérios executivos de sucesso
- contratos de UX reproduzíveis
- latência input→frame
- nenhum asset de áudio sem autorização

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
