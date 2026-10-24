# Executor Grok Build no PW Web

## Por que Grok Build

O `pw-web` usa o Grok Build oficial da xAI como agente de programação, executando workorders JSON em sessões headless e gerando patches para validação do GitHub Actions. Os épicos e contratos existentes permanecem iguais.

**Referências oficiais**:
- https://docs.x.ai/build/overview
- https://docs.x.ai/build/cli/headless-scripting
- https://docs.x.ai/build/features/sandbox
- https://www.npmjs.com/package/@xai-official/grok
- https://github.com/xai-org/grok-build

## Autenticação e custos no GitHub-hosted runner

Seu login no `grok` local é uma sessão separada; não é automaticamente disponível no GitHub Actions. Para o runner Ubuntu crie uma **chave API xAI** (https://console.x.ai) e salve **somente** em `Settings → Secrets and variables → Actions → XAI_API_KEY`.

- Não salve a chave em `.env`, código, commit, logs ou issue.
- Configure orçamento e limites de gasto da API xAI. Assinaturas Grok/SuperGrok não devem ser presumidas como cotas de API.
- A chave xAI aparece **só** no job `coder`, no step de execução. O job `publish` tem token GitHub com write, mas **não** a chave API.
- Antes de ligar o scheduler, audite o risco de injeção em workorders/arquivos de repo e passe por revisão de segurança (issue #11).

## Versão de execução

O GitHub Actions instala a dependência oficial npm **fixada em `@xai-official/grok@1.0.70`**, via `npm install --prefix "$RUNNER_TEMP/grok-build" --no-audit --no-fund`. O próprio npm roda em um step **sem** segredo da API. Confirme o binário `grok --version`.

Os comandos headless são encapsulados em `scripts/run-grok-workorder.sh`, usando:

```bash
grok --no-auto-update \
  --sandbox strict \
  --always-approve \
  --no-memory \
  --disable-web-search \
  --max-turns 35 \
  -p "$(cat artifacts/agent/prompt.md)" \
  --output-format json
```

`strict` limita os subprocessos e escrita/leitura no Linux; a API do modelo continua acessível. O agente ainda pode executar comandos, então seu output é tratado como **não confiável**. O patch passa por análise independente e somente depois entra num PR draft. Nenhum merge automático.

## Teste local opcional

```bash
# Com Grok Build instalado na máquina; não usar chave no comando
export XAI_API_KEY="...apenas-no-seu-terminal..."
python3 .agents/automation/build_prompt.py \
  --epic e01-agent-orchestration-v1 --workorder wo-01 \
  --attempt 1 --output /tmp/pw-web-workorder-prompt.md
bash scripts/run-grok-workorder.sh /tmp/pw-web-workorder-prompt.md /tmp/pw-web-grok-result.json
```

**Atenção:** a execução local pode alterar seu checkout. Use branch/worktree isolado, verifique diffs antes de commitar e não execute isso com material/protocolos de terceiros não autorizados. Não há loop GitHub neste modo local.

## Provas e visibilidade

1. `Bootstrap CI` e `Agent Controller Smoke` validam projetos/testes sem segredo.
2. Smoke da CLI verifica instalação e argumentos do `grok` sem fazer chamada de modelo.
3. Após configurar `XAI_API_KEY`, rode `Agent Execute` com `dry_run=true` primeiro.
4. Para execução real, `dry_run=false`; acompanhe `Agent Execute`, `Bootstrap CI`, `Agent CI Feedback`, artifacts de patch/JSON e PR.
5. Retry automático somente se `AGENT_AUTONOMY_ENABLED=true`; backlog somente com `AGENT_BACKLOG_ENABLED=true`; ambos desativados por padrão.

## Limitações

- Não houve execução autenticada do Grok Build no repositório enquanto a chave não estiver configurada. A verificação de CLI/dry-run é distinta.
- A CLI Grok Build tem comandos interativos e ACP; **este pipeline utiliza headless**, não `grok agent stdio`, nem `/goal` interativo.
- Rotação de versões npm exige PR, revisão da supply chain e atualização dos testes. Fixar por versão reduz deriva, mas não equivale a fixar integralmente o hash da cadeia de dependências.
