---
id: PW23-042
title: "Novo teste FTDriver exige clang++ ausente no runner e quebra o CI do cliente"
priority: P1
status: open
kind: reproduced-ci-regression
first_observed: 2026-10-29
verified_against: 64579d021a51460a2e0d5dfe5dfa659859f8ded2
epic: e16-qa-observability-v1
scope: pr-23
---

# PW23-042 — Novo teste FTDriver exige clang++ ausente no runner e quebra o CI do cliente

## Evidência reproduzida no GitHub Actions

- **Branch/PR:** `feat/client153-common-containers`, [PR #23](https://github.com/brasalabs6/pw-web/pull/23).
- **SHA do código:** [`64579d021a51460a2e0d5dfe5dfa659859f8ded2`](https://github.com/brasalabs6/pw-web/commit/64579d021a51460a2e0d5dfe5dfa659859f8ded2).
- **Execução real:** [Client 1.5.3 port build — run 40861100236, job 123404241032](https://github.com/brasalabs6/pw-web/actions/runs/40861100236/job/123404241032), concluída em **failure** em 2026-10-29. O CTest nativo concluiu **33/33 PASS**, mas `python3 -m unittest discover -s tests/client153 -p 'test_*.py'` terminou com **31 testes, 1 erro**. O estágio WASM foi **skipped**.
- **Erro exato do log:** `ERROR: test_error_does_not_claim_ui_font_fallback` em `test_ftdriver_failure.FTDriverFailureTest`; `FileNotFoundError: [Errno 2] No such file or directory: 'clang++'`.
- **Causa diretamente observável:** [`tests/client153/test_ftdriver_failure.py:15-21`](https://github.com/brasalabs6/pw-web/blob/64579d021a51460a2e0d5dfe5dfa659859f8ded2/tests/client153/test_ftdriver_failure.py#L15-L21) invoca `clang++` diretamente, enquanto o [workflow instala apenas zlib e executa os testes Python no container Emscripten](https://github.com/brasalabs6/pw-web/blob/64579d021a51460a2e0d5dfe5dfa659859f8ded2/.github/workflows/client153-port.yml#L10-L37), sem garantir o executável `clang++` no PATH.
- **Regressão de gate:** o mesmo workflow no [SHA anterior `c1333f6` (run 40859344755)](https://github.com/brasalabs6/pw-web/actions/runs/40859344755) terminou em success, com **30 testes Python**. O arquivo `test_ftdriver_failure.py` não existia naquele SHA e foi incluído no intervalo de três commits até o HEAD auditado.

## Impacto

Falha **real e reproduzida no CI**, não falha de renderização nem prova de defeito no jogo. O novo teste impede a validação WASM por curto-circuito do workflow e torna o gate do PR vermelho. A integração do FreeType/FTDriver original, adicionada no mesmo intervalo, continua **sem validação WASM completa nesse run**. Não confundir com desempenho/FPS ou testes mobile.

## Correção sugerida, sem tuning

Tornar o compilador usado pelo teste explícito e disponível no ambiente: selecionar `CXX`/compilador encontrado por `shutil.which` com opções compatíveis, **ou** instalar/configurar `clang++` na imagem de CI antes da descoberta Python. Se o teste ainda valida o stub legado, confirmar que seu propósito continua distinto do probe do driver original e que não impede a cobertura do novo caminho integrado. Evitar `skip` silencioso que produza falsa aprovação.

## Testes e aceite

1. Em checkout limpo do SHA corrigido, executar `python3 -m unittest discover -s tests/client153 -p 'test_*.py'` no mesmo container do workflow e confirmar que o teste FTDriver é executado, não omitido.
2. Executar workflow `Client 1.5.3 port build` até o estágio WASM; registrar o run URL, SHA, resultado de CTest nativo, testes Python e CTest WASM.
3. Atualizar este arquivo com `status: resolved`, `resolved_at`, commit e evidências **apenas após** CI verde no SHA corrigido.

## Deduplicação e limites

Não duplica [PW23-013](PW23-013-test-gates-not-browser-game.md): aquele trata cobertura/semântica dos gates e protocolo Chromium, enquanto este documenta **regressão concreta do workflow por dependência de compilador não declarada**. Não duplica PW23-038/039 (regressões anteriores já resolvidas). Nenhum benchmark, teste Chrome com jogo original ou teste mobile foi realizado nesta auditoria; não há alteração de código-fonte, servidor PR #17 ou outro PR.
