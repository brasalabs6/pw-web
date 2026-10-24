# Epic Charter: Inventário, equipamentos, crafting, loja e economia

- **Identificador:** `e14-items-economy-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Reproduzir telas e protocolos de inventário, stack, equipamentos, comércio, leilão, craft, banco e transações com validação 100% server-side.
- **Principal dependência:** `e13-multiplayer-social-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **dup de item; replay/ordem de trade; manipulação de preço por cliente**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- web/inventory
- engine/items
- gateway/
- tests/economy

## Contratos de saída obrigatórios
- ItemInstance
- InventorySlot
- EquipTransaction
- MerchantOffer
- TradeProposal

## Critérios executivos de sucesso
- invariantes contra duplicação
- idempotência de transação
- auditoria de trade no servidor

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
