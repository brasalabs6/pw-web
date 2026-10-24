# Fila autônoma determinística (opt-in)

A arquitetura de `docs/AUTONOMY.md` suporta dois modos: despacho manual e backlog automático. A fila é **desligada por padrão**.

## Como ativar após testes e orçamento
- Colocar `CF_PROXY_API_KEY` no Secrets do repositório.
- Ajustar Settings → Actions para permitir Actions criar PRs.
- Definir variável de repositório `AGENT_BACKLOG_ENABLED=true`. Isso permite a `Agent Backlog Scheduler` considerar uma task duas vezes ao dia (07:13 e 19:13 UTC) ou via workflow_dispatch.
- Opcionalmente definir `AGENT_REVIEW_LOOP_ENABLED=true` para reparos sucessivos baseados em **dois pareceres independentes e CI** (padrão 8 rodadas, teto de 30) até aprovação ou bloqueio.
- Aplicar branch protection e revisão humana antes de integrar qualquer PR. Não existe auto merge.

## Algoritmo de seleção
1. Buscar PRs abertos. Se algum `agent/*` estiver pendente, **não iniciar outra workorder**.
2. Buscar execuções recentes do `Agent Execute`. Se houver execução ativa ou se a mais recente falhou sem PR, **parar** para evitar custo repetido.
3. Construir conjunto de workorders concluídas a partir de PRs mergidos com título `agent: epic / wo-XX` ou `workorder: epic/wo-XX`, somado a `.agents/completed-workorders.json`.
4. Validar identidade de workorder, booleano `automation` e todas as dependências. `automation:false` nunca será despachada.
5. Selecionar o primeiro item elegível em ordem de diretório/ID; disparar **um** workflow. No sucesso o PR fica aberto para revisão humana. No failure, aplicar limite de tentativa.

**Limitador adicional:** GitHub Actions não garante agendamento em horário exato; forks e repositórios desativados podem suspender schedules. Workflows executam apenas depois do merge em `main`.

## Como atestar trabalho manual
Para etapas que demandem direitos, ambiente Windows, assets ou aprovação humana, criar PR com título `workorder: epic-id/wo-XX`, anexando relatório e revisão, ou registrar entrada manual em `.agents/completed-workorders.json` no formato:

```json
{
  "schema_version": 1,
  "completed": {
    "e00-rights-governance-v1/wo-01": {
      "reviewer": "pessoa-responsavel",
      "evidence_url": "https://github.com/brasalabs6/pw-web/pull/123"
    }
  }
}
```

A alteração dessa lista requer revisão de maintainer; declarar manualmente trabalho concluído sem evidência é proibido. A fila não substitui validação de licença.

## Dependências e escopo
Workorders são descritas em `.agents/epics/<epic>/workorders/<wo>.json` e validadas por `.agents/automation/catalog.py`. O scheduler usa `backlog_pick.py`. Código gerado só pode alterar pastas permitidas e nunca `.agents/`, `.github/`, `references/`, scripts de verificação protegidos ou arquivos originais. Mudanças do próprio sistema de agentes sempre via PR humano.

## Custos e proteção contra loop
Um despacho automático no máximo a cada horário do cron; um PR por vez; até três tentativas apenas se retry estiver habilitado; tempo máximo do job; API key com budget; falta de dependência bloqueia. Se o agente falhar antes de abrir PR, será necessário resolver manualmente o bloqueio.

## Observabilidade
- Actions → Agent Backlog Scheduler → seleção/decisão.
- Actions → Agent Execute → prompt, patch e output da execução (com retenção).
- Actions → Bootstrap CI → `report.json` + `verify.log`.
- Actions → Agent CI Feedback → comentário de resultado e reexecução condicional.
- Issues/PRs → evidência, discussão e aceite. Nada é observado continuamente por uma sessão do ChatGPT; os resultados permanecem no GitHub para consultas posteriores.

O loop de revisão DeepSeek + GLM Flash é documentado em [GROK_BUILD.md](GROK_BUILD.md).