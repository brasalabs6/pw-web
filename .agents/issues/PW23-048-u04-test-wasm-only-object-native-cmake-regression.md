---
id: PW23-048
title: "U04 test references WASM-only OBJECT target during native CMake generation"
priority: P1
status: resolved
kind: reproduced-ci-regression
first_observed: 2026-10-30
verified_against: 69a670f1c9f9a83803372ee90743893ceb4f1f43
resolved_at: 2026-10-30
resolved_by: b048b842c3c66bf08440dc6a9264de51e12df9d9
epic: e16-qa-observability-v1
scope: pr-23
---

# PW23-048 — U04 CTest breaks native CMake generation

## Evidência reproduzida

- **PR / branch / SHA:** [PR #23](https://github.com/brasalabs6/pw-web/pull/23), `feat/client153-common-containers`, [`e8cd605bd093558c28d1f11e309c6e544580a226`](https://github.com/brasalabs6/pw-web/commit/e8cd605bd093558c28d1f11e309c6e544580a226).
- **CI oficial:** [run 41045371229 / job 124028044162](https://github.com/brasalabs6/pw-web/actions/runs/41045371229/job/124028044162), 2026-10-30 11:05 UTC, conclusão **failure**.
- **Comando:** etapa `Native original-layout and bounded package tests`, `cmake -S . -B build/client153-native -DCMAKE_BUILD_TYPE=Debug` (CMake native); falha na geração, exit 1.
- **Erro literal:** `CMake Error at CMakeLists.txt:218 (target_link_libraries): Error evaluating generator expression: $<TARGET_OBJECTS:client153_engine_original>; Objects of target "client153_engine_original" referenced but no such target exists.`
- **Causa:** [`CMakeLists.txt:214–225`](https://github.com/brasalabs6/pw-web/blob/e8cd605bd093558c28d1f11e309c6e544580a226/CMakeLists.txt#L214-L225) cria o teste sob `BUILD_TESTING` e injeta `$<TARGET_OBJECTS:client153_engine_original>`, mas [`src/client153/CMakeLists.txt:113–115`](https://github.com/brasalabs6/pw-web/blob/e8cd605bd093558c28d1f11e309c6e544580a226/src/client153/CMakeLists.txt#L113-L115) só define esse target em `if(EMSCRIPTEN)`. A opção `-sSTACK_SIZE=1048576` no mesmo bloco também é específica de Emscripten e precisa de guarda para o build nativo.
- **Impacto observado:** native build/CTest não executados; etapa WebGL2/WASM marcada `skipped`. Não é prova de crash do jogo no browser.

## Correção proposta

Separar as dependências nativas da variante WASM do teste U04. Para a variante nativa, linkar apenas os objetos necessários do parser e bibliotecas nativas existentes, sem depender de `client153_engine_original`; guardar flags `-s*` e alvos exclusivos de Emscripten. Preservar o teste de parser e adicionar, quando possível, fixtures que chamem `CECInstance::Load` de fato, em vez de apenas validar `width < 10` no próprio teste.

## Critérios de aceite

1. Checkout limpo: CMake native configura, compila e executa o novo CTest U04; CTest nativo completo passa.
2. Checkout limpo: CMake Emscripten configura, compila/linka o browser e executa CTest WASM, incluindo U04; sem skips induzidos por falha nativa.
3. Registrar SHA, comandos, logs e contagens; testes de parsing não são prova de renderização, login ou gameplay.
4. Para declarar U04 funcionalmente encerrado, fixtures válidas e inválidas devem exercitar o loader real `CECInstance::Load` e comprovar sucesso/rejeição corretos.

## Revalidação parcial — 2026-10-30 (SHA `b048b842c3c66bf08440dc6a9264de51e12df9d9`)

- [`CMakeLists.txt:214–248`](https://github.com/brasalabs6/pw-web/blob/b048b842c3c66bf08440dc6a9264de51e12df9d9/CMakeLists.txt#L214-L248) agora compila `AWScriptFile.cpp` diretamente no teste U04, sem `$<TARGET_OBJECTS:client153_engine_original>`; a opção `-sSTACK_SIZE` está protegida por `if(EMSCRIPTEN)`.
- [CI run 41065095748 / job 124094150575](https://github.com/brasalabs6/pw-web/actions/runs/41065095748/job/124094150575): CMake nativo configurou/gerou, teste `client153_u04_instance_parse` compilou e passou no conjunto **36/36 CTest nativos**. Python discovery falhou depois por **PW23-049**; etapa WASM foi **skipped**, logo o gate completo ainda não foi revalidado.
- O teste U04 ainda exercita o parser `AWScriptFile` e a condição `width < 10`, mas **não invoca `CECInstance::Load`**. Manter o critério de validação funcional do loader.
- **Estado: in_progress**, não `resolved`.

## Deduplicação

- [Review #5711554959](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5711554959) registra SHA, linhas e correção.
- PW23-047 é o warning de `window.cpp` reproduzido no SHA anterior; este erro CMake novo é independente.
- PW23-042 cobre o skip FTDriver, não a falha deste target U04.
- A revisão U04 anterior tratou da divergência `continue` vs `return false`; o novo SHA restaura os `return false` por inspeção, mas ainda não valida o loader completo.
- Preservar os IDs migrados PW23-021…037 em optimization; nenhum tuning de FPS/RAM é parte desta issue.

## Resolução da regressão CMake — 2026-10-30, CI do HEAD `69a670f1`

- [`b048b842`](https://github.com/brasalabs6/pw-web/commit/b048b842c3c66bf08440dc6a9264de51e12df9d9) separou o teste U04 das dependências OBJECT exclusivas de Emscripten e protegeu a flag `-sSTACK_SIZE`.
- [CI run 41098247196, job 124209512436](https://github.com/brasalabs6/pw-web/actions/runs/41098247196/job/124209512436), SHA `69a670f1c9f9a83803372ee90743893ceb4f1f43`: configuração e geração CMake nativa completas, `client153_u04_instance_parse_test` compilado e CTest `client153_u04_instance_parse` **24/37 PASS**; CTest nativo **37/37 PASS**. A configuração/build Emscripten também concluiu, o teste U04 **27/41 PASS** e CTest WASM **41/41 PASS**. O target `client153_browser_entry` linkou.
- **Escopo do fechamento:** a regressão de configuração e execução do teste U04 em duas toolchains foi resolvida. O teste ainda exercita `AWScriptFile` e a condição de dimensões, **não** o `CECInstance::Load` real; a semântica do loader com assets originais e o boot da cena permanecem não comprovados (ver [review U04](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5708843221), [PW23-041](PW23-041-login-ecwld-scene-load-bypass.md) e [PW23-013](PW23-013-test-gates-not-browser-game.md)). Não declarar U04 funcionalmente encerrado com base somente neste CI.
