---
id: PW23-049
title: "Manifest regeneration gate differs for nested vs external pinned reference checkout"
priority: P1
status: open
kind: reproduced-ci-regression
first_observed: 2026-10-30
verified_against: b048b842c3c66bf08440dc6a9264de51e12df9d9
epic: e16-qa-observability-v1
scope: pr-23
---

# PW23-049 — Regeneração do manifesto não é determinística entre raízes; CI WASM bloqueado

## Evidência reproduzida

- **PR / branch / SHA:** [PR #23](https://github.com/brasalabs6/pw-web/pull/23), `feat/client153-common-containers`, [`b048b842c3c66bf08440dc6a9264de51e12df9d9`](https://github.com/brasalabs6/pw-web/commit/b048b842c3c66bf08440dc6a9264de51e12df9d9).
- **CI oficial:** [run 41065095748 / job 124094150575](https://github.com/brasalabs6/pw-web/actions/runs/41065095748/job/124094150575), 2026-10-30 13:41 UTC, conclusão **failure**.
- **Comando:** `python3 -m unittest discover -s tests/client153 -p 'test_*.py'` após CMake/compilação/CTest nativos.
- **Erro literal:** `FAIL: test_committed_fragment_matches_regeneration` em `tests/client153/test_game_sources_emitter.py:171`; `AssertionError: ... PW153_REPO_ROOT}/references/client-153/... != ... PW153_REFERENCE_ROOT}/...`; `Ran 41 tests`, `FAILED (failures=1, skipped=1)`, exit 1.
- **Progresso no mesmo job:** CTest nativo **36/36 passou**, assets unittest **7/7 passou**. A etapa **Build WebGL2 runtime and execute WASM parser regressions** foi marcada **skipped**, portanto não há resultado WASM novo neste SHA.
- **Causa:** [`emit-client153-game-sources.py:213–227`](https://github.com/brasalabs6/pw-web/blob/b048b842c3c66bf08440dc6a9264de51e12df9d9/scripts/emit-client153-game-sources.py#L213-L227) verifica se o caminho está dentro de `repo_root` antes de verificar se está dentro da referência. Como o workflow materializa o submódulo em `references/client-153` dentro do repo, fontes da referência são emitidas com `${PW153_REPO_ROOT}/references/client-153/...`. O manifesto versionado usa `${PW153_REFERENCE_ROOT}/...`, gerado com a referência externa. O [novo teste de igualdade byte a byte](https://github.com/brasalabs6/pw-web/blob/b048b842c3c66bf08440dc6a9264de51e12df9d9/tests/client153/test_game_sources_emitter.py#L153-L171) expõe a diferença. Não é erro de gameplay reproduzido.

## Impacto

A representação do manifesto depende da topologia do checkout, não apenas da revisão pinada e dos arquivos. O novo gate de regressão fica vermelho em checkout limpo, interrompendo o pipeline antes do build/link e CTest WASM.

## Correção proposta

1. Na função `portable`, priorizar a raiz explícita `PW153_REFERENCE_ROOT` para arquivos da referência; usar `PW153_REPO_ROOT` para os arquivos adaptados e demais arquivos do repo, sem alterar a precedência das cópias adaptadas no conjunto de fontes.
2. Testar referência pinada **interna** (`references/client-153`) e **externa** ao checkout; exigir fragmento idêntico ao versionado nas duas topologias.
3. Preservar testes que falham com TUs obrigatórias ausentes e exclusões obsoletas; não desabilitar a asserção de determinismo.

## Critérios de aceite

- Regeneração com as duas raízes resulta em bytes idênticos e não contém caminhos absolutos de workstation.
- `cmake -S . -B build/client153-native -DCMAKE_BUILD_TYPE=Debug`, `cmake --build build/client153-native --parallel 2`, `ctest --test-dir build/client153-native --output-on-failure` e Python discovery passam no checkout limpo; `CommittedFragment` realmente executa (sem skip).
- `emcmake cmake -S . -B build/client153-wasm -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug`, build/link do browser e CTest WASM executam e passam no **mesmo SHA**.
- Registrar SHA, job e logs. Nenhum destes gates substitui teste de pixels, login ou gameplay.

## Deduplicação

- [Engineering Review #5713199343](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5713199343).
- PW23-046 cobre o incidente anterior de caminhos absolutos e o trabalho residual de regeneração em duas raízes; **esta issue registra a regressão nova, reproduzida no CI, provocada pelo novo teste de igualdade**. Não duplicar a causa histórica.
- PW23-048 é a falha CMake do teste U04, agora superada na configuração/CTest nativos deste SHA; PW23-042 continua cobrindo o skip FTDriver.
- Sem tuning de performance; preservar PW23-021…037 na pasta de otimização.
