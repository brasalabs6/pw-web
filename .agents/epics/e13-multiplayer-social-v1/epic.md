# Epic Charter: Sincronização MMO, presença, chat, grupo e guilda

- **Identificador:** `e13-multiplayer-social-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Escalar sincronização de entidades, visibilidade AOI, chat, equipe, guildas, trading social e estado persistente com limites adequados a navegador.
- **Principal dependência:** `e12-entities-gameplay-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **pacotes fora de ordem; vazamento de chat ou identidade; AOI e tempo real sobrecarregam frame**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- engine/netstate
- gateway/
- web/social
- tests/multiplayer

## Contratos de saída obrigatórios
- EntitySnapshot
- AOISubscription
- ChannelMessage
- PartyRoster
- GuildState

## Critérios executivos de sucesso
- 2 clientes sincronizados sob cenários de movimentação
- limite de msgs/s
- ressincronização após perda

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
