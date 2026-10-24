# Epic Charter: Estratégia de testes, tracing, replay e QA automatizado

- **Identificador:** `e16-qa-observability-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Fazer toda capacidade depender de logs e testes objetivos: unit, integração, protocolo, render golden, Browser E2E, regressão, fuzz, perf e relatórios de evidência.
- **Principal dependência:** `e15-assets-streaming-perf-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **falso verde do scaffold; fixtures com direitos incertos; testes flaky e builds não determinísticas**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- tests/
- tools/qa
- docs/
- web/telemetry

## Contratos de saída obrigatórios
- TestMatrix
- EvidenceReport
- PacketReplay
- FrameTrace
- BuildProvenance

## Critérios executivos de sucesso
- gates por cenário reais
- flakiness registrada
- evidências por commit e navegador

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
