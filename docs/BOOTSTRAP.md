# Bootstrap técnico reproduzível

Este marco estabelece ferramentas mínimas sem código/asset original. Veja `README.md` e `AGENTS.md`.

## Comandos
```bash
bash scripts/bootstrap.sh         # checa dependências e executa suite
bash scripts/verify.sh            # unit tests + compilação C++ nativa quando disponível
bash scripts/dev.sh               # localhost:8080: WebGL 2 procedimental
bash scripts/build-wasm.sh        # exige emcc; gera JS + WASM ignorados pelo Git
```

Resultado: contexto WebGL 2 com triângulo rotativo e diagnóstico. Com emsdk, `pw_core.cpp` é compilado como WebAssembly e seu código C++ responde ao host JS. Isso não contém Perfect World.

## Preparando o Emscripten
Usar emsdk oficial: https://emscripten.org/docs/getting_started/downloads.html
Em ambiente POSIX após instalar/ativar `emsdk_env.sh`, verificar `emcc --version`.
O script gera arquivos em `web/wasm/` (não versionados).

## Não incluído
Bibliotecas DirectX, modelos, mapas, audio, servidor dedicado, login, multiplayer, inventário. Cada capacidade só vira status `done` após testes próprios.

## Após o bootstrap
Primeiro marco de desenvolvimento real: extrair/inventariar módulos portáveis da Angelica e compilar/testar um módulo REAL (de código com direitos esclarecidos) em dois targets: nativo e WASM. Priorizar testes de paridade.

## Gate de CI
O GitHub Actions não baixa os submódulos e compila apenas o core original criado para esta pesquisa; não testa perfect world. A compilação wasm deve ser executada manualmente com emsdk até criarmos job dedicado e com pin de toolchain.
