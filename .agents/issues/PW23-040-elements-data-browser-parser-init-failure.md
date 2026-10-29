---
id: PW23-040
title: "Falha de parsing de elements.data no boot WASM foi contornada, não corrigida"
priority: P0
status: open
kind: browser-port-blocker
first_observed: 2026-10-29
verified_against: 413717155c1ac72573f447e3c29ae801d84949c9
epic: e05-asset-pipeline-v1
scope: pr-23
---

# PW23-040 — Falha de parsing de elements.data no boot WASM

## Evidência e limite da confirmação

No [commit 4137171](https://github.com/brasalabs6/pw-web/commit/413717155c1ac72573f447e3c29ae801d84949c9), o desenvolvedor informa que o parser binário `itemdataman` falha no boot browser, com possível diferença de alinhamento de structs ainda **não diagnosticada**. A alteração em [EC_Game.cpp:558–577](https://github.com/brasalabs6/pw-web/blob/413717155c1ac72573f447e3c29ae801d84949c9/src/client153/Element/CElementClient/EC_Game.cpp#L558-L577) substituiu o `return false` após falha de `Load` por um log e continuação, explicitamente para diagnóstico do boot. A mensagem do commit relata avanço até `CECGameRun::StartLogin` / `CreateLoginWorld`, com falha posterior em `LoadTerrain`. Esta auditoria confirmou o desvio **no código** e a evidência **declarada no commit**; não executou o cliente nem validou logs independentes.

## Impacto funcional

Sem carregar `elements.data` corretamente, a inicialização pode avançar para diagnóstico, mas dados de itens/elementos permanecem ausentes ou incompletos. Isso impede considerar login, inventário e gameplay como funcionais. Não tratar a supressão da falha como correção, nem inferir que a hipótese de alinhamento está provada.

## Diagnóstico / correção proposta (port antes de tuning)

1. Preservar um modo de diagnóstico explicitamente identificado; em execução normal, exigir `elements.data` válido e reportar erro sem falso sucesso.
2. Reproduzir com fixture autorizado, capturar offsets, tamanho esperado/lido, layout e packing das structs, versão dos dados e primeira entrada divergente no parser nativo e WASM.
3. Corrigir o contrato de serialização/layout na origem, sem descartar silenciosamente campos, sem alterar conteúdo original para mascarar incompatibilidade.
4. Após correção, continuar o boot e investigar separadamente a falha de `LoadTerrain`, sem marcar este registro resolvido por alcançar `StartLogin`.

## Critério de aceite

Mesmos dados originais autorizados carregam com contagens e checksums verificáveis no nativo e WASM; erros de truncamento/versão são explícitos; boot no Chromium alcança a UI de login com dados de elementos íntegros. Anexar logs, comandos, SHA e ambiente. Sem evidência de execução em Android/iOS nesta rodada.

## Deduplicação

Distinto de [PW23-007](PW23-007-missing-full-game-wasm-target.md) (entry/link do jogo), [PW23-012](PW23-012-encoding-gbk-utf16-utf8-mismatch.md) (conversões textuais) e [PW23-014](PW23-014-angelica-d3d8-webgl2-not-integrated.md) (render). É um erro de **dados binários do cliente** observado no caminho de boot. Não é tuning.

## Complemento sobre memória (29/10) — não validar 2,9 GiB sem verificar o parser

O executor relatou que `elements.data` exigiria ~2,9 GiB e configurou `-sMAXIMUM_MEMORY=4GB`. **Isto é relato local, não benchmark independente de necessidade legítima:** o parser `itemdataman` continua com falha/bypass de `elements.data` neste issue. Um `sizeof`/packing diferente, contagem ou offset corrompido e loop de parsing inválido podem levar a alocações gigantes sem que o dataset demande isso.

**Diagnóstico obrigatório antes de refatorar:** medir tamanho real do arquivo, versão, contagens por seção, offsets, checksums, `sizeof/alignof/offsetof` MSVC 32-bit vs WASM32, pico heap e cópias em MEMFS/JS; abortar com erro explícito em contagens absurdas. `MAXIMUM_MEMORY=4GB` é limite superior em wasm32 com memory growth, **não orçamento viável de celular**. Depois da correção funcional, se a necessidade for real, aplicar streaming de registros/índices compactos/descartar temporários/lazy decode, mantendo consistência dos dados. Não aprovar game loop com `itemdataman` incompleto. [Análise no PR](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6499021552).
