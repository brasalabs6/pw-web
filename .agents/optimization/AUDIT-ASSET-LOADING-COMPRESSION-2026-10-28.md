# Auditoria incremental — loading, arquivos, assets, compressão e entrega do PW 1.5.3

**Data:** 2026-10-28. **Repo:** `brasalabs6/pw-web`. **HEAD inspecionado:** `6178b0f224cda80fa0173ceb9d7f92b3a7d81d8c`. **PR:** [#23](https://github.com/brasalabs6/pw-web/pull/23). **Fase:** estudo estático pós-port; foco imediato permanece WASM funcional no desktop e mobile.

## Limites da evidência
Este repositório contém leitores e ferramentas de asset, mas nesta sessão **não foi fornecido um diretório original autorizado de cliente com os PCK, PKX, DDS, MOX e outros recursos para inventariar**. Portanto **não há dados verificados de GB totais, p95 de tamanho, taxa real de compressão, savings, TTFP, upload GPU, VRAM ou FPS**. Não transformar hipótese em número. Não houve execução em dispositivo, benchmarking, conversão nem extração de assets; nenhum subagente foi lançado.

## Pipeline observado
`element/*.pck [+.pkx] -> tools/assets/pck153.py -> scripts/prepare-client153-scene.py -> (MOX + DDS hash filenames + scene.json) -> web/client153/app.mjs -> fetch/sha256 -> Emscripten VFS + load_file -> texture upload -> viewer WebGL2`.

**Separação importante:** esse fluxo é o viewer diagnóstico atual, **não** o game completo com login, mundo, combate e assets online. A estratégia futura de CDN/R2/cache/chunks está especificada em `docs/CLOUDFLARE-DELIVERY.md`, mas não deve ser tratada como pipeline funcional implantado. O cliente Angelica original tem caminho separado `AFileImage/AFilePackGame/AFilePackage`, e o carregamento de cena `CECScene/CECWorld` tem suas próprias filas/threads.

## Achados novos e a pergunta mensurável de cada um

| ID | Recorte específico (deduplicado) | Métrica de validação |
|---|---|---|
| [OPT-029](OPT-029-pck-entry-size-compression-inventory.md) | Medir peso e compressibilidade de cada asset dentro dos PCK/PKX | Rodar em cliente 1.5.3 autorizado com e sem PKX |
| [OPT-030](OPT-030-viewer-scene-stage-select-only-used-dds.md) | Evitar extração de DDS irmãs não referenciadas pela cena MOX | Cenas sintéticas com 2 DDS usadas e 20 irmãs extras |
| [OPT-031](OPT-031-pck-catalog-open-many-small-reads.md) | Reduzir custo de abertura do índice PCK com milhares de entradas | Fixtures PCK com 100, 10 mil e 100 mil registros, 0/50/100% comprimidos, PCK/PKX e índice truncado |
| [OPT-032](OPT-032-game-pck-shared-open-redecode-lifetime.md) | Avaliar reabertura e descompressão repetida de arquivos compartilhados no AFilePackGame | Abrir 1/10/100 vezes a mesma entrada sob leitura concorrente/serial |
| [OPT-033](OPT-033-browser-dependency-fetch-priority-pipelining.md) | Pipelinar dependências da cena Web com concorrência e prioridades limitadas | 20/100 DDS distintas sob RTT 25/100/300 ms e rede 1/5/30 Mbps |
| [OPT-034](OPT-034-pck-codec-policy-by-content-type.md) | Comparar compressão seletiva por tipo de asset sem alterar assets originais | Amostra autorizada representativa de texturas, malhas, UI e arquivos de configuração. Relatar bytes comprimidos/dec |
| [OPT-035](OPT-035-scene-block-loading-byte-time-budget.md) | Substituir política de loading de cena baseada só em número de blocos | Travessia cidade/campo, teleporte rápido e retorno, blocos com 1/1000 objetos, velocidades 1/2/8 blocos: p95/p99 lo |
| [OPT-036](OPT-036-pck-read-short-read-integrity-contract.md) | Verificar leitura completa e limites de payload no leitor PCK legado | Fixtures PCK de payload curto, PKX ausente, truncate entre header e data, falhas de seek simuladas, byte count 0 e  |
| [OPT-037](OPT-037-pck-replacement-append-fragmentation.md) | Evitar crescimento histórico de PCK por substituições append-only no pipeline de build | Criar fixture sintética com 1 arquivo substituído 10/100 vezes, medir tamanho físico vs soma de entradas vivas, rou |
| [OPT-038](OPT-038-asset-inventory-non-pck-coverage.md) | Incluir assets fora de element/*.pck na análise de peso e prioridades de download | Fixture com DDS avulsa, script, áudio e PCK/PKX |

## Análise de tamanho e compressão — sem números inventados

O `inventory-client153-assets.py` já gera hashes de PCK/PKX completos e contagens por extensão, mas não produz bytes armazenados/descomprimidos de cada entrada, nem inclui avulsos. O objeto `pck153.Entry` já expõe `size`, `stored_size`, `offset`. Um relatório de inventário autenticado deveria calcular:

- `packed_bytes`: tamanho físico de .pck e .pkx; `entry_stored_bytes`: soma do campo de entrada efetivamente armazenado; `entry_logical_bytes`: soma de bytes originais; `container_overhead_and_dead_bytes`: diferença com ressalvas sobre headers, índice e payload obsoleto.
- Percentis e top N por extensão, família de asset, região e dependência de login/primeiro spawn. Separar DDS compactada, DDS BGRA, malha, animação, SGC/GFX, UI, áudio, scripts e dados de mapa. Não misturar bytes de GPU com bytes de PCK.
- `compression_ratio = entry_stored_bytes / entry_logical_bytes` quando denominador não zero, inclusive observação de entradas de comprimento zero. `download_bytes` são medidos na rede e não iguais automaticamente ao payload de PCK.
- Custo frio/quente por etapa: network, SHA-256, PCK decode, parser MOX/DDS, VFS/WASM, GPU upload e primeiro frame, em Chrome desktop e Android com instrumentação.

## Testes de orçamento futuro (propostas, não execução)

| Cenário | Rede simulada | Instrumentação |
|---|---|---|
| Primeira instalação/cold | RTT 25/100/300ms e 1/5/30 Mbps | first render, critical bytes, JS/WASM heap peak |
| Retorno/warm cache | cache quente/evicted parcial | re-download, hashes, I/O, quotas |
| Cliente com pacote grande | PCK 0, 10k, 100k entradas sintéticas | directory init, allocations, bytes/seek, decoder p95 |
| Streaming do mundo | campo/cidade/teleporte/volta | scene blocks, prioritized bytes, p99 long tasks |
| Mudança de versão | bundles hash iguais/alterados, rollback | bytes transferidos, consistência, cache GC seguro |
| Arquivo corrompido | truncated PCK/PKX, partial network, checksum mismatch | erro determinístico, nenhum conteúdo não verificado aplicado |

## Deduplicação realizada
- `PW23-020` segue sendo a lacuna **funcional** ampla de streaming/CDN: não abrir outra issue.
- `PW23-024` cobre cópias transitórias na passagem browser->WASM: não recriar uma OPT para o mesmo problema.
- `PW23-027` cobre streaming **A3DTerrain2**; `OPT-035` trata **CECScene** e o mecanismo de loading/progresso.
- `PW23-032` já cobre URLs, timeouts, quantidade/tamanho e retry como hardening de trust: `OPT-033` se limita ao **waterfall sequencial e planejamento do primeiro frame**.
- `OPT-002` cobre arquivos e uploads DDS duplicados por mesh; `OPT-030` cobre **arquivos selecionados no cooker de exemplo**, não os downloads repetidos.
- `OPT-003` cobre zlib e buffers de payload no momento de cada `ReadFile`; `OPT-031` cobre indexação no `Open`; `OPT-032` cobre payload aberto repetidas vezes por múltiplos consumidores; `OPT-034` cobre codec no momento de empacotar.
- `OPT-014` já cobre DDS parsing/repacking e ownership; não tocar codecs de textura sem testes de aparência.
- A qualidade e compatibilidade do cliente v145 versus formatos empacotados deve ser validada, sem deduzir suporte de versões apenas por ter código de parsing.

## Precedência executável
1. **Agora:** concluir port do jogo e o caminho autorizado mínimo de assets usado no boot (não introduzir tuning). Usar ferramentas existentes de inventário e smoke apenas quando dados autorizados estiverem disponíveis.
2. **Assim que houver assets reais e jogo funcional:** extrair inventário de metadados e traces de cold start para orientar o que vale estudar.
3. **Primeira otimização de loading após baseline:** staging seletivo/TTFP/concurrency, catálogo PCK, cenas e retenção de heap. Priorizar por p95 e MB medidos; não adotar compressão nova por suposição.
4. **Entrega escalável:** sharding/content addressing/CDN/versioning/cache GC conforme e15/e17; validar hashes, licença, contexto mobile e rollback.

**Proteção de escopo:** nenhum source file alterado, PR #23 continua único do cliente, PR #17 do servidor intocado, nenhuma recomendação substitui port primeiro.
