# Arquitetura — Streaming, CDN, cache OPFS e desempenho

Estado: planejado. O desenho específico de Cloudflare substitui o diagrama
genérico desta área, sem afirmar implementação. Cenário: jogador entra em uma
instância e recebe sua closure de entry; vizinhas/qualidade vêm depois.

`tools/assets` produz bytes/manifests e detém o schema; `web/streaming` resolve
grafo, fila e aborto; `web/storage` possui índice/LRU/leases; `engine/resources`
possui handles vivos de RAM/GPU. Ninguém infere layout de pacote do nome do jogo.

## Contratos

- [Manifest e empacotamento](contracts/content-manifest.md): raiz pequena, catálogos
  lazy, bundles e objetos; SHA, limites e erros; schemas futuros em tools/assets.
- [Streaming/cache/instâncias](streaming.md): estados, prioridades, memória,
  cancelamento, storage negado e múltiplas abas.
- [Release pinada](../e17-security-deployment-v1/contracts/release-manifest.md):
  app e publisher consomem o contrato de conteúdo, não o redefinem.

Métricas: bytes até UI/entry, p50/p95 download/decode/compile, heap/GPU, hits e
requests. Ainda sem medição do cliente completo. V1 usa objetos inteiros pequenos;
Range é evolução com critérios próprios de WO-02, que não está concluída.

[Plano integrado](../../../docs/CLOUDFLARE-DELIVERY.md) e
[tarefas](../e17-security-deployment-v1/delivery-tasks.md).
