---
id: OPT-045
title: "Medir conversão e upload integral em cada UnlockRect de textura D3D8"
priority: P2
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: cf1121324c6cca0e566b698311c45a85eea0dba9
first_observed: 2026-10-29
validation: static-only
epic: e06-render-webgl2-v1
scope: pr-23
---

# OPT-045 — Conversão e upload integral de texturas em UnlockRect

**Estado:** candidato estático para investigação **após** o cliente Angelica 1.5.3 executar gameplay no navegador desktop e mobile. Não é defeito reproduzido, não é blocker de render, e não justifica modificar o código nesta fase.

## Evidência no código (SHA imutável)

- [`d3d8_texture.cpp:44-57`](https://github.com/brasalabs6/pw-web/blob/cf1121324c6cca0e566b698311c45a85eea0dba9/src/client153/platform/web/d3d8_texture.cpp#L44-L57): `LockRect` aceita uma região e devolve ponteiro para o espelho CPU de pixels, com `Pitch` da textura completa.
- [`d3d8_texture.cpp:59-85`](https://github.com/brasalabs6/pw-web/blob/cf1121324c6cca0e566b698311c45a85eea0dba9/src/client153/platform/web/d3d8_texture.cpp#L59-L85): todo `UnlockRect` com escrita aloca `width * height * 4` bytes, converte **todos** os pixels BGRA→RGBA em loop e envia a textura inteira via `glTexSubImage2D`, mesmo quando o lock foi de um retângulo menor.
- [`d3d8_texture.cpp:89-113`](https://github.com/brasalabs6/pw-web/blob/cf1121324c6cca0e566b698311c45a85eea0dba9/src/client153/platform/web/d3d8_texture.cpp#L89-L113): a textura gerenciada mantém um espelho CPU de `width * height * 4` bytes, além do armazenamento GL. A alocação temporária no unlock pode elevar o pico simultâneo de heap.

## Impacto possível e limites da conclusão

Há trabalho de CPU proporcional à **área total** por atualização e uma alocação transitória proporcional ao tamanho total, independentemente do retângulo alterado. Em atualizações frequentes de atlas, UI ou texturas dinâmicas isso **pode** elevar tempo de CPU, tráfego de upload e pico de RAM/WASM; a relevância para o cliente real é desconhecida. Não há medição de FPS, RAM, VRAM, energia ou ocorrência comprovada no gameplay. Não inferir que o espelho CPU possa ser removido sem revisar o contrato D3D8 de `LockRect`/pool/retention.

## Plano de benchmark (fase post-browser-port)

1. Após o primeiro jogo funcional, inventariar call sites reais de `LockRect/UnlockRect`, formato, tamanho, frequência, retângulos e pool. Comparar texturas de UI, modelos e efeitos; identificar quantas são atualizadas e quantas apenas carregadas uma vez.
2. Instrumentar, no **mesmo SHA, cena, resolução, qualidade e dispositivo**, bytes convertidos e enviados por unlock, contagem/tempo de alocações, CPU frame time p50/p95/p99, GPU frame time quando disponível, draw calls, pico WASM heap/RAM e estimativa de VRAM. Executar em Chrome desktop e Android físico; registrar aquecimento e consumo/temperatura somente quando houver instrumentação adequada.
3. Criar fixture de textura 512×512 e 2048×2048, com atualizações totais e de 16×16/64×64, preservando semântica D3D8, pixel format e estado GL. Testar perda/restauração de contexto e comparação de pixels/screenshot.
4. Só então comparar uma implementação de dirty rect/upload parcial e conversão limitada, ou staging reutilizável; garantir `Pitch`, leitura após escrita, lifetime e compatibilidade com WebGL2. Não otimizar com base apenas no fixture sintético.

## Deduplicação e governança

Distinto de [OPT-002](OPT-002-dedup-texture-fetch-upload.md) (download/upload redundante **entre meshes**), [OPT-014](OPT-014-dds-mipmap-repacking-allocation-bounds.md) (loader **legado DDS**) e [PW23-024](PW23-024-asset-copy-amplification-wasm.md) (cópias **fetch/VFS/parser**). Este item cobre exclusivamente o **caminho recém-adicionado de atualização D3D8 LockRect→UnlockRect→WebGL2**. Revalidar contra HEAD antes de executar.

Não abrir issue sem regressão/crash/segurança crítica reproduzidos; não alterar fonte para tuning, não tocar PR #17, não criar PR novo e não executar auto-merge. Histórico PW23-021…037 preservado.
