# R0 — matemática e primeiro desenho WebGL2

Data: 2026-10-25. Base: `2eb44a3` (com alterações preexistentes de documentação
preservadas). Contrato: [render-r0.md](../../.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md).

Hipótese P1: uma fronteira com inteiros de tamanho fixo, matriz row-major e
conversão explícita de ARGB preserva as fixtures R0 em C++ nativo e wasm32,
rejeitando entradas inválidas antes do draw. Não é a ABI binária do cliente.

## Evidência atual

- RED: compilação de `tests/render_clip.cpp` falhou por ausência de `clip.h`,
  antes da implementação.
- GREEN nativo: 14 grupos de casos passaram com GCC 16.2.1, C++17,
  `-O2 -Wall -Wextra -Werror -pedantic`.
- AddressSanitizer + UndefinedBehaviorSanitizer: os mesmos 14 grupos passaram.
- `./scripts/verify.sh`: 18 testes Python na base local inicial passaram; após
  integração com `c7c74e1`, passaram 31 testes Python, smoke C++ e 14 grupos R0.
  Resultados locais, não CI do novo commit.
- WASM: PASS com Emscripten 6.0.13 e Node 24.20.0: os mesmos 14 grupos
  passaram via `./scripts/test-render-clip.sh` em código de `681c8b7`.
- P2: backend GLES3/WebGL2 adicionado por ChatGPT Web; build Emscripten e pixels/lifecycle em browser **NOT RUN** neste ambiente. Suite estrita de prova positiva/negativa adicionada, não substitui prova.

## Arquivos e reprodução

P1: `engine/render/clip.h`, `clip.cpp`, `tests/render_clip.cpp` e
`scripts/test-render-clip.sh`. O script exige compilador C++, Emscripten e Node;
ferramenta ausente produz erro, sem skip. Ele exige os 14 grupos em cada alvo e
compara suas saídas; cada execução verifica valores esperados da spec.

```bash
source /tmp/pw-web-emsdk/emsdk_env.sh
./scripts/test-render-clip.sh
./scripts/verify.sh
```

Validação nativa executada antes da disponibilidade de Emscripten:

```bash
c++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic -Iengine/render \
  engine/render/clip.cpp tests/render_clip.cpp -o /tmp/pw-render-clip
/tmp/pw-render-clip
c++ -std=c++17 -g -fsanitize=address,undefined -fno-omit-frame-pointer \
  -Wall -Wextra -Werror -pedantic -Iengine/render \
  engine/render/clip.cpp tests/render_clip.cpp -o /tmp/pw-render-clip-sanitized
/tmp/pw-render-clip-sanitized
```

Ambos imprimiram `PASS render-clip: 14 cases`. O script de ambos os alvos
foi executado sem Emscripten e rejeitou a pré-condição com exit 1. O build P2
também rejeitou a ausência de `em++`; esse resultado não conta como teste RED
do comportamento gráfico. A sintaxe do módulo JavaScript do harness passou em
`node --check`; `git diff --check` passou. Scripts shell novos foram verificados
pelo `verify.sh`. Nenhum desses checks substitui os testes de pixels pendentes.

Os casos cobrem identidade, profundidade, perspectiva, matriz não simétrica,
translação, canais ARGB, triângulo válido, ponteiros nulos, contagens inválidas,
índices fora dos limites, NaN/Infinity, overflow da transformação, viewport e
contagens máximas. Erros de transformação precisam preservar a saída anterior.
`VertexPC` verifica em compile time tamanho de 16 bytes e offsets 0/12.

P2: `tests/probes/render.html` e `scripts/build-render-probe.sh` preparam uma
prova opt-in, sem alterar a demo pública ou o build padrão. O teste exige
WebGL2, pixels interiores, ARGB, translação, profundidade em ambas as ordens,
erros específicos, perda/restauração de contexto e dispose idempotente.
O backend `engine/platform/web/render_probe.cpp` agora está implementado
como candidato, mas **não foi compilado com Emscripten nem executado no browser**
nesta sessão. A prova opt-in usa `scripts/probe-render-browser.sh` e
`tests/probes/render-browser-proof.cjs` com 10 grupos, 10 pixels obrigatórios,
controle negativo deliberado e manifest/screenshot local. Resultados PASS só
podem ser registrados após build e execução reais.

## Limites e próximos passos

P1 passou no nativo e em WASM/Node. P2 exige execução real no
browser; compilação não basta. Não houve cópia de código/assets de referência,
mudança de protocolo, atualização de submódulo ou impacto em outro repositório.
Não há evidência nova de compilação Windows, integração Angelica, login ou mundo.
O próximo gate é executar o build e a prova positiva/negativa P2 antes da
integração de um draw original e comparação com baseline Windows.

## Revalidação do componente original — 2026-10-25

Após provisionar o SDK oficial 6.0.13, `./scripts/probe-angelica.sh` recompilou
ARandomGen in-place do SHA fixado. WASM: 22.280 bytes. A página em localhost:8481
foi executada no Chrome headless 152, em contexto isolado via Chrome DevTools.
4.096 uniformes coincidiram exatamente e 256 gaussianos passaram na tolerância
1e-12; 1.922 triângulos sintéticos e pixel central [50,136,143,255].
Renderer: ANGLE/SwiftShader (software); nenhuma medida de GPU física.

Negativo: reload com wrapper de fetch que altera exclusivamente a primeira
amostra de native.json (+0.01); a página reportou FAIL com
`uniform: seed 0, sample 0 differs`. Reload sem wrapper voltou a PASS.
Não houve alteração das fontes ou fixtures em disco. Evidência local em
`artifacts/angelica-probe/browser-rerun-evidence.json` e manifest.json; sem
upload de binários/assets. Isso não comprova P2, render Angelica ou gameplay.

## Incremento ChatGPT Web — 2026-10-25 (PR #15)

Base lida: `25a409baf91986b644f76a209c0b7cf22c699577`.
Alterações de P2 e WEB-03 publicadas na mesma branch sem force-push:
backend GLES3 C++ (`engine/platform/web/render_probe.cpp`), captura de erros
de import/boot na página, validador estrito Node com 5 testes, runner de
Chrome/Playwright opt-in (localhost:8482), integração dos testes de evidência
no Bootstrap CI. Não foram alterados contratos internos P1 nem assets originais.

- **PASS CI** em SHA `f18301de131231274bb87a4c48e0c190789e1a76`:
  [Bootstrap CI](https://github.com/brasalabs6/pw-web/actions/runs/40229275399)
  com portable-core-and-bootstrap e chrome-rendering-proof; e
  [Agent Controller Smoke](https://github.com/brasalabs6/pw-web/actions/runs/40229275362)
  com controller-dry-run e grok-build-cli-install. Chrome CI é **demo independente**.
  Verificações após este SHA devem ser consultadas separadamente.
- **P1 código mantido**: os 14 grupos são rodados pelo verify no CI, mas
  `./scripts/test-render-clip.sh` nativo+WASM não foi reexecutado aqui (em++ ausente).
  Evidência WASM/Node anterior é atribuída ao Codex, não a esta execução.
- **P2 NOT RUN**: não há `em++`, headers Emscripten/GLES3 nem `playwright-core`
  disponíveis neste ambiente; Chromium isolado não basta para compilar módulo.
  Build/browser pixels, negativo, context loss/restore, mobile e GPU real são
  **BLOCKED**, mesmo que scripts/fixtures estejam escritos.
- **R1 SPEC ONLY**: propostas explícitas para capabilities, sampler/texture
  RGBA8 2x2 e framebuffer offscreen em `docs/contracts/`; 0 casos R1 executados.

Para Codex reproduzir no laboratório com Emscripten oficial ativo:

```bash
./scripts/test-render-clip.sh
./scripts/build-render-probe.sh
node --test tests/probes/render-proof-utils.test.cjs
CHROME_BIN=/caminho/chrome PW_BROWSER_MODULE_DIR=/caminho/pai-node_modules \\
  bash scripts/probe-render-browser.sh
./scripts/verify.sh
```

Exigir no manifesto local `artifacts/render-probe/browser-proof.json`:
status PASS global, 10 grupos positivos, 10 pixels checados, negativo FAIL
**pela divergência intencional do triângulo** (não por erro de import), e zero
erros JS. Inspecionar contexto/restauração e driver antes de afirmar prova.
Nada disso prova draw original Angelica, baseline Windows, protocolo ou gameplay.
