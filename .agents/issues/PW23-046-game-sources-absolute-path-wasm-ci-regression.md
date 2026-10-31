# PW23-046 — Manifest de fontes do cliente completo usa caminhos absolutos e quebra CI WASM

- **Prioridade:** P0 — blocker reproduzido do gate de configuração WASM
- **Estado:** in_progress — regressão do CI corrigida no HEAD 198ff691; resta comprovar regeneração em duas raízes e política de TUs ausentes
- **Primeiro SHA observado:** `4ebdbcda01f8bec8181bf786a2f9c2df1d44d74d` (2026-10-30)
- **Área:** e04-wasm-runtime-v1 / PW23-008 (fechamento de TUs e link)
- **Tipo:** regressão de build reproduzida em CI, não falha de gameplay reproduzida

## Evidência

- [`src/client153/game-sources.cmake:5-12`](https://github.com/brasalabs6/pw-web/blob/4ebdbcda01f8bec8181bf786a2f9c2df1d44d74d/src/client153/game-sources.cmake#L5-L12) contém caminhos absolutos de uma workstation `/home/nonexphere/workspace/pw/pw-browser-port/...`.
- [`src/client153/CMakeLists.txt:628-630`](https://github.com/brasalabs6/pw-web/blob/4ebdbcda01f8bec8181bf786a2f9c2df1d44d74d/src/client153/CMakeLists.txt#L628-L630) inclui esse fragmento ao configurar EMSCRIPTEN, antes de qualquer compilação.
- [`scripts/emit-client153-game-sources.py:287-314`](https://github.com/brasalabs6/pw-web/blob/4ebdbcda01f8bec8181bf786a2f9c2df1d44d74d/scripts/emit-client153-game-sources.py#L287-L314) grava os caminhos resolvidos e inclui diretórios absolutos.
- [GitHub Actions run 40996399471 / job 123870421557](https://github.com/brasalabs6/pw-web/actions/runs/40996399471/job/123870421557), 2026-10-30 03:17 UTC: após 34/34 CTest nativos aprovados, `emcmake cmake -S . -B build/client153-wasm` falha com `Cannot find source file: /home/nonexphere/workspace/pw/pw-browser-port/src/client153/Element/CElementClient/DlgAskHelpToGM2.cpp`, depois `No SOURCES given to target: client153_game_full`, exit 1. Os testes WASM e a validação de artefatos não chegam a executar.
- [Engineering Review #5707734780](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5707734780).

## Revalidação incremental — 2026-10-30 (HEAD `198ff6914d290f9bfe28a971ecdf6bebd4d722ec`)

- O [manifesto no HEAD](https://github.com/brasalabs6/pw-web/blob/198ff6914d290f9bfe28a971ecdf6bebd4d722ec/src/client153/game-sources.cmake#L5-L12) agora usa `${PW153_REPO_ROOT}` em vez de `/home/nonexphere/...`. O [gerador](https://github.com/brasalabs6/pw-web/blob/198ff6914d290f9bfe28a971ecdf6bebd4d722ec/scripts/emit-client153-game-sources.py#L213-L232) emite caminhos relativos a `${PW153_REPO_ROOT}` ou `${PW153_REFERENCE_ROOT}`; o [CMake](https://github.com/brasalabs6/pw-web/blob/198ff6914d290f9bfe28a971ecdf6bebd4d722ec/src/client153/CMakeLists.txt#L620-L635) define as raízes.
- **Proven no CI:** [run 41005718247 / job 123900046024](https://github.com/brasalabs6/pw-web/actions/runs/41005718247/job/123900046024), 2026-10-30: checkout limpo configura, compila 943 objetos do target `client153_game_full` e registra `Built target client153_game_full`, `Linking CXX executable client153_browser_entry.js` e `Built target client153_browser_entry`. CTest nativo **34/34** e WASM **37/37**; artifacts do viewer validados. O erro CMake original **não se reproduziu** nesse runner.
- **Ainda não provado:** geração determinística do manifesto em duas raízes distintas; o gerador ainda acumula `missing` e faz `skip` em TUs sem falhar ([trecho](https://github.com/brasalabs6/pw-web/blob/198ff6914d290f9bfe28a971ecdf6bebd4d722ec/scripts/emit-client153-game-sources.py#L281-L304)). Verificar cobertura completa dos projetos e exclusões deliberadas antes de fechar PW23-008/046.
- **Não é prova de gameplay:** o próprio [workflow](https://github.com/brasalabs6/pw-web/blob/198ff6914d290f9bfe28a971ecdf6bebd4d722ec/.github/workflows/client153-port.yml#L57-L60) declara que não executa assets originais, login, realm ou gameplay. Python unittest ainda mostra `OK (skipped=1)` (PW23-042).

## Impacto

O target de cliente completo não configura em checkout limpo do runner, mesmo quando os arquivos do projeto existem sob outra raiz. A regressão torna o CI obrigatório vermelho e bloqueia a comprovação do link WASM completo. **Não prova falha de login, renderização ou jogo no navegador.**

## Correção proposta

Emitir paths relativos ao checkout (resolvidos em CMake a partir de `CMAKE_CURRENT_SOURCE_DIR`) e usar variável explícita para fontes da referência pinada. Preferir regenerar o fragmento durante a configuração com entradas declaradas, sem paths da workstation. Tornar ausências de TUs obrigatórias erro do gerador e manter allowlist para exclusões intencionais.

## Critérios de validação

1. `emcmake cmake -S . -B build/client153-wasm` passa em checkout limpo no runner e em outro diretório local.
2. `cmake --build build/client153-wasm --target client153_game_full --parallel 2` realmente compila os TUs listados; não mascarar falha de link como sucesso.
3. CTest WASM e gates de artefatos browser voltam a executar e passar no SHA da correção.
4. A geração em duas raízes distintas produz a mesma lista relativa e não contém paths `/home/nonexphere/...`.
5. Registrar SHA, job, resultado e evidência na resolução. Não classificar como gameplay comprovado.
