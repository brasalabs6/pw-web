# Grok Build — Cloudflare GLM 5.3 + dois agentes revisores

## Objetivo

O GitHub Actions usa o **Grok Build**, com oito modelos personalizados servidos pelo proxy Cloudflare da organização. O coder trabalha com `cf-glm-5.3` (GLM 5.3). Quando o PR do coder passa de draft a *ready for review*, o sistema aciona independentemente dois revisores:

| Papel | Agente | Modelo |
| --- | --- | --- |
| Implementação / reparo | `.grok/agents/coder.md` | `cf-glm-5.3` |
| Segurança/correção adversarial | `.grok/agents/reviewer-deepseek.md` | `cf-deepseek-v4-flash` |
| Arquitetura/testes/funcionalidade | `.grok/agents/reviewer-glm-flash.md` | `cf-glm-5.3-flash` |

Os oito modelos com seus IDs, contexto, parâmetros e nomes estão em [`.grok/models.ci.toml`](../.grok/models.ci.toml); seu segredo não está nesse arquivo. Os modelos `cf-glm-5.2`, `cf-glm-5.3-cyber`, `cf-glm-5.2-cyber`, `cf-glm-5.3-flash-cyber` e `cf-qwen3.8-27b` também ficam disponíveis para futuras workorders, sem seleção automática silenciosa.

## Importante: onde o Grok carrega modelos

O Grok Build só lê `[models]` e `[model.*]` do **arquivo de usuário** `$GROK_HOME/config.toml` (por padrão `~/.grok/config.toml`). O arquivo `.grok/config.toml` do projeto aceita apenas configurações restritas de MCP/plugins/permissões. Por isso `scripts/configure-grok-models.sh` copia o catálogo sem segredo para um diretório temporário isolado em cada runner. Use `grok inspect` para diagnosticar.

Para usar esses modelos **localmente** com seu login do Grok, incorpore as seções de `.grok/models.ci.toml` ao seu `~/.grok/config.toml`, mantendo configurações locais pessoais e substituindo `api_key = "cfp_key_aqui"` pelo mecanismo seguro `env_key = "CF_PROXY_API_KEY"`. Defina essa variável no seu sistema sem commitar a chave. Não sobrescrevemos seu arquivo pessoal automaticamente.

## Como habilitar no GitHub

1. Crie um secret `CF_PROXY_API_KEY` em **Settings → Secrets and variables → Actions → Secrets**, com o valor REAL de autenticação do proxy Cloudflare. O placeholder `cfp_key_aqui` da configuração de exemplo **não é uma credencial funcional**.
2. Autorize GitHub Actions a criar PRs e executar `workflow_dispatch` com permissões mínimas (`contents/pull-requests/actions` nos jobs autorizados).
3. Configure branch protection/ruleset da `main`: checks **Bootstrap CI** e **Agent Controller Smoke**, revisão humana obrigatória, restrições para arquivos de workflow e código de controle.
4. Opcional: variável `AGENT_REVIEW_LOOP_ENABLED=true` para **habilitar reparos automáticos após a dupla revisão**. Ausente/false mantém comentários mas deixa o PR para triagem humana.
5. Opcional: `AGENT_MAX_REVIEW_ROUNDS=8` (default). Pode aumentar até 30, após revisar orçamento/custos; cada rodada utiliza coder + dois revisores + CI. Ao atingir o limite o pipeline para e exige intervenção humana.
6. `AGENT_BACKLOG_ENABLED=true` é separado: permite escolher novas tasks após integração; não é necessário para revisar uma task já iniciada.

## Fluxo

```text
Workorder .agents/epics/.../workorders/wo-XX.json
           |
      coder GLM-5.3 (grok -p, profile coder)
           |
    patch validado sem segredos + draft PR
           |
       PR ready for review
           |
      CI ──┬── DeepSeek reviewer (parallel / read-only)
           └── GLM Flash reviewer (parallel / read-only)
                        |
               findings JSON independentes
                        |
             controlador valida resultado
                        |
             comentário GitHub no PR
               /                   \
    ambos aprovam + CI verde     problemas reais/CI falha
               |                   |
        revisão HUMANA     reparar com coder GLM-5.3
                                   |
                           novo commit/PR round N+1
                                   |
                           repetir (máximo controlado)
```

A alteração de draft para ready via `GITHUB_TOKEN` pode não desencadear evento normal. Por isso o publisher chama explicitamente `Agent Dual Review` via `workflow_dispatch` com número do PR, SHA e rodada. Uma transição **manual** para ready também é suportada via `pull_request.ready_for_review`. Os revisores não executam código de PR com permissões privilegiadas; produzem artefatos para o job de comentários/retrabalho.

## Privacidade, segurança e critérios de aceite

- Nunca escrever chave em `.grok/`, `.env`, PR, logs, issue, saída de modelo ou artefatos. A CLI usa `env_key`.
- Cada revisor opera em job independente de somente leitura com seu modelo; não recebe token GitHub de escrita. A chave de proxy está apenas no step de inferência.
- O controlador só aceita PR aberto, dentro do mesmo repositório, branch `agent/<epic>/<wo>`, `head_sha` exato e commit com `[agent-attempt:N]`.
- O relatório JSON exige `decision`, `summary`, `findings`, `severity`, `path`, `title`, `detail` e `recommendation`; findings apontam apenas para arquivos realmente alterados. Resultado ilegível/ausente bloqueia automaticamente.
- Relatórios por revisor, diff, status CI e o comentário do PR permanecem rastreáveis por SHA e rodada. Resultado duplicado/stale não deve provocar nova execução.
- Os revisores são **consultivos**: `approved` em JSON não significa aprovação do GitHub. **Merge continua sempre humano**, com gates jurídicos de direitos e licenças.
- O loop é finito por execução para controlar custos e corrigir falhas repetitivas. Para mais rodadas, mantenedores devem revisar as evidências e aprovar a ampliação do limite.
- Esses checks **não provam que Perfect World é jogável** nem validam licenças dos fontes e assets originais.

## Testes sem acesso à API

`Bootstrap CI` valida TOML, os agentes e o parser de findings; `Agent Controller Smoke` instala a CLI sem chave. O teste com *mock* verifica seleção do modelo, profile, sandbox e proxy secret. **O ciclo de inferência real só pode ser considerado comprovado depois de um piloto com `CF_PROXY_API_KEY` e PR de uma workorder de baixo risco.**

## Referências técnicas

- https://docs.x.ai/build/settings
- https://docs.x.ai/build/overview
- https://github.com/xai-org/grok-build/blob/main/crates/codegen/xai-grok-pager/docs/user-guide/11-custom-models.md
- https://github.com/xai-org/grok-build/blob/main/crates/codegen/xai-grok-agent/README.md

**Prompts longos:** o runner usa `grok --prompt-file` em vez de `-p "$(cat ...)"`, evitando o limite de tamanho dos argumentos do sistema operacional e a exposição do texto completo no comando do processo. O diff de revisão tem limite de 240 KB; PR maior exige segmentação/revisão humana.

## JSON estruturado e provas de navegador
Cada agente possui um schema versionado em `.grok/schemas/*.schema.json` e o runner invoca `grok --output-format json --json-schema <arquivo>`. O controlador usa exclusivamente `structuredOutput`, exige `stopReason=end_turn` e valida status final + contrato + findings. Texto livre nunca é interpretado como JSON confiável. Chrome DevTools MCP está em `.grok/config.toml` com wrapper em `scripts/chrome-mcp-launch.sh`; veja [BROWSER_TESTING.md](BROWSER_TESTING.md). A CI contém um teste independente no Chrome com screenshots e relatório por viewport.
