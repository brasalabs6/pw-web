## Resumo e hipótese

> O que muda, por quê e qual comportamento observável deve existir?

## Escopo, workorder e dependências

- Épico/workorder (ou **pesquisa manual sem workorder**):
- Arquivos alterados e efeitos colaterais:
- Dependências/gates humanos:
- Estado: **DRAFT / READY** (justificativa):

## Implementação e decisões

- Código ou documentação entregue:
- ADR/contrato técnico relacionado:

## Evidências reproduzíveis

| Teste / evidência | Comando exato ou link | Resultado (PASS / FAIL / BLOCKED / NOT RUN) |
| --- | --- | --- |
| Unit/static/C++ | | |
| CI (SHA específico) | | |
| WASM/browser, se aplicável | | |
| Testes negativos | | |
| POC original Angelica, se aplicável | | |

**Nota:** Chrome WebGL da demo ≠ port do renderer original da Angelica. Resultados locais não são CI.

## Revisão e findings

- Reviewer 1 (DeepSeek ou humano; link/estado):
- Reviewer 2 (GLM Flash ou humano; link/estado):
- Findings corrigidos ou explicitamente pendentes:
- PR de pesquisa `feat/*` não aciona automaticamente workflow de review de branches `agent/*`.

## Direitos, segurança e limitações

- Proveniência/autorização de código/assets:
- Ferramentas/ambientes indisponíveis ou testes não executados:
- Riscos residuais, incluindo SwiftShader versus GPU real:
- Credenciais e recursos de terceiros excluídos do patch:

## Rollback e próximos passos

- Forma de reverter:
- Gate que falta antes do merge:

## Checklist de publicação

- [ ] Li `.agent/skills/create-pr/SKILL.md`.
- [ ] Nenhum trabalho paralelo substituído.
- [ ] Diff revisto, escopos seguros e nenhum segredo/asset original adicionado.
- [ ] Evidências reais, comandos e logs discriminados.
- [ ] Verificação jurídica e dependências manuais respeitadas.
- [ ] Draft mantido enquanto houver WIP ou bloqueio.
- [ ] Merge somente com aprovação humana.
