# ADR-0001 — Referências upstream por gitlinks pinados

**Status:** aceito; gate jurídico documentado em `docs/LEGAL.md` (2026-10-25).
**Data:** 2026-10-24.

## Contexto
É necessário inspecionar fontes antigas C++ sem incorporar conteúdo de terceiros ao histórico principal ou ao CI.

## Decisão
Adicionar `.gitmodules` e gitlinks com SHA fixo para 1.5.3, 1.5.2 modernizada e servidor Rust sob `references/`. Checkout padrão sem `--recursive`. Upstreams permanecem isolados e não entram no artefato do navegador.

## Consequências
- Rastreabilidade de versões e diffs; desenvolvedor pode inicializar apenas a referência necessária.
- Submodule fixa o upstream; uso/redistribuição segue a autorização registrada em `docs/LEGAL.md`.
- Edições em submódulo precisam de fork/autorização e mudança deliberada do gitlink, não são projeto próprio automaticamente.
