---
id: PW23-038
title: "Regressão CI: layout DDSURFACEDESC2 diverge no build nativo 64-bit"
priority: P1
status: resolved
kind: reproduced-ci-regression
first_observed: 2026-10-28
verified_against: 4bd108b30650647d1c510db252442de764b27dd1
resolved_at: 2026-10-28
resolution_commit: 368c9da207cb6b8a38e86dea39d39b0b6dd2b8f8
resolution_ci_run: 40712327768
epic: e03-platform-portability-v1
scope: pr-23
validation: github-actions-ci-log
---

# PW23-038 — CI falha em layout DDSURFACEDESC2

## Resolução revalidada no HEAD 4bd108b (2026-10-28)

**Estado: resolved (regressão de layout/CI).** O commit [368c9da](https://github.com/brasalabs6/pw-web/commit/368c9da207cb6b8a38e86dea39d39b0b6dd2b8f8) substituiu `LONG lPitch` por campo de 32 bits e o slot `void *lpSurface` por `DWORD dwSurfaceSlot` no layout do DDS serializado. O header mantém os `static_assert` de 124 bytes e offsets fixos: [ddraw.h L78–137](https://github.com/brasalabs6/pw-web/blob/4bd108b30650647d1c510db252442de764b27dd1/src/client153/platform/common/ddraw.h#L78-L137). O teste [test_ddraw_header.py](https://github.com/brasalabs6/pw-web/blob/4bd108b30650647d1c510db252442de764b27dd1/tests/client153/test_ddraw_header.py#L1-L22) compila o contrato no host nativo.

O [CI Client 1.5.3 port build #158 — run 40712327768](https://github.com/brasalabs6/pw-web/actions/runs/40712327768) terminou **success** no SHA `4bd108b30650647d1c510db252442de764b27dd1`; job `native-and-wasm` #122900609259: **Native original-layout and bounded package tests = success** e **Build WebGL2 runtime and execute WASM parser regressions = success**. Isso confirma que o gate antes falho voltou a passar. Não demonstra render da Angelica original, gameplay, nem prova teste de DDS em todos os aparelhos/formatos.

**Observação:** a seção abaixo conserva o diagnóstico histórico da falha no SHA original. Os testes de corpus DDS completo, GPUs reais e a validação de compatibilidade de todas as interfaces COM permanecem trabalho futuro, não são necessários para classificar como resolvida a regressão CI específica.

## Evidência reproduzida pelo CI
No [workflow Client 1.5.3 port build, run 40699845272](https://github.com/brasalabs6/pw-web/actions/runs/40699845272), job [native-and-wasm #122856356455](https://github.com/brasalabs6/pw-web/actions/runs/40699845272/job/122856356455), a etapa **Native original-layout and bounded package tests** falhou em 2026-10-28 13:49 UTC, executando `tests/client153/test_ddraw_header.py::test_interface_contract`.

Logs: `src/client153/platform/common/ddraw.h:123` — `sizeof(DDSURFACEDESC2)` obtido **136**, esperado **124**; `ddraw.h:130` — `offsetof(ddpfPixelFormat)` obtido **80**, esperado **72**. Os 31 testes CTest passaram, mas 1 dos 22 testes Python falhou e a etapa WASM seguinte foi pulada.

[`src/client153/platform/common/ddraw.h:72-132`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/src/client153/platform/common/ddraw.h#L72-L132) define `DDSURFACEDESC2` com `void *lpSurface` (largura de ponteiro do host, 64-bit no CI) dentro de um layout serializado originalmente 32-bit. [`tests/client153/test_ddraw_header.py:1-22`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/tests/client153/test_ddraw_header.py#L1-L22) compila o header com C++ nativo. [`src/client153/Angelica2/Angelica3D/Source/A3DTexture.cpp:414-423`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/src/client153/Angelica2/Angelica3D/Source/A3DTexture.cpp#L414-L423) faz cast direto do cabeçalho DDS em memória para `DDSURFACEDESC2` e depende dos offsets documentados.

## Classificação e impacto
**Regressão CI efetivamente observada**, não hipótese de FPS e não prova de exploração. O build/teste da branch falha, interrompendo o gate antes da etapa de testes WASM. Pode indicar também leitura de metadados DDS incorreta no caminho legado se o mesmo struct for usado com ponteiros de 64 bits; isso precisa de teste independente.

## Proposta de correção para o agente do port (não aplicada aqui)
Separar o wire-format DDS (campos 32-bit e offsets fixos) das interfaces DirectDraw/COM de runtime com ponteiros nativos; alternativamente representar `lpSurface` no struct serializado como `uint32_t` e converter somente na borda apropriada, sem alterar o ABI das interfaces COM. Preservar os `static_assert` de 124 bytes, offset 72 e layout de `DDPIXELFORMAT`. Evitar `#pragma pack` isolado se o campo permanecer 64-bit, pois packing não reduz sua largura.

## Testes / aceite
1. Reexecutar `python -m unittest tests/client153/test_ddraw_header.py` no mesmo container/compilador CI.
2. Reexecutar a suíte nativa, incluindo os 31 CTest e os testes Python; comprovar o stage WASM não foi pulado.
3. Testar parse de DDS 32-bit com cabeçalho real e DXT1/DXT3/DXT5/BGRA, tamanhos e offsets; evitar leitura via struct com ponteiro host.
4. Registrar SHA de correção, link da execução verde e resultados no próprio arquivo; fechar somente com evidência.

## Deduplicação
PW23-008 acompanha closure/link geral, PW23-010 acompanha S3TC para BGRA, PW23-013 acompanha lacunas de prova no Chrome. Nenhum registra a falha concreta de layout `DDSURFACEDESC2` do run acima. OPT-014 cobre apenas hardening/tuning do repacker DDS depois do port e não substitui esta correção.

## Referências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA `07b8e843acd4a019d2fda991bf14c2f5b63433f1`.
- [e03-platform-portability-v1](../epics/e03-platform-portability-v1/README.md).
