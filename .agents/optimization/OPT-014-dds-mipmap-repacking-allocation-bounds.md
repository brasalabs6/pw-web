---
id: OPT-014
title: "Auditar repacking de mipmaps DDS e ownership de buffer temporário"
priority: P1
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 07b8e843acd4a019d2fda991bf14c2f5b63433f1
first_observed: 2026-10-28
validation: static-only
epic: e05-asset-pipeline-v1
scope: pr-23
---

# OPT-014 — Auditar repacking de mipmaps DDS e ownership de buffer temporário

**Escopo:** Legacy Angelica DDS loading. **Candidato preventivo pós-port**, não blocker comprovado. Não implementar tuning até o cliente WASM original executar no navegador desktop e celular e existir baseline de FPS, RAM/VRAM, frame time e estabilidade. Caso um crash/vulnerabilidade grave seja reproduzido, registrar a falha em `.agents/issues` com evidência, sem esperar pelo tuning.

## Evidência exata no SHA auditado
[`src/client153/Angelica2/Angelica3D/Source/A3DTexture.cpp:413-432`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/src/client153/Angelica2/Angelica3D/Source/A3DTexture.cpp#L413-L432) acessa o cabeçalho e os campos DDS antes de checar tamanho mínimo do buffer no caminho legado; [`src/client153/Angelica2/Angelica3D/Source/A3DTexture.cpp:459-490`](https://github.com/brasalabs6/pw-web/blob/07b8e843acd4a019d2fda991bf14c2f5b63433f1/src/client153/Angelica2/Angelica3D/Source/A3DTexture.cpp#L459-L490) usa `(width>>2)*(height>>2)` para blocos DXT (subestima níveis menores que 4px), calcula offsets com `DWORD`, aloca `new char[size]` e libera com `delete buf` (em vez de `delete[]`). Há também dependência do layout de `DDSURFACEDESC2` atualmente quebrado no CI, documentada em PW23-038.

## Impacto possível e grau de certeza
O padrão de liberação new[]/delete é UB em C++; offsets incorretos podem corromper mip chains ou disparar erros de leitura/texture upload. Nenhum teste de crash ou GPU foi executado. O helper pode não ser usado no visualizador WebGL2 atual.

## Estratégia candidata, sem mudança de código nesta rodada
Somente depois do port funcional: determinar call sites e formatos DDS reais, substituir leitura por parsing com limites/endianness explícitos e armazenamento RAII; calcular blocos comprimidos com max(1,ceil(w/4))*max(1,ceil(h/4))*blockBytes e tamanhos checked; validar mip count e invariantes de formato. Não confundir com a correção imediata de layout em PW23-038.

## Testes e critério de aceite pós-port
DDS DXT1/3/5, BGRA, mips 1x1/2x2/3x3/4x4, NPOT, mip chains truncadas, cabeçalhos <128 bytes, fuzz de offsets; ASan/UBSan, upload WebGL2 e diff de textura/mipmaps; medir CPU, bytes copiados e pico de heap apenas após baseline.

**Gate de aceitação:** revalidar contra HEAD, registrar dispositivo/SHA/cena/qualidade, before/after com CPU/GPU frame time p50/p95/p99, memória e regressões funcionais; não prometer ganhos antes dos testes.

## Deduplicação e dependências
Distinto de PW23-010 (S3TC obrigatório indevidamente), PW23-028 (ASTC/ETC2), OPT-002 (dedup de upload) e PW23-038 (layout 32/64-bit confirmado no CI).
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), SHA auditado `07b8e843acd4a019d2fda991bf14c2f5b63433f1`.
- [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md).
- Preservar PW23-021…037 e OPT-001…012; não abrir PR, alterar o servidor ou realizar auto-merge.
