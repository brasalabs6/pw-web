# Epic Charter: Identidade, login, personagem e sessão

- **Identificador:** `e11-login-session-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Completar autenticação, enumeração/criação/seleção de personagem, transição para mundo, falhas de login e recuperação de sessão mantendo autoridade do servidor.
- **Principal dependência:** `e10-network-transport-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **sessão roubada; ticket em URL ou log; duplicidade de login e replay**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- web/auth
- engine/session
- gateway/
- tests/session

## Contratos de saída obrigatórios
- AuthStateMachine
- SessionTicket
- CharacterList
- EnterWorldRequest
- SessionError

## Critérios executivos de sucesso
- sem credencial em client logs
- testes de retry/expiry
- fluxo login→world repetível

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
