# ADR-0003 — Seleção de cliente/engine ainda condicional

**Status:** proposto (não aprovado).
**Data:** 2026-10-24.

## Candidatos
- 1.5.3 com Angelica2/Direct3D8: candidato inicial pela proximidade potencial da engine e código disponível.
- 1.5.2 modernizado com Angelica2.2/Direct3D9: fallback para toolchain moderno.
- Rust server: referência de protocolo, em especial versões 1.2.6 e 1.5.5, sem presumir 1.5.3 funcional.

## Experimentos requeridos
Build Windows limpo, rastreio de APIs/arquivos, entrada no mundo, compatibilidade assets/protocolo, prova de componente WASM, avaliação jurídica por fonte.

## Saída
Registrar resultados reais em `docs/VALIDATION.md` e converter esta ADR em aceita ou rejeitada. Não escolher por opinião.
