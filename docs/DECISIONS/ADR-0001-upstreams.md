# ADR-0001 — Referências upstream por gitlinks pinados

**Status:** aceito para bootstrap, sujeito ao gate jurídico.
**Data:** 2026-10-24.

## Contexto
É necessário inspecionar fontes antigas C++ sem incorporar conteúdo de terceiros ao histórico principal ou ao CI.

## Decisão
Adicionar `.gitmodules` e gitlinks com SHA fixo para 1.5.3, 1.5.2 modernizada e servidor Rust sob `references/`. Checkout padrão sem `--recursive`. Upstreams permanecem isolados e não entram no artefato do navegador.

## Consequências
- Rastreabilidade de versões e diffs; desenvolvedor pode inicializar apenas a referência necessária.
- Submodule não dá permissão de redistribuir, licença de modificação ou ownership; gate `docs/LEGAL.md`.
- Edições em submódulo precisam de fork/autorização e mudança deliberada do gitlink, não são projeto próprio automaticamente.
