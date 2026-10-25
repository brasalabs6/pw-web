# Operação autônoma por workorder — versão 1

## O que REALMENTE funciona
- `Agent Execute` é iniciado manualmente por `workflow_dispatch` com `epic`, `workorder` e `attempt=1`.
- O contrato JSON versionado fornece descrição, escopo, testes e aceite; IDs arbitrários são recusados.
- Job **coder** (read-only para GitHub) usa `@xai-official/grok@1.0.70` em modo headless com `CF_PROXY_API_KEY`, produz patch e anota diagnóstico.
- Job **publish** (write de GitHub, sem `CF_PROXY_API_KEY`) verifica patch contra allowlist de caminhos e cria draft PR.
- Publica explicitamente `workflow_dispatch` do `Bootstrap CI` na branch do agente, pois PR criado com `GITHUB_TOKEN` pode não acionar eventos normais.
- CI salva `artifacts/ci/verify.log` e `report.json` (artefatos + resumo).
- `Agent CI Feedback` observa a conclusão do CI e comenta evidências. A decisão de nova rodada pertence exclusivamente ao `Agent Dual Review`, após os relatórios independentes DeepSeek + GLM Flash e o status da CI.
- Atualizações na branch ficam disponíveis para inspeção via PR. **Este chat não fica conectado em execução contínua:** posso ler resultados do GitHub durante uma sessão ou quando você voltar, enquanto a automação segue no GitHub por conta própria.

## Requisitos de configuração (ação humana)
1. Em GitHub repo Settings → Secrets and variables → Actions → Secrets, cadastrar `CF_PROXY_API_KEY` (chave API da sua organização, com limite de orçamento).
2. Em Actions → General → Workflow permissions, permitir operações necessárias a `contents:write`, `pull-requests:write`, `actions:write` e PRs criados por Actions, conforme política do repositório.
3. Rodar manualmente `Bootstrap CI` e conferir artefatos.
4. Rodar **primeiro** um workorder `automation: true` baixo risco via Actions → Agent Execute → Run workflow.
5. Verificar PR/diff/artefatos, ação por ação.
6. Após o piloto, definir `AGENT_REVIEW_LOOP_ENABLED=true` para habilitar reparo automático após dupla revisão + CI, usando `AGENT_MAX_REVIEW_ROUNDS=8` (limite técnico: 30). Ausente = desabilitado.
7. Definir regras de proteção de `main` com revisão humana + `Bootstrap CI` obrigatório, e limites de uso do Actions/xAI.

## O que NÃO automatizamos nesta fase
- Merge, deploy público, decisões jurídicas novas, credenciais, pagamentos, releases, alteração de workflows do próprio agente, leitura/escrita de servers de produção.
- Escolha automática de PRs sem revisão humana, ou despachador programado de backlog sem limites: programação de novas tasks requer uma governança adicional.
- Certificar funcionalidade Perfect World por checks de scaffold: apenas testes reprodutíveis do que foi realmente implementado.

## Modelo de estados
```text
workorder autorizado -> dispatch attempt1 -> Grok Build (read-only token, patch)
  -> patch validation (isolated write job) -> draft PR -> CI dispatch
  -> artifacts (log, report.json) -> workflow_run feedback
       ├─ CI verde + 2 reviews aprovados -> comment + human review
       ├─ findings/CI falha + AGENT_REVIEW_LOOP_ENABLED -> coder round N+1
       └─ limite, erro de review ou CI pendente -> bloqueio + triagem humana
```

## Threat model
- O modelo pode propor mudanças maliciosas/incorretas: patch é untrusted input. Validação em job separado com caminho, tamanho, tipo, symlink e quantidade limitados; CI independente.
- Workorders e validator vêm da `main`, e mudanças neles não são permitidas automaticamente.
- Não interpolar corpo de issue ou comentários em shell/Grok Build prompt. Inputs são IDs validados por regex e resolvidos localmente.
- GitHub token de write só no publish job; chave proxy Cloudflare só em steps de inferência dos 3 agentes, injetada somente na execução do Grok Build; usar sandbox estrito (`--sandbox strict`), com limite de 35 turnos e sem acesso do modelo ao token de escrita do GitHub.
- Não considerar testes verdes garantia de segurança. PR exige code review.
- GitHub Actions tem cobrança/cotas; limite por `AGENT_MAX_REVIEW_ROUNDS` (padrão 8, máximo 30), timeout por job, concurrency por workorder e ausência de auto-merge.
- Scripts de build da branch PR rodam CI **sem segredo** e podem ser maliciosos; colaboradores externos não devem ganhar write ao repositório.
- Atualizações nos workflows e paths de segurança exigem revisão específica. Recomenda-se branch protection/CODEOWNERS.

## Diagnóstico
- Sem secret? `Agent Execute` falhará no coder; não existirá PR, e isso é esperado.
- 403 ao abrir PR ou iniciar CI? Ajustar permissões do GitHub Actions de forma mínima.
- Runner sem emsdk? CI básico prova C++ nativo; executar `WASM Smoke` manual para wasm real.
- Patch recusado? Ver `validate_patch.py` e `write_scopes`; não afrouxar para permitir arquivos privilegiados.
- Execução falhou antes do PR? Logs e `pw-agent-patch-<run_id>` ficam em Agent Execute.
- Sem nova rodada? Checar `AGENT_REVIEW_LOOP_ENABLED=true`, os dois relatórios JSON válidos, status exato do CI e `AGENT_MAX_REVIEW_ROUNDS`. Erros de modelos ou CI pendente impedem reexecução.
- Como acompanhar: Actions → Agent Execute / Bootstrap CI / Agent CI Feedback; PR comments e artifacts.

## Limites importantes
Este workflow é infraestrutura para fazer PRs assistidos por IA e seus testes, não autonomia ilimitada e não um serviço que mantém uma sessão do ChatGPT aberta. Nenhuma chave, assinatura de serviço, token pessoal ou configuração externa é criada automaticamente.

## Validação integrada sem API key

Use `Agent Execute` com `dry_run=true`, `epic=e01-agent-orchestration-v1`, `workorder=wo-01`, `attempt=1`. O job `prepare` valida workorder e dependências; o job `dry-run` gera prompt (sem executar modelo), roda o verificador e grava artefatos com catalog, prompt e log. Isso comprova a ligação do controlador com contratos sem criar código/PR. **Não comprova geração por IA.**

## Execução programada da fila

`Agent Backlog Scheduler` é opt-in independente via variável `AGENT_BACKLOG_ENABLED=true`, e é documentado em `docs/AGENT_BACKLOG.md`. O scheduler seleciona workorders com dependências concluídas e nenhuma outra PR de agente pendente. `AGENT_REVIEW_LOOP_ENABLED` controla apenas reparos após os dois reviews, não o agendamento do backlog.

## Executor Grok Build (substituição do executor anterior)

- Instalamos `@xai-official/grok@1.0.70` em um diretório temporário, **sem** expor `CF_PROXY_API_KEY` ao npm/postinstall.
- Executamos `scripts/run-grok-workorder.sh` usando `grok --no-auto-update --sandbox strict --always-approve --no-memory --disable-web-search --max-turns 35 -p '<prompt>' --output-format json`.
- O valor de `-p` é o conteúdo do prompt gerado localmente a partir de `.agents/epics/.../workorders/*.json`; nenhuma issue externa instrui o agente.
- O shell do job *coder* não recebe `contents: write`, `pull-requests: write` nem um token GH com acesso de escrita. A chave `CF_PROXY_API_KEY` existe apenas no step Grok e fica fora dos logs de shell. A execução retorna JSON em `artifacts/agent/final.json` e um diff, a serem revisados isoladamente pelo job privilegiado.
- Sandbox `strict` limita filesystem e rede de processos-filhos no Linux, mas **não impede** o próprio agente de consultar a API do modelo. Não tratar sandbox como prova absoluta de isolamento: análise de prompts, revisão de dependências e gates do PR continuam obrigatórios.
- `--always-approve` é usado apenas em ambiente de execução limitado e headless; nunca significa aprovação de PR, licença ou release.
- Documentação oficial: https://docs.x.ai/build/overview e https://docs.x.ai/build/cli/headless-scripting.
- Seu plano/assinatura Grok Build local **não garante** que um runner GitHub Actions tenha acesso; a execução hospedada precisa da API key gerada em https://console.x.ai/ e pode ter cobrança independente. Se preferir um runner próprio autenticado, desenhar uma integração separada com armazenamento seguro de credenciais, sem copiar tokens de login para GitHub.
- O workflow de prova `Agent Controller Smoke` deve validar CLI instalada e teste com *mock* que não envia prompt à xAI. Somente o piloto real com segredo poderá atestar execução do modelo.

## 2 reviewers e looping por findings

O gatilho para code review é `Agent Dual Review`, conforme [docs/GROK_BUILD.md](GROK_BUILD.md). `Agent CI Feedback` apenas relata a CI, **não** reexecuta o coder em paralelo; a dupla revisão e o estado da CI são reunidos no controlador do PR, que decide o reparo.

- `AGENT_REVIEW_LOOP_ENABLED=true`: ativa solicitações de reparo automático; desligado por padrão.
- `AGENT_MAX_REVIEW_ROUNDS=8`: limite operacional padrão, máximo técnico 30, sem merge automático.
- A chave é o secret `CF_PROXY_API_KEY`; não é `XAI_API_KEY` neste proxy.
