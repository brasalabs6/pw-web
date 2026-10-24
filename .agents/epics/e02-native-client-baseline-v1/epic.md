# Epic Charter: Baseline do cliente Windows e seleção validada da Angelica

- **Identificador:** `e02-native-client-baseline-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Medir build real do cliente 1.5.3 D3D8 versus alternativa 1.5.2 D3D9, registrar dependências, inicialização, cena, login e decisão por evidência técnica e direitos.
- **Principal dependência:** `e01-agent-orchestration-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **repositório incompleto; SDK legado indisponível; cliente não autentica em servidor atual**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- references/client-153
- references/client-152-modern
- documentos de build Windows
- testes de comparação

## Contratos de saída obrigatórios
- matriz de compiladores e SDK
- log/sha de executável
- matriz client↔assets↔protocol↔server

## Critérios executivos de sucesso
- reprodução com toolchain documentada
- 0 erros não documentados no build qualificado
- versão por screenshot/hash/protocolo

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
