# Epic Charter: Avatares, esqueletos, animações e customização

- **Identificador:** `e08-avatar-animation-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Carregar malhas e hierarquias de ossos, skinning e blend de clips, partes de equipamento, morph/face, movimentos, montaria e voo com sincronização de rede.
- **Principal dependência:** `e07-world-terrain-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **formato binário incompatível; hierarquias de bones divergentes; diferença GPU/CPU skinning e retargeting**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- engine/animation
- engine/character
- engine/formats
- engine/render

## Contratos de saída obrigatórios
- Skeleton
- SkinMesh
- AnimationClip
- AvatarAppearance
- AnimationStateMachine

## Critérios executivos de sucesso
- erro de pose dentro de tolerância
- custo ms/avatar
- 100 personagens simulados no teste de carga

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
