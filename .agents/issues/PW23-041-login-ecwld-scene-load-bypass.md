---
id: PW23-041
title: "Login.ecwld falha ao abrir no Chromium; bypass de LoadSceneObjects não comprova cena renderizada"
priority: P0
status: open
kind: browser-port-blocker
first_observed: 2026-10-29
verified_against: b1d838f8612328dc22cf8b7b2031bb1cacc47532
epic: e05-asset-pipeline-v1
scope: pr-23
---

# PW23-041 — Carregamento da cena de login (`Login.ecwld`) não concluído

## Evidência e grau de confirmação

O commit [`b1d838f`](https://github.com/brasalabs6/pw-web/commit/b1d838f8612328dc22cf8b7b2031bb1cacc47532) relata que `Maps\\Login\\Login.ecwld` falha ao abrir no browser e que a causa ainda depende da investigação do índice de pacotes. O código em [`EC_World.cpp:572–579`](https://github.com/brasalabs6/pw-web/blob/b1d838f8612328dc22cf8b7b2031bb1cacc47532/src/client153/Element/CElementClient/EC_World.cpp#L572-L579) deixou de retornar erro quando `LoadSceneObjects(szFile)` falha, emitindo somente log e continuando o boot. Trata-se de **falha funcional declarada pelo executor e bypass confirmado por inspeção do código**; esta auditoria não reproduziu a abertura do arquivo em Chromium nem validou os assets.

O mesmo commit relata que o probe Chromium permaneceu em execução por mais de 600 segundos, até timeout, em vez de encerrar em 10–15 segundos. **Isso sugere progresso na inicialização, mas não comprova por si só que o game loop renderiza frames, processa input ou está livre de deadlock**. Não há screenshot, captura de frame, evidência de login renderizado ou teste mobile associado a esse marco.

## Impacto no port

A cena original de login não está carregada corretamente. Prosseguir sem objetos de cena é útil como diagnóstico temporário, mas não valida a renderização do cliente original nem o carregamento íntegro do mundo. Não transformar um erro de asset em sucesso de boot definitivo.

## Diagnóstico e correção propostos (antes de tuning)

1. Capturar o caminho exato solicitado pelo `LoadSceneObjects`, resultado de `AFilePack`/VFS, pacote PCK/PKX consultado, entrada de índice e erro de abertura; comparar com a mesma versão dos assets originais autorizados no nativo.
2. Distinguir ausência do arquivo, normalização de separadores/caixa, roteamento de pacote, índice/offset e falha de parsing do formato `.ecwld`. Registrar SHA dos assets de fixture e logs sanitizados.
3. Corrigir o carregamento original. Manter qualquer bypass apenas atrás de modo diagnóstico explícito; no fluxo normal, reportar falha sem falso sucesso.
4. No Chromium real, confirmar carregamento de cena, renderização visível e avanço de input/login com evidência (screenshot ou frame, logs e ambiente). Depois validar em aparelho móvel real, sem pressupor equivalência ao desktop.

## Critério de aceite

A mesma `Login.ecwld` autorizada abre e é interpretada corretamente no cliente WASM; o caminho normal de `LoadSceneObjects` retorna sucesso sem bypass, com cena e UI originais efetivamente desenhadas no Chromium. Anexar comandos, logs, SHA, asset fixture e evidência visual; testes mobile separados.

## Deduplicação

Distinto de [PW23-040](PW23-040-elements-data-browser-parser-init-failure.md), que trata do parser binário de `elements.data`; de [PW23-014](PW23-014-angelica-d3d8-webgl2-not-integrated.md), que trata do backend gráfico Angelica; e de [PW23-020](PW23-020-assets-streaming-cache-cloudflare-pending.md), que trata de entrega/streaming amplo de assets. Este registro acompanha **uma falha específica de abertura da cena original**, já reconhecida no caminho de boot. Não é hipótese de FPS ou otimização.
