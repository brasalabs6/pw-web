# e01-agent-orchestration-v1

**Orquestração autônoma, isolamento e governança de agentes · v1.0.0 · status: planejado**

Controlar trabalho de agentes por workorders versionados, branch por escopo, sandbox, validação de patches, provas em CI, reparo limitado e intervenção humana em operações sensíveis.

**Arquivos de especificação:** [Epic](epic.md) · [Requisitos](requirements.md) · [Arquitetura](architecture.md) · [Implementação](implementation.md) · [Testes](testing.md) · [Riscos](risks.md) · [Tasks](tasks.md) · [Task resumida](task.md).

Cada workorder está em `workorders/wo-0N.json` e `workorders/wo-0N.md`, com dependências, limites de alterações, critérios de aceite e autonomia individual. **Nenhuma etapa concluída por este plano**; progresso depende de resultados verificados na CI/PR. Consulte `docs/AUTONOMY.md` e `docs/LEGAL.md`.
