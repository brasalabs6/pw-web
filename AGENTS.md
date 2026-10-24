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
- **Não** copiar código ou assets proprietários de referências para `engine/`, `web/` ou a branch principal até haver uma decisão jurídica documentada. Acesso ao repositório não é autorização.
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

O executor no GitHub Actions é Grok Build, pela CLI oficial `grok -p` e segredo `XAI_API_KEY`. O GitHub Actions é o orquestrador; Grok é o agente de código, não um serviço de deploy ou merge automático. Consulte `docs/GROK_BUILD.md`, `docs/AUTONOMY.md` e `scripts/run-grok-workorder.sh`. Qualquer mudança de executor/versão exige PR e tests de integração.
