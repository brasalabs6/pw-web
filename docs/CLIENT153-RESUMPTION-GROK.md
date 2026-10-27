# Retomada Grok — onde eu estava quando o Codex assumiu (2026-10-27)

Documento de revalidação: quando eu retomar (quota do Codex esgotada ou
sessão dele parada), este arquivo é o ponto de partida para revalidar TODO o
estado antes de continuar. Não commitado — o Codex está ativo; eu o commito
no meu primeiro commit de retomada.

## Estado no momento da transferência (meu head: `5581b016`, pushed)

- Campanha compile-first: **572/787 TUs do jogo compilando para objetos
  WASM (72,7%)** (probe `objects-cycle39.json`).
- Regressões do port: nativo 29/29 a cada milestone (WASM/ASan verdes no
  mesmo ciclo).
- Achado crítico já provado: o **trial-link** dos ~526 objetos (na época)
  com as libs WASM do port deu apenas **12 símbolos indefinidos, todos do
  jogo/AUI, zero de plataforma** — a ligação do cliente é viável com os
  seams existentes. Reproduzir:
  ```
  em++ -o /tmp/trial.mjs build/client153-original-objects/*.o \
    build/client153-wasm-tests/src/client153/libclient153_{skeleton,animation,file,common,platform,math,formats}.a \
    -sMODULARIZE=1 -sEXPORT_ES6=1
  ```
  (capturar os `undefined symbol:` — o inventário de adaptadores).
- Projeto AUI (46 TUs) wired no probe via `--extra-project`; o prelude foi
  corrigido para preferir as cópias adaptadas (`Common/AUInterface2`).

## Commits do Codex sobre o meu head (revisados por mim, todos corretos)

- `6dacf263` — probe: build de objetos do extra-project
- `008a7388` — case de include de interface
- `1a69893c` — string de fonte para o label formatter
- `9c32fe1b` — WAVEFORMAT/PCMWAVEFORMAT (alinhamento natural, pack(1)
  inofensivo — mesmos bytes) + strcmpi
- `d0632b47` — handlers de evento/fonte da AUI
- `cb14d3a5` — `max(1, ...)` → `max(1.0f, ...)`: resolve a ambiguidade das
  minhas sobrecargas (int+float não tem match exato), semântica idêntica
- `2604a56a` — casts explícitos no AUIObject::Save + teste de regressão
- `1265ebd1` — AUITagRegister/Unregister movidos (lazy instantiation, ok)

## Checklist de revalidação ao retomar (nesta ordem)

1. **Sessão do Codex**: `stat` no
   `~/.codex/sessions/2026/10/25/rollout-...01a17447....jsonl` — confirmar
   que parou de fato (idade > 15 min OU entrada terminal/quota no fim).
2. **WIP dele**: `git status --short` — preservar TODO arquivo modificado
   não-commitado (ler o diff, entender, completar ou commitar
   coerentemente; NUNCA descartar). Padrão das rodadas anteriores.
3. **HEAD**: `git log --oneline -20` — mapear os commits dele desde
   `5581b016`; revisar os que eu não revisei ainda (os listados acima já
   foram revisados por mim em 2026-10-27).
4. **Probe**: rodar o cycle em estado único (com `--extra-project` do AUI):
   ```
   python3 scripts/probe-client153-project.py \
     --source /home/nonexphere/workspace/pw/pw-web/references/client-153 \
     --emxx ~/.local/share/pw-toolchains/emsdk/upstream/emscripten/em++ \
     --port-root src/client153/Angelica2 --adapted-root src/client153 \
     --extra-project .../Common/AUInterface2/AUInterface_2.2.vcproj \
     --output artifacts/client153-project-probe/objects-resume.json \
     --all --jobs 8 --object-dir build/client153-original-objects
   ```
   Registrar o número (jogo + AUI) como a nova linha de base.
5. **Regressões do port**: `cmake --build build/client153-native &&
   ctest` — 29/29 esperado (ou mais, se ele adicionou testes).
6. **CI**: `gh pr checks 23 --repo brasalabs6/pw-web` — tudo verde no head.

## Frentes abertas (do meu handoff, seção terceira rodada)

1. **atlbase.h** (10 TUs, o bloqueador principal) — DOIS subagentes morreram
   por max_tokens nisso; fazer INLINE/INCREMENTAL: ler
   `src/client153/Element/CElementClient/PWAtl.h`, um símbolo por vez
   (CComPtr/CComBSTR/CAxWindow...), seam em `platform/common/atlbase.h`.
2. **Psapi.h** (6 TUs) — mesmo padrão honesto do tlhelp32 (enumeração de
   processos → falha explícita ERROR_CALL_NOT_IMPLEMENTED).
3. **Primeiro link real** (O MARCO): entry + libs do port + objetos AUI num
   módulo carregável; o inventário de símbolos faltantes guia o que falta.
4. Grupos pequenos: varargs AWString restantes (~6), case variants
   (EL_GrassLand.h, A3DGfxEx.h), PCMWAVEFORMAT (Codex já fez? verificar),
   AList2 include order, temporários de search commands (padrão: locals
   nomeados — os setters clonam sincronamente, VERIFICADO).

## Armadilhas ativas (não recair)

- Subagentes de seam com muita leitura morrem por max_tokens — escopo
  estrito ou inline.
- Comentários `//` inline podem engolir `);` — linha separada para
  comentário de adaptação.
- Scan de case variants: verificar a DIREÇÃO contra o arquivo real antes de
  criar forwarder (loop circular é fácil de criar).
- WikiSearcher setters CLONAM sincronamente — locals nomeados, não membros.
- O volume de clients foi reconstruído pelo agente do servidor (root-owned)
  — assets autorizados não estão lá; testes que dependem pulam por design.
- `-fshort-wchar` obrigatório em tudo (o CRT wide do sistema lê 32 bits —
  usar o seam `widestr`, nunca `wcs*` da libc).
