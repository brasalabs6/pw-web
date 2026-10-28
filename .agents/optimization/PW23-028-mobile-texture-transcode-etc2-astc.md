---
id: PW23-028
title: "Pipeline de formatos comprimidos para GPUs móveis"
priority: P1
status: open
kind: architecture-optimization
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e05-asset-pipeline-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-028 — Pipeline de formatos comprimidos para GPUs móveis

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`src/client153/platform/web/runtime.cpp:260-297`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/src/client153/platform/web/runtime.cpp#L260-L297) suporta DDS BGRA por `glTexImage2D` RGBA8 e DXT1/3/5 comprimidos mediante S3TC; não há caminho de entrega de KTX2/BasisU/ETC2/ASTC nessa função.

## Impacto e hipótese técnica
S3TC não é garantido em mobile; RGBA8 pode multiplicar memória/bandwidth em relação a formatos comprimidos. Sem fallback, parte dos assets pode deixar de carregar.

## Implementação proposta
Pipeline offline verificado por hash: DDS origem → intermediário KTX2/BasisU, gerar ETC2 para WebGL2 mobile (ou transcoder com ASTC quando compatível) e S3TC desktop, normal maps com atenção aos canais; mipmaps, budget e fallback explícito.

## Testes / plano de medição
Assets dourados visuais, Android Mali/Adreno e Windows/Mac; medir tamanho em rede, tempo decode, tempo upload, VRAM estimada e artefatos.

## Critério de aceite
Mesmo conjunto autorizado de texturas carrega em GPUs sem S3TC, mantendo equivalência de materiais dentro de tolerância aprovada.

## Dependências e governança
- Épico: [e05-asset-pipeline-v1](../epics/e05-asset-pipeline-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
