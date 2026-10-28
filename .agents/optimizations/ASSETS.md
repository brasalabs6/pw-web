# Assets, streaming e persistência

## Problema
O cliente 1.5.3 original é grande (aproximadamente 9 GB no pacote discutido); tamanho instalado não equivale a download inicial, RAM ou VRAM. Medir inventário real antes de prometer redução.

## Pipeline
1. Inventariar por tipo, tamanho, frequência de uso, dependências, licença e hash.
2. Converter offline formatos originais para pacotes de distribuição versionados; preservar fontes e rastreabilidade.
3. Manifestos assinados ou verificados por hash com dependências, prioridades, variantes de qualidade e chunks.
4. Bootstrap mínimo: UI, login, cena inicial e assets indispensáveis.
5. Streaming por região e prefetch preditivo com prioridades e cancelamento.
6. Compressão e transcodificação de texturas por capacidade (formatos suportados, fallback seguro).
7. Cache Storage/OPFS conforme disponibilidade; política de quota, eviction e recovery.
8. Downloads retomáveis, deduplicação, versionamento atômico e rollback de manifestos.

## Restrições
Persistência de dados no navegador não é garantida: cotas, limpeza pelo SO, navegação privada e pressão de armazenamento podem remover cache. Solicitar persistent storage quando suportado e manter recuperação por rede. Não carregar 9 GB na memória.

## Métricas
TTFI (tempo até interface), TTP (tempo até jogável), bytes até primeira cena, cache hit ratio, uso em disco, RAM/VRAM, stalls de upload e latência p95 de streaming. Testar primeira visita, cache quente, rede lenta, offline parcial e upgrade de versão.
