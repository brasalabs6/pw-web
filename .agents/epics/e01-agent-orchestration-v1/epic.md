# Epic Charter: Orquestração autônoma, isolamento e governança de agentes

- **Identificador:** `e01-agent-orchestration-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Controlar trabalho de agentes por workorders versionados, branch por escopo, sandbox, validação de patches, provas em CI, reparo limitado e intervenção humana em operações sensíveis.
- **Principal dependência:** `e00-rights-governance-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **prompt injection via PR e logs; chave API exposta a código não confiável; loops, race conditions e custo descontrolado**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- .agents/automation
- GitHub Actions
- GitHub PR/Issues
- revisões e logs

## Contratos de saída obrigatórios
- workorder JSON + allowlist
- Grok Build patch artifact
- CI report.json
- PR comments e controladores workflow_run

## Critérios executivos de sucesso
- 100% patches validados fora do job com chave
- tentativas por workorder <=3
- 100% PRs com log e hash de CI

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
