# PW Web — Perfect World no navegador

[![Bootstrap CI](https://github.com/brasalabs6/pw-web/actions/workflows/ci.yml/badge.svg)](https://github.com/brasalabs6/pw-web/actions/workflows/ci.yml)

Objetivo: construir um **cliente de MMORPG executado localmente no navegador**, com C++/WebAssembly, WebGL 2 e WebSocket. Não confundir com cloud gaming, streaming de vídeo ou emulação de Windows.

> **Estado: fundação e provas de viabilidade (não jogável).** A demo em `web/` usa geometria procedimental e NÃO contém Perfect World nem seus assets.

## Início rápido

```bash
git clone https://github.com/brasalabs6/pw-web.git
cd pw-web
./scripts/bootstrap.sh
./scripts/dev.sh
```

Abra http://localhost:8080 para conferir o diagnóstico do navegador e a prova gráfica WebGL 2. Não é necessária nenhuma dependência de terceiros para esse primeiro teste. Para compilar a prova de C++ → WASM, instale o [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) e execute `./scripts/build-wasm.sh`. A página deverá passar a informar que o núcleo WebAssembly foi carregado.

## Referências externas (opcionais e **não redistribuídas**)

Os diretórios em `references/` são submódulos fixados por commit. **Não** são usados pela demo, pelo build padrão ou pelo CI. Baixe apenas se você tiver a autorização pertinente:

```bash
git submodule update --init references/client-153
git submodule update --init references/client-152-modern
git submodule update --init references/server-rust
```

**Atenção jurídica:** a disponibilidade pública de código-fonte de Perfect World não demonstra direito de reprodução, criação de derivados ou distribuição comercial. Não publique código, assets ou marcas de terceiros sem avaliar os direitos aplicáveis. Veja [docs/LEGAL.md](docs/LEGAL.md).

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
