# Operação autônoma por workorder — versão 1

## O que REALMENTE funciona
- `Agent Execute` é iniciado manualmente por `workflow_dispatch` com `epic`, `workorder` e `attempt=1`.
- O contrato JSON versionado fornece descrição, escopo, testes e aceite; IDs arbitrários são recusados.
- Job **coder** (read-only para GitHub) usa `openai/codex-action@v1` com `OPENAI_API_KEY`, produz patch e anota diagnóstico.
- Job **publish** (write de GitHub, sem `OPENAI_API_KEY`) verifica patch contra allowlist de caminhos e cria draft PR.
- Publica explicitamente `workflow_dispatch` do `Bootstrap CI` na branch do agente, pois PR criado com `GITHUB_TOKEN` pode não acionar eventos normais.
- CI salva `artifacts/ci/verify.log` e `report.json` (artefatos + resumo).
- `Agent CI Feedback` acompanha `workflow_run` para branches `agent/...`, comenta resultado e **só** se `AGENT_AUTONOMY_ENABLED=true`, quando CI falha, agenda reparação 2 ou 3. Após 3 tentativas para; quando passa, aguarda revisão humana.
- Atualizações na branch ficam disponíveis para inspeção via PR. **Este chat não fica conectado em execução contínua:** posso ler resultados do GitHub durante uma sessão ou quando você voltar, enquanto a automação segue no GitHub por conta própria.

## Requisitos de configuração (ação humana)
1. Em GitHub repo Settings → Secrets and variables → Actions → Secrets, cadastrar `OPENAI_API_KEY` (chave API da sua organização, com limite de orçamento).
2. Em Actions → General → Workflow permissions, permitir operações necessárias a `contents:write`, `pull-requests:write`, `actions:write` e PRs criados por Actions, conforme política do repositório.
3. Rodar manualmente `Bootstrap CI` e conferir artefatos.
4. Rodar **primeiro** um workorder `automation: true` baixo risco via Actions → Agent Execute → Run workflow.
5. Verificar PR/diff/artefatos, ação por ação.
6. Quando estiver satisfeito, definir repo variable `AGENT_AUTONOMY_ENABLED=true` para habilitar **apenas** retry automático por falha (até 3 tentativas). Default ausente = desligado.
7. Definir regras de proteção de `main` com revisão humana + `Bootstrap CI` obrigatório, e limites de uso do Actions/OpenAI.

## O que NÃO automatizamos nesta fase
- Merge, deploy público, download de conteúdo proprietário, decisões jurídicas, credenciais, pagamentos, releases, alteração de workflows do próprio agente, leitura/escrita de servers de produção.
- Escolha automática de PRs sem revisão humana, ou despachador programado de backlog sem limites: programação de novas tasks requer uma governança adicional.
- Certificar funcionalidade Perfect World por checks de scaffold: apenas testes reprodutíveis do que foi realmente implementado.

## Modelo de estados
```text
workorder autorizado -> dispatch attempt1 -> Codex (read-only token, patch)
  -> patch validation (isolated write job) -> draft PR -> CI dispatch
  -> artifacts (log, report.json) -> workflow_run feedback
       ├─ success -> comment + human review
       ├─ failure + AUTONOMY_ENABLED + attempt<3 -> attempt2/3
       └─ attempt3 fail -> comment + human triage
```

## Threat model
- O modelo pode propor mudanças maliciosas/incorretas: patch é untrusted input. Validação em job separado com caminho, tamanho, tipo, symlink e quantidade limitados; CI independente.
- Workorders e validator vêm da `main`, e mudanças neles não são permitidas automaticamente.
- Não interpolar corpo de issue ou comentários em shell/codex prompt. Inputs são IDs validados por regex e resolvidos localmente.
- GitHub token de write só no publish job; chave OpenAI só no coder job, processada por action oficial. `drop-sudo`.
- Não considerar testes verdes garantia de segurança. PR exige code review.
- GitHub Actions tem cobrança/cotas; máximo 3 tentativas, timeout por job, concurrency por workorder e ausência de auto-merge.
- Scripts de build da branch PR rodam CI **sem segredo** e podem ser maliciosos; colaboradores externos não devem ganhar write ao repositório.
- Atualizações nos workflows e paths de segurança exigem revisão específica. Recomenda-se branch protection/CODEOWNERS.

## Diagnóstico
- Sem secret? `Agent Execute` falhará no coder; não existirá PR, e isso é esperado.
- 403 ao abrir PR ou iniciar CI? Ajustar permissões do GitHub Actions de forma mínima.
- Runner sem emsdk? CI básico prova C++ nativo; executar `WASM Smoke` manual para wasm real.
- Patch recusado? Ver `validate_patch.py` e `write_scopes`; não afrouxar para permitir arquivos privilegiados.
- Execução falhou antes do PR? Logs e `pw-agent-patch-<run_id>` ficam em Agent Execute.
- No loop de retry? Verificar GitHub variable `AGENT_AUTONOMY_ENABLED=true`; retry é só em CI disparada com `workflow_dispatch` e commit marcado.
- Como acompanhar: Actions → Agent Execute / Bootstrap CI / Agent CI Feedback; PR comments e artifacts.

## Limites importantes
Este workflow é infraestrutura para fazer PRs assistidos por IA e seus testes, não autonomia ilimitada e não um serviço que mantém uma sessão do ChatGPT aberta. Nenhuma chave, assinatura de serviço, token pessoal ou configuração externa é criada automaticamente.
