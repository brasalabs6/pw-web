---
id: PW23-039
title: "Regressão CI: macro __in do DirectShow contamina headers C++ padrão"
priority: P1
status: open
kind: reproduced-ci-regression
first_observed: 2026-10-28
verified_against: d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8
epic: e03-platform-portability-v1
scope: pr-23
validation: github-actions-ci-log
---

# PW23-039 — CI nativo quebrado por macro __in em headers C++

## Evidência reproduzida

O [Client 1.5.3 port build — run 40749029155](https://github.com/brasalabs6/pw-web/actions/runs/40749029155), job [native-and-wasm #123030412830](https://github.com/brasalabs6/pw-web/actions/runs/40749029155/job/123030412830), falhou no SHA `d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8` em 2026-10-28, na etapa **Native original-layout and bounded package tests**. Ao compilar `tests/client153/math.cpp`, o GCC 13.3 reportou dezenas de erros em `/usr/include/c++/13/bits/locale_facets.h` e `istream.tcc`, por exemplo `expected primary-expression before ',' token` em `locale_facets.h:2009`. A etapa **Build WebGL2 runtime and execute WASM parser regressions** foi **skipped**.

O arquivo [`tests/client153/math.cpp:1-7`](https://github.com/brasalabs6/pw-web/blob/d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8/tests/client153/math.cpp#L1-L7) inclui headers da Angelica antes de `<iostream>`. O header de plataforma [`Windows.h:3505-3508`](https://github.com/brasalabs6/pw-web/blob/d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8/src/client153/platform/common/Windows.h#L3505-L3508) inclui `dshow.h`, que inclui [`strmif.h:17-28`](https://github.com/brasalabs6/pw-web/blob/d3ebee66b6a1de165a0f5eba28f62cbbe81d24f8/src/client153/platform/common/strmif.h#L17-L28). Este último define `#define __in` vazio como anotação SAL. A biblioteca padrão libstdc++ usa `__in` como identificador de parâmetro; a expansão do macro o apaga, compatível com a sequência de erros observada.

**Classificação:** regressão real de compilação no CI. A cadeia de includes e o macro são verificáveis no código; a causa é inferência estática forte apoiada pelo log, não teste de correção. Não há evidência de crash do jogo ou vulnerabilidade explorada.

## Ação proposta ao agente do port (não executada aqui)

Confinar as macros SAL legadas ao escopo dos headers DirectShow, sem poluir headers C++ padrão; considerar anotações com nomes próprios ou `push_macro/pop_macro` e isolamento de includes, verificando os callers que precisam dessas anotações. Não desativar warnings nem modificar a libstdc++.

## Aceite

1. Reproduzir no container CI e corrigir a contaminação de `__in` preservando contratos dos headers DirectShow.
2. O build nativo e os testes CTest/Python passam, sem erros de headers padrão.
3. O estágio WASM executa e apresenta seu resultado; registrar SHA de correção e link do CI verde neste arquivo.
4. Não confundir CI verde com jogo renderizado ou jogável em Chrome desktop/mobile.

## Deduplicação

[PW23-038](PW23-038-ddraw-struct-layout-ci-regression.md) é uma falha anterior de layout DDS resolvida; [PW23-006](PW23-006-winapi-false-success-and-pointer-state.md) trata comportamento de shims WinAPI; [PW23-013](PW23-013-test-gates-not-browser-game.md) trata cobertura. Nenhum cobre esta contaminação do namespace C++ pelo `__in` de `strmif.h`. A falha transitória de `CompareStringW` no [run 40742474498](https://github.com/brasalabs6/pw-web/actions/runs/40742474498) já recebeu `(void)flags` no SHA atual e não é confundida com o erro novo.

Sem alteração de código-fonte, PR adicional, merge ou modificação do servidor #17.
