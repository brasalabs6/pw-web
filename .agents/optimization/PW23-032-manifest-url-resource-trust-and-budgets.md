---
id: PW23-032
title: "Restringir origem, tamanho e quantidade de assets do manifest"
priority: P1
status: open
kind: security-hardening
first_observed: 2026-10-28
verified_against: 6184be84d4d8766ac02f489c4867df2e0044539a
epic: e17-security-deployment-v1
scope: pr-23
validation: static-inspection-only
---

# PW23-032 — Restringir origem, tamanho e quantidade de assets do manifest

> **Nível de evidência:** inspeção estática no SHA acima. Não houve benchmark, exploração, ou execução em Android. Revalidar HEAD imediatamente antes de corrigir. Referência [PR #23](https://github.com/brasalabs6/pw-web/pull/23).

## Evidência no código
[`web/client153/app.mjs:7-12`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/web/client153/app.mjs#L7-L12) obtém bytes com `fetch(asset.url)` e só depois compara tamanho/sha256; [`web/client153/app.mjs:59-69`](https://github.com/brasalabs6/pw-web/blob/6184be84d4d8766ac02f489c4867df2e0044539a/web/client153/app.mjs#L59-L69) carrega `scene.json`. Há validação de integridade (positivo), mas não limite antes do download, timeout, controle de concorrência ou allowlist explícita de origem.

## Impacto e hipótese técnica
Manifesto comprometido ou configurado incorretamente pode induzir downloads externos/grandes, consumo de rede e RAM, ainda que o hash impeça aplicação do conteúdo incorreto. É hardening de supply-chain/DoS, não exploração confirmada.

## Implementação proposta
Permitir apenas mesmo-origin/CDN aprovado com HTTPS, schema estrito, size/quantity caps, AbortController timeout, Content-Length advisory não confiável + streaming hard cap, limites de concorrência, assinatura opcional do manifest, CSP e logs sem segredos.

## Testes / plano de medição
Manifest com URL externo, redirect, content-length falso, bytes infinitos, hash inválido, duplicatas, paths ../, timeout, CORS e cancelamento; verificar rejeição sem alocar todo payload.

## Critério de aceite
Nenhum asset não autorizado ou acima do teto de bytes entra no cache/heap; contratos CSP/CORS e rollback documentados.

## Dependências e governança
- Épico: [e17-security-deployment-v1](../epics/e17-security-deployment-v1/README.md).
- Consultar `AUDIT-CLIENT153-PERFORMANCE-MOBILE.md` e questões anteriores `PW23-001..020`; não duplicar backlog.
- Atualizar este arquivo com `status: resolved`, commit, dispositivo/comandos, before/after e link de evidência quando corrigido.
- Nenhum merge automático e não abrir terceiro PR.
