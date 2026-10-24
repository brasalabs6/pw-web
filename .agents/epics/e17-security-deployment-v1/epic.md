# Epic Charter: Segurança, infraestrutura, CI/CD e entrega

- **Identificador:** `e17-security-deployment-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Provisionar distribuição segura de build web e gateway com TLS, CSP, isolamento de realm, secrets, observabilidade, rollbacks e proibição de releases sem IP liberado.
- **Principal dependência:** `e16-qa-observability-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **expor pw-gs/postgres à internet; supply chain de actions; deploy de conteúdo sem direitos**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- infra/
- gateway/
- web/
- docs/

## Contratos de saída obrigatórios
- ReleaseManifest
- DeploymentConfig
- HealthCheck
- SecurityGate
- RollbackPlan

## Critérios executivos de sucesso
- 0 portas internas expostas
- releases com SBOM/proveniência
- tempo rollback testado

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
