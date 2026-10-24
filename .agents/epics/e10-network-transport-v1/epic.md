# Epic Charter: GNET, protocolo de mundo e gateway WSS seguro

- **Identificador:** `e10-network-transport-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Implementar transporte binário browser↔gateway↔servidor, parsing GNET/gamedata por versão, negociação de sessão, auth, backpressure e cobertura byte-a-byte.
- **Principal dependência:** `e09-interface-input-audio-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **endianness e framing; compatibilidade 1.5.3↔realm v155; flood/malformed packets e auth bypass**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- gateway/
- engine/network
- tests/protocol
- docs/

## Contratos de saída obrigatórios
- PacketReader/Writer
- ProtocolVersion
- ClientSession
- WsGateway
- PacketTrace

## Critérios executivos de sucesso
- fixtures de ida/volta byte-a-byte
- max payload definido
- gateway sem exposição do barramento interno

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
