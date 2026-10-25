# PW Web — Perfect World no navegador

[![Bootstrap CI](https://github.com/brasalabs6/pw-web/actions/workflows/ci.yml/badge.svg)](https://github.com/brasalabs6/pw-web/actions/workflows/ci.yml)

Objetivo: construir um **cliente de MMORPG executado localmente no navegador**, com C++/WebAssembly, WebGL 2 e WebSocket. Não confundir com cloud gaming, streaming de vídeo ou emulação de Windows.

> **Estado: fundação e provas de viabilidade (não jogável).** A demo em `web/` usa geometria procedimental e NÃO contém Perfect World nem seus assets.

**Primeira prova executada:** um componente original da Angelica (`ARandomGen`)
foi compilado sem alterações para WASM e validado no navegador contra a execução
nativa: 4.096 valores uniformes idênticos e 256 gaussianos dentro da tolerância.
Uma malha procedural usa essa saída em WebGL 2. Isso ainda não é o renderer da
Angelica nem o cliente completo. Veja [resultado e reprodução local](docs/POC-ANGELICA.md)
e o [método de implementação para Grok/GLM](docs/ANGELICA-PORTING-METHOD.md).

A fronteira matemática R0 já passa nos mesmos 14 grupos em C++ nativo e WASM/Node,
incluindo validação de dados e overflow. O backend gráfico ainda está pendente; veja [evidência e reprodução](docs/evidence/render-r0.md).

Execução assistida: [tarefas para o ChatGPT Web e validação pelo Codex](docs/CHATGPT-WEB-HANDOFF.md).

## Início rápido

```bash
git clone https://github.com/brasalabs6/pw-web.git
cd pw-web
./scripts/bootstrap.sh
./scripts/dev.sh
```

Abra http://localhost:8480 para conferir o diagnóstico do navegador e a prova gráfica WebGL 2. Não é necessária nenhuma dependência de terceiros para esse primeiro teste. Para compilar a prova de C++ → WASM, instale o [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) e execute `./scripts/build-wasm.sh`. A página deverá passar a informar que o núcleo WebAssembly foi carregado.

## Referências externas (opcionais)

Os diretórios em `references/` são submódulos fixados por commit. **Não** são usados pela demo, pelo build padrão ou pelo CI. O uso é permitido no escopo da autorização documentada em [docs/LEGAL.md](docs/LEGAL.md) (2026-10-25):

```bash
git submodule update --init references/client-153
git submodule update --init references/client-152-modern
git submodule update --init references/server-rust
```

**Atenção jurídica:** a disponibilidade pública de código-fonte de Perfect World não demonstra, por si só, direito de reprodução ou distribuição. O uso neste projeto está autorizado no escopo da decisão documentada em [docs/LEGAL.md](docs/LEGAL.md) (2026-10-25); fora desse escopo, não publique código, assets ou marcas de terceiros.

## Guia do projeto

| Documento | Conteúdo |
| --- | --- |
| [Onboarding](docs/ONBOARDING.md) | instalação, primeira contribuição, troubleshooting e regras de trabalho |
| [Plano de iniciação](docs/INITIATION.md) | como sair do zero, responsáveis, entregáveis, critérios de aceite |
| [Arquitetura](docs/ARCHITECTURE.md) | componentes e fronteiras, renderização, rede e assets |
| [Roadmap](docs/ROADMAP.md) | etapas, milestones, riscos e go/no-go |
| [Validação](docs/VALIDATION.md) | testes objetivos para escolher versão e engine |
| [Pesquisa e upstreams](docs/RESEARCH.md) | fontes, commits fixados, hipóteses e evidências |
| [Política de terceiros](docs/LEGAL.md) | direitos, proveniência, autorização e distribuição |
| [Guia para agentes](AGENTS.md) | regras de implementação e entregas |

## Filosofia

1. Evidência antes de promessa: execução e testes reproduzíveis.
2. Preferir preservar semântica do cliente, mas adaptar a plataforma de forma limpa.
3. Isolar interfaces Win32/Direct3D/TCP e formatos de dados em módulos testáveis.
4. Não misturar versões de cliente, protocolo e assets sem prova de compatibilidade.
5. Substituir código dependente de licença incerta por implementações originais/permitidas, se necessário.

Propriedade do scaffold de pesquisa: projeto brasalabs6. **Não há licença open-source global concedida neste repositório**; direitos de contribuições e referências são tratados separadamente.

## Agentes e programação de trabalho

- [Programa de 20 épicos (135 workorders)](.agents/README.md): requisitos, arquitetura, specs, testes, riscos e `tasks.md` por etapa.
- [Autonomia assistida por GitHub Actions](docs/AUTONOMY.md): Grok Build → patch isolado → PR → CI → evidências → reparo via dupla revisão, com limite configurável.
- [Scheduler do backlog (opt-in)](docs/AGENT_BACKLOG.md): dispara uma workorder elegível sem PR ativo, até duas vezes ao dia quando habilitado.
- **Prova sem credenciais:** Actions → **Agent Execute** → epic `e01-agent-orchestration-v1`, workorder `wo-01`, attempt `1` e `dry_run=true`. Não usa xAI API, não grava código e não cria PR.
- **Execução real:** requer secret `CF_PROXY_API_KEY` e autorização do GitHub Actions para criar PR. Default seguro: sem auto-merge e loop opcional desligado.

Os relatórios permanecem nos comentários de PR e artefatos de Actions; não se deve prometer que esta conversa observará novos eventos continuamente.

### Executor oficial: Grok Build

A automação usa `@xai-official/grok@1.0.70` em modo **headless** (`grok -p`) e `CF_PROXY_API_KEY` como segredo do proxy Cloudflare. Seu login local do Grok Build não é transportado para runners hospedados do GitHub. Consulte [configuração e segurança](docs/AUTONOMY.md) e [como executar localmente](docs/GROK_BUILD.md). `dry_run=true` continua disponível sem credenciais.

### Revisão automática dupla

O executor padrão é **CF GLM 5.3**. Cada PR gerado pelo coder dispara em paralelo **DeepSeek V4 Flash (segurança)** e **GLM 5.3 Flash (arquitetura/QA)**; os findings vão para os comentários do PR e, quando o loop está habilitado, provocam nova implementação. Configuração, limites e evidências: [Grok Build + revisores](docs/GROK_BUILD.md).
