# Workitems — Renderer WebGL2 da engine Angelica

16 workorders, 64 workitems. Checkboxes não concluídas. Follow
@implementation-loop para código e @architecture-spec-authoring para contratos.
Os JSONs são os contratos do dispatcher; cada WI pertence ao seu WO, sem nova
semântica de agendamento. Executar pelo DAG, não por ordenação numérica.

## [WO-01 — Inventariar estados D3D e escapes usados pela primeira fatia](workorders/wo-01.md)

- [ ] **WI-01** — Partir de A3DDevice/A3DStream e identificar acessos diretos alcançados por uma malha indexada.
- [ ] **WI-02** — Listar vertex formats, texturas, transforms, shader/fixed-function e estados com defaults e call sites.
- [ ] **WI-03** — Separar capacidades necessárias à fatia de água, sombras, UI e efeitos que podem ficar fora dela.
- [ ] **WI-04** — Registrar macros/configuração e prova de alcance; ocorrências textuais não equivalem a uso em runtime.

## [WO-02 — Fechar equivalência de vertex formats, matrizes e estados](workorders/wo-02.md)

- [ ] **WI-01** — Mapear formatos realmente usados para layouts explícitos, offsets, stride e conversão de cores.
- [ ] **WI-02** — Preservar R0 como contrato matemático; documentar winding, profundidade, UV e orientação de textura separadamente.
- [ ] **WI-03** — Definir tabelas blend/cull/depth/alpha-test com defaults e UnsupportedFeature para casos não cobertos.
- [ ] **WI-04** — Criar fixtures e valores esperados independentes da implementação para cada mapeamento aprovado.

## [WO-03 — Implementar P2: primeiro desenho indexado GLES3/WebGL2](workorders/wo-03.md)

- [ ] **WI-01** — Criar contexto WebGL2 e buffers GLES3 pelo Emscripten, sem emular D3D/Win32.
- [ ] **WI-02** — Aplicar VertexPC/matriz/ARGB do R0 e estados fixos; validar DrawView antes de upload.
- [ ] **WI-03** — Desenhar triângulo simples e sobreposição de profundidade; verificar pixels interiores com tolerância R0.
- [ ] **WI-04** — Gerar harness local em artifacts/render-probe, relatório JSON e falha explícita por shader/contexto/input.

## [WO-04 — Integrar materiais, texturas, transparência e render targets](workorders/wo-04.md)

- [ ] **WI-01** — Compor capacidades aprovadas numa pequena cena sintética sem duplicar implementações de WOs anteriores.
- [ ] **WI-02** — Garantir binding/ordem de estados e ownership de material/textura/framebuffer ao trocar draws.
- [ ] **WI-03** — Exercitar unload/reload e context loss sem usar handles antigas.
- [ ] **WI-04** — Registrar imagem/contadores e cobertura do subconjunto; flags fora dele continuam unsupported.

## [WO-05 — Homologar paridade visual contra baseline Windows autorizada](workorders/wo-05.md)

- [ ] **WI-01** — Usar a mesma cena, câmera, tempo e recursos identificados no Windows e no browser.
- [ ] **WI-02** — Comparar pontos/cores/profundidade e regiões interiores; separar tolerância de driver de erro semântico.
- [ ] **WI-03** — Registrar hardware, browser, hashes, capturas autorizadas e divergências remanescentes.
- [ ] **WI-04** — Liberar e07 somente quando o subconjunto acordado passar; não confundir renderer procedural com renderer original.

## [WO-06 — Definir matriz de capacidades suportadas e erros do backend](workorders/wo-06.md)

- [ ] **WI-01** — Enumerar operações R0 e extensões previstas, com versão e UnsupportedFeature para cada ausência.
- [ ] **WI-02** — Definir validação de requests antes de mutar estado GPU e limites consultados no contexto.
- [ ] **WI-03** — Relacionar capabilities a fixtures e tarefas donas sem prometer suportar todo DirectX.
- [ ] **WI-04** — Documentar fluxo para pedido de feature desconhecida, contexto perdido e recurso inválido.

## [WO-07 — Especificar R1 de textura RGBA8 e sampler mínimo](workorders/wo-07.md)

- [ ] **WI-01** — Definir textura RGBA8 própria, dimensões, stride, orientação, alinhamento de upload e tamanho exato.
- [ ] **WI-02** — Definir UV e sampler mínimo com filtros/wrap aceitos; excluir DDS/compressão enquanto sem contrato.
- [ ] **WI-03** — Definir ownership/update/dispose e atualização da capability matrix.
- [ ] **WI-04** — Escrever fixtures 2x2 de cores distintas e casos inválidos para provar UV/orientação.

## [WO-08 — Implementar upload e desenho texturizado de R1](workorders/wo-08.md)

- [ ] **WI-01** — Implementar RGBA8 e sampler aprovados sem converter formatos desconhecidos.
- [ ] **WI-02** — Validar dimensões/stride/byte length antes de alocar ou enviar à GPU.
- [ ] **WI-03** — Desenhar fixture 2x2 com orientação/UV definidos e verificar todos os quadrantes.
- [ ] **WI-04** — Liberar textura no owner e testar update/dispose conforme R1.

## [WO-09 — Implementar blend, alpha test, cull e depth do subconjunto aprovado](workorders/wo-09.md)

- [ ] **WI-01** — Implementar apenas combinações enumeradas no mapping de WO-02, incluindo defaults explícitos.
- [ ] **WI-02** — Traduzir alpha test para shader quando definido e manter transparência separada de depth write.
- [ ] **WI-03** — Desenhar fixtures de ordem, winding, recorte alpha e profundidade com resultados numéricos/pixel.
- [ ] **WI-04** — Testar troca de estados entre draws e rejeição de enums/combinações fora da tabela.

## [WO-10 — Especificar render targets e attachments da primeira fatia](workorders/wo-10.md)

- [ ] **WI-01** — Definir formatos de cor/depth, dimensão, limites, binding e ownership do framebuffer.
- [ ] **WI-02** — Enumerar transições janela/offscreen, clear, resize e readback com origem e alinhamento.
- [ ] **WI-03** — Definir falha por incompletude/limite e rollback de alocação parcial.
- [ ] **WI-04** — Criar fixture offscreen pequena e cenários de resize/context loss sem conteúdo do jogo.

## [WO-11 — Implementar variantes GLSL explícitas do material mínimo](workorders/wo-11.md)

- [ ] **WI-01** — Implementar somente variantes unlit/cor/textura/alpha enumeradas no mapping e R1.
- [ ] **WI-02** — Usar chave determinística de variantes e validar combinações antes de compilar shader.
- [ ] **WI-03** — Propagar erro de compile/link e liberar objetos parciais; não inventar tradutor geral de shaders D3D.
- [ ] **WI-04** — Compilar e desenhar cada variante válida, além de combinação inválida, em WebGL2 real.

## [WO-12 — Implementar framebuffer, resize e readback conforme R1](workorders/wo-12.md)

- [ ] **WI-01** — Implementar criação/binding/clear/readback dos attachments acordados.
- [ ] **WI-02** — Verificar completude e limites antes de promover recurso a Ready.
- [ ] **WI-03** — Fazer resize com ownership claro, liberando recurso antigo somente após criação válida.
- [ ] **WI-04** — Testar alternância tela/offscreen e falha no meio da alocação sem vazamento.

## [WO-13 — Fechar dependências do primeiro draw original da Angelica](workorders/wo-13.md)

- [ ] **WI-01** — Selecionar entrada mínima real de mesh/stream que evite inicialização de UI, áudio e rede.
- [ ] **WI-02** — Rastrear pelo compilador todos os includes, símbolos, libs e chamadas diretas D3D do caminho.
- [ ] **WI-03** — Relacionar cada dependência a implementação portátil, adapter necessário, middleware ou bloqueio.
- [ ] **WI-04** — Fixar lista de arquivos, cenário de comparação e autorização necessária para WO-14.

## [WO-14 — Adaptar o primeiro draw original para o backend browser](workorders/wo-14.md)

- [ ] **WI-01** — Confirmar autorização específica e área de port aprovada antes de incluir qualquer código de referência.
- [ ] **WI-02** — Adaptar somente arquivos/chamadas da closure aprovada, mantendo references congeladas.
- [ ] **WI-03** — Converter dados para a fronteira sem mudar semântica de matriz/cor e fechar acessos diretos D3D usados.
- [ ] **WI-04** — Comparar o mesmo draw no baseline Windows e no browser, registrando capabilities ausentes.

## [WO-15 — Implementar restauração de contexto e descarte de recursos](workorders/wo-15.md)

- [ ] **WI-01** — Completar estados R0 Uninitialized/Ready/Lost e dispose idempotente.
- [ ] **WI-02** — No context loss impedir draws e invalidar handles; restaurar somente com recriação explícita de recursos.
- [ ] **WI-03** — Testar perda/restauração pelo mecanismo do browser e ausência de reuso de IDs antigos.
- [ ] **WI-04** — Conferir primeiro frame após restore e contadores de recursos após ciclos repetidos.

## [WO-16 — Medir renderer e preparar gate visual com evidência reproduzível](workorders/wo-16.md)

- [ ] **WI-01** — Definir cena, quantidade de draws/recursos, resolução e intervalo de amostragem reproduzíveis.
- [ ] **WI-02** — Medir frame time, uploads e memória observável sem fixar metas inventadas nem otimizar antes de medir.
- [ ] **WI-03** — Registrar GPU/browser/driver e distinguir SwiftShader de hardware; medir perda/recuperação de contexto separadamente.
- [ ] **WI-04** — Entregar recomendações baseadas em dados para batching/cache e lacunas para gate WO-05.

## Tarefas humanas e gate

(HUMAN) Liberar integração do legado e recursos da baseline (WO-14/05). R0 sintético avança sem assets comerciais; paridade original não.

Todos os WOs precisam de evidência homologada; não preencher completed-workorders.json
por inferência da POC. Nenhuma aprovação passada é automaticamente prova da revisão 1.1.
