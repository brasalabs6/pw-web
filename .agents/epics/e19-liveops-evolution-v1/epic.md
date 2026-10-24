# Epic Charter: Operações contínuas, versionamento, conteúdo e manutenção

- **Identificador:** `e19-liveops-evolution-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Preparar operação após beta: versionamento de clientes/realms/protocolos, migrações, observabilidade, suporte, antiabuso, incidentes, governance e evolução sem regressões.
- **Principal dependência:** `e18-browser-ux-accessibility-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **migrar dados jogadores de forma errada; incompatibilidade patch/version; custos de banda/infra e abuso**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- ops/
- infra/
- gateway/
- docs/

## Contratos de saída obrigatórios
- VersionMatrix
- IncidentRecord
- PlayerSupportCase
- MaintenanceWindow
- FeatureFlag

## Critérios executivos de sucesso
- rollbacks sem perda indevida
- tempo de resposta incidente medido
- compatibilidade por release

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
