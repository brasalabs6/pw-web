---
id: OPT-036
title: "Verificar leitura completa e limites de payload no leitor PCK legado"
priority: P1
status: candidate
phase: post-browser-port
blocking: false
kind: static-optimization
source_sha: 6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c
first_observed: 2026-10-28
validation: static-only
epic: e17-security-deployment-v1
scope: PR-23
---

# OPT-036 — Verificar leitura completa e limites de payload no leitor PCK legado

**Gate:** hipótese estática para **depois** que o cliente Angelica 1.5.3 rodar funcionalmente via WASM no navegador de PC e mobile; nenhuma alteração de código-fonte de tuning foi executada. Não alegar ganho de MB, taxa de compressão, FPS ou carga de browser real.

## Área
PCK / hardening preventivo

## Evidência no HEAD auditado
[`src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp:686-737`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp#L686-L737) chama `m_fpPackageFile->read(...)` nas rotas comprimida e não comprimida sem verificar explicitamente o retorno neste método; [`src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp:742-757`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/src/client153/Angelica2/AngelicaFile/Source/AFilePackGame.cpp#L742-L757) recebe o tamanho lido mas chama `Decrypt` com o tamanho **solicitado**. O leitor offline Python [`tools/assets/pck153.py:48-67`](https://github.com/brasalabs6/pw-web/blob/6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c/tools/assets/pck153.py#L48-L67) rejeita truncamento.

## Impacto possível (hipótese)
Arquivo corrompido/truncado pode seguir para processamento com bytes parciais; em caminho comprimido inflate tende a falhar, mas no caminho cru e no ReadCompressedFile não há rejeição explícita aqui. **Hipótese de robustez**, não foi demonstrado crash/exploração.

## Experimento / estratégia posterior
Adicionar no futuro testes/contrato de retorno exato da leitura, offset/length antes do seek, max decoded bytes por classe e validação de decompressed length; impedir que conteúdo parcial entre no cache/hash. **Se reproduzir crash ou corrupção impeditiva, promover a .agents/issues**.

## Plano de benchmark / validação
Fixtures PCK de payload curto, PKX ausente, truncate entre header e data, falhas de seek simuladas, byte count 0 e máximo; sanitizers e testes WASM/nativos com bytes exatos e erro determinístico, sem registrar dados sensíveis.

**Critério de aceite:** demonstrar equivalência de bytes e hashes de arquivos, comportamento de loading e de gameplay, compatibilidade com formato legado; medir TTFP/TTI, bytes transferidos, compressão real, p50/p95/p99 de CPU/decode/I/O e pico RAM/VRAM quando disponível. Registrar SHA, dispositivo, rede/RTT, tamanho da amostra e before/after na mesma cena.

## Deduplicação e dependências
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23), fonte inspecionada `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`.
- Épico [e17-security-deployment-v1](../epics/e17-security-deployment-v1/README.md).
- Checar PW23-020 (entrega/cloud), PW23-024 (cópias JS/WASM), PW23-027 (streaming do terreno), PW23-032 (política de confiança do manifest), OPT-002 (dedup de textura), OPT-003 (inflate do payload), OPT-014 (DDS legado) e OPT-024 (SGC de skills). Este registro trata exclusivamente o recorte descrito.
- Se confirmar vulnerabilidade crítica, crash ou falha impeditiva de port, reclassificar com reprodutor em `.agents/issues`; esta pasta não é autorização de adiamento de incidente real.
- Não abrir terceiro PR, não tocar PR #17 do servidor, não fazer auto-merge.
