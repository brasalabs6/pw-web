# Epic Charter: Movimento, combate, skills, NPCs, quests e interação

- **Identificador:** `e12-entities-gameplay-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Integrar gameplay com semântica cliente original: navegação, seleção de alvo, habilidades, recursos, buffs, ataques, quests, NPC, loot, voo e montarias.
- **Principal dependência:** `e11-login-session-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **cliente altera estado servidor-autoritativo; desync movimento/skill; protocol opcode incompleto**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- engine/game
- engine/animation
- web/ui
- tests/gameplay

## Contratos de saída obrigatórios
- EntityState
- ActionCommand
- SkillCast
- TargetSelection
- QuestState

## Critérios executivos de sucesso
- estado cliente converge com servidor
- latência ação→ack medida
- regressões por habilidade

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
