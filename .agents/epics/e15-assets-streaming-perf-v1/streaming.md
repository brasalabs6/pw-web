# Streaming, memória e loading por zona — spec v1

Estado: planejado, sem medições do jogo. Produtor do manifesto: tools/assets;
loader: web/streaming; cache: web/storage; handles vivos: engine/resources.
Fonte canônica de dados: [content-manifest](contracts/content-manifest.md).
Semântica de entry no realm continua pertencendo ao servidor/e10–e11.

## Pacotes lógicos

| Conjunto | Conteúdo e momento | Prioridade |
| --- | --- | --- |
| shell | HTML/CSS/JS mínimo, diagnóstico, erro/retry | antes de WASM |
| runtime | JS glue + WASM principal; sem assets embutidos | após validar bootstrap |
| bootstrap/ui | fontes/ícones/interface mínima autorizada | obrigatório para login |
| locale/<id> | apenas idioma escolhido | junto a bootstrap |
| login | fundos/UI e criação/lista de personagem necessários | antes do login visual |
| common/<family> | materiais/animações/skeletons realmente compartilhados | dependência explícita |
| avatar/<archetype>/<lod> | recursos do personagem e equipamentos visíveis | entrada/câmera |
| world/<map>/cell/<cell>/<lod> | terreno, colisão, objetos de uma célula | célula atual, depois vizinhas |
| instance/<id>/entry | spawn, colisão e visibilidade inicial da instância | ao preparar transição |
| instance/<id>/cell/<cell> | restante da instância | sob demanda |
| audio/<region>/<bank> | bancos autorizados da região | após gesto; sem bloquear por música opcional |
| optional/<feature> | cosméticos/qualidade/recursos não essenciais | ocioso, nunca competir com entrada |

Esses IDs são nomenclatura de distribuição, não módulos DLL/WASM. Mapas/células,
archetypes, IDs e dependências reais virão do cooker e do levantamento e05/e07/e08;
a tabela não afirma conhecer os formatos ou quantidades da 1.5.3.

## Bootstrap e transição de instância

1. Shell recebe release imutável pinada; verifica compatibilidade de schema,
   feature set, idioma, capacidades de browser e realm permitido.
2. Baixa runtime e índice; resolve apenas dependências da UI/login. Nunca faz um
   eager fetch de todos os BundleManifests do jogo.
3. Na preparação de mapa, calcula closure de `entry` e prioriza colisão, terreno
   inicial, avatar e recursos obrigatórios antes da aparência de alta resolução.
4. Estado por transição: Preparing → Fetching → Verifying → Decoding →
   AwaitingServer → Active; Failed/Cancelled nunca viram Active. Não anunciar
   readiness de mundo ao servidor antes dos recursos obrigatórios. Handshake e
   confirmação reais são gate do contrato do realm, sem mensagem inventada.
5. Ao cancelar/teleportar outra vez: incrementa generation da transição, aborta
   requests sem consumidores e ignora resultados atrasados da generation anterior.
   Uma conclusão assíncrona antiga não pode reativar mapa descartado.
6. Pin temporário do mundo antigo + entry do novo somente dentro do budget.
   Sem memória para ambos: usar tela de loading, liberar o antigo após protocolo
   permitir e carregar o novo. Não prometer transição sem tela em todo dispositivo.
7. Após Active, prefetch por vizinhança/câmera e possíveis portais. Descartar GPU
   handles sem referências; compartilhar assets comuns por refcount explícito.

## Scheduler e cache

Fila P0 obrigatório entry; P1 visível; P2 previsível; P3 opcional. Deduplicar
requests por hash e compartilhar promise/refcount. Downloads propostos: 4 em
paralelo no desktop, 2 no perfil de memória reduzida; limites configuráveis por
perfil e revisados por benchmark. Três tentativas no máximo por erro transitório,
backoff com jitter e respeito a Retry-After; 401/403 pausa para autenticação,
404/hash inválido falham a release, nunca buscam `latest`. Offline sem conteúdo
completo exibe motivo/retry e não simula entrada no mundo.

Cache OPFS para bytes + IndexedDB para índice/LRU, com fallback IndexedDB quando
OPFS ausente e modo somente memória quando armazenamento negado. persist() é
pedido, não garantia. QuotaExceeded: apagar apenas objetos sem lease/pin, tentar
uma vez e seguir em memória se couber. Separar cache em disco, heap WASM, decode
em flight e GPU: reduzir download não reduz automaticamente RAM/VRAM.

Write temp → checar tamanho/hash → commit metadata Ready → servir; nunca usar
entrada parcial. Startup remove temps órfãos; índice órfão/corrompido é reconstruído
por hash ou descartado. Em múltiplas abas, lease local com owner+expiração, renovado
enquanto usado; Web Locks quando disponível e transação IndexedDB como coordenação.
Evicção revalida lease na mesma transação; lock local não substitui hash.
Fallback sem persistência usa caches por aba, sem fingir coordenação durável.

LRU considera acesso de objetos Ready, exclui pins da sessão/release e de entry.
Budget de cache proposto: min(20% da quota reportada, 1 GiB desktop/256 MiB perfil
reduzido), condicionado ao espaço livre; valores são hipóteses a medir. Não
reservar quota que o navegador não concedeu. Decode grande limitado por bytes em
flight, não só número de workers. Uploads GPU com budget por frame configurado;
operação que ultrapasse budget espera frame seguinte. Recursos obrigatórios não
são descartados por LRU enquanto referenciados.

## Redução de tamanho e testes de aceite

Primeiro medir: bytes de shell/runtime/login/entry, duplicação por conteúdo,
requests, hit ratio, heap/GPU, tempo de decode/compile, first usable UI e world
ready. Candidatos separados: remover debug/símbolos do artefato público, tree
shaking JS, LTO/dead stripping C++, não incluir idiomas/mapas não usados,
LOD/mipmaps e compressão de textura validada nas GPUs alvo. Não reduzir precisão,
remover gameplay ou recomprimir conteúdo já comprimido sem equivalência medida.

Benchmarks propostos (não promessas de FPS/segundos): cache frio/quente; 5 e 50
Mbps, RTT 150 e 30 ms; falha de rede no meio do chunk; storage negado; duas abas;
teleporte A→B→C durante download; GPU context loss; troca de release em background.
Registrar p50/p95 e bytes reais por cenário. O gate inicial exige zero conteúdo
exclusivo de B depois de Active em C, nenhum hash inválido consumido e nenhuma
queda para objetos de release diferente. Meta de tamanho inicial só é fixada
quando houver runtime+entry reais; não inventar MB atuais para o cliente completo.
