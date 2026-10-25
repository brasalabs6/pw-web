# AGENTS — instruções para humanos e agentes de código

## Missão
Produzir o cliente do Perfect World executado no navegador, **não** um clone visual sem protocolo. O trabalho atual é provar viabilidade técnica por etapas independentes. A demo WebGL 2 é apenas uma prova de ambiente, não gameplay real.

## Ordem obrigatória de leitura
1. `README.md`
2. `docs/ONBOARDING.md`
3. `docs/INITIATION.md`
4. `docs/ARCHITECTURE.md`
5. `docs/ROADMAP.md`
6. `docs/VALIDATION.md`, `docs/LEGAL.md` e `docs/RESEARCH.md`

## Regras estritas
- Copiar código ou assets proprietários de referências para `engine/`, `web/` ou a branch principal é permitido **somente no escopo da decisão jurídica documentada** em `docs/LEGAL.md` (2026-10-25), com registro de proveniência e preservação de headers de copyright. Acesso ao repositório, por si só, nunca é autorização.
- Não rodar `git submodule update --remote`; usar commits fixados e registrar revisão em `docs/RESEARCH.md`.
- Não presumir que a compilação Windows 1.5.3 funciona nem que a versão 1.5.3 autentica no servidor Rust.
- Nunca afirmar que existe um port jogável sem uma prova com entrada no mundo e troca de pacotes.
- Todo trabalho deve indicar: hipótese, evidência, código alterado, comandos para reproduzir, resultado observado, riscos e próximos passos.
- Manter interfaces de plataforma fora do núcleo C++. No browser, rede via WebSocket seguro, render por WebGL 2 (WebGPU candidato futuro), assets com consentimento e streaming/cache.
- Um milestone por PR; abrir ADR quando decidir mudar versão-alvo, transporte de rede, backend gráfico ou propriedade dos assets.
- Não expor credenciais, IPs internos, tokens nem dados do cliente original.
- README/docs atualizados com status factual, especialmente quando um teste falha.

## Definition of Done
- Testes do escopo passam.
- Instruções de reprodução documentadas.
- Diferenças e limitações declaradas com precisão.
- Nenhum arquivo não autorizado ou grande asset adicionado.
- Critérios de aceite de `docs/VALIDATION.md` preenchidos.

## Comandos
```bash
./scripts/bootstrap.sh
./scripts/verify.sh
./scripts/dev.sh
# opcional, exige emcc:
./scripts/build-wasm.sh
```

## Áreas
- `engine/core`: C++ independente de Win32.
- `web`: host WebGL2/JS e bridge WASM.
- `references`: upstreams congelados, não compilados em CI.
- `tests`: testes isolados sem assets proprietários.
- `docs`: documentação, evidências e decisões.

## Prioridade operacional
P0: comprovar bootstrap, compilar WASM trivial e selecionar cliente/engine Windows funcional.
P1: modelo real legalmente autorizado + pipeline gráfico.
P2: gateway/protocolo e entrada no mundo.
P3: gameplay completo e otimizações.

## Executor de IA: Grok Build

O executor no GitHub Actions é Grok Build, pela CLI oficial `grok -p` e segredo `CF_PROXY_API_KEY` para nosso proxy Cloudflare. O GitHub Actions é o orquestrador; Grok é o agente de código, não um serviço de deploy ou merge automático. Consulte `docs/GROK_BUILD.md`, `docs/AUTONOMY.md` e `scripts/run-grok-workorder.sh`. Qualquer mudança de executor/versão exige PR e tests de integração.

Os agentes de sistema ficam em `.grok/agents/{coder,reviewer-deepseek,reviewer-glm-flash}.md` e o catálogo em `.grok/models.ci.toml` (usado via `GROK_HOME`). Reviews são consultivas e a aprovação final é humana.

## Padrão obrigatório de pull request
**Antes de abrir, atualizar ou solicitar review de um PR, consulte `.agent/skills/create-pr/SKILL.md` e siga o template `.github/PULL_REQUEST_TEMPLATE.md`.** Para commits produzidos por agente, o publisher privilegiado abre e solicita o PR; o coder não executa `gh pr` diretamente. Não marcar provas locais como testes CI. Branches de pesquisa humana não dispararam automaticamente os dois revisores no workflow restrito a `agent/*`.

## Portas da pesquisa
**8480** = host PW Web + Chrome/DevTools (variável `PW_WEB_PORT`, padrão 8480). **8481** = POC Angelica opt-in. A porta 8080 é reservada para outro software e não deve ser usada pelo projeto.
