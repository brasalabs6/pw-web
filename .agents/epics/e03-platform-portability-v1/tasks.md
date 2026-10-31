# Workitems — Desacoplamento de Win32, SDK e Direct3D

12 workorders, 48 workitems. Checkboxes não concluídas. Follow
@implementation-loop para código e @architecture-spec-authoring para contratos.
Os JSONs são os contratos do dispatcher; cada WI pertence ao seu WO, sem nova
semântica de agendamento. Executar pelo DAG, não por ordenação numérica.

## [WO-01 — Inventariar includes, APIs e escapes de plataforma por módulo](workorders/wo-01.md)

- [ ] **WI-01** — Usar as entradas dos projetos ativos; separar cópias Nova pasta, backups e arquivos excluídos por configuração.
- [ ] **WI-02** — Mapear Windows/D3D/ATL/sockets/asm e GetD3DDevice com arquivo:linha e macro de configuração.
- [ ] **WI-03** — Distinguir busca lexical de grafo de compilação; registrar dependências transversais e limites do scanner.
- [ ] **WI-04** — Testar classificação com fixture própria e produzir mapa da primeira fatia sem modificar referências.

## [WO-02 — Fechar contratos de plataforma e erros sem stubs de sucesso](workorders/wo-02.md)

- [ ] **WI-01** — Definir relógio monotônico em ms, input por frame, leitura de bytes e ownership de buffers para a fatia.
- [ ] **WI-02** — Especificar estados, unidades, limites e erros para cada operação realmente necessária; excluir APIs sem consumidor.
- [ ] **WI-03** — Preservar R0 como fonte única de tipos/matrizes; documentar tratamento de locales e recursos indisponíveis.
- [ ] **WI-04** — Walkthrough de pause/resume, arquivo ausente e cancelamento; listar operações bloqueadas em vez de fake implementations.

## [WO-03 — Integrar adapters portáteis de tempo, caminhos e leitura em memória](workorders/wo-03.md)

- [ ] **WI-01** — Conectar adapters já entregues a um cenário sintético de carregamento e tick, sem reimplementá-los.
- [ ] **WI-02** — Respeitar ownership e política de alocação aprovada; o core não inclui headers Windows.
- [ ] **WI-03** — Propagar erros de arquivo e relógio até o caller com contexto sem expor dados privados.
- [ ] **WI-04** — Rodar o mesmo cenário nativo e WASM; registrar diferenças e atualizar o mapa de fronteiras.

## [WO-04 — Integrar input e scheduler sem dependências Win32 no core](workorders/wo-04.md)

- [ ] **WI-01** — Conectar snapshot de input ao tick cooperativo do runtime, preservando as unidades do contrato.
- [ ] **WI-02** — Separar handlers DOM da simulação C++; nenhuma mensagem Windows entra no core.
- [ ] **WI-03** — Limpar estado de teclas ao perder foco e impedir múltiplos loops após resume.
- [ ] **WI-04** — Documentar threads/áudio fora da fatia e validar que não há Sleep/polling bloqueante nesse caminho.

## [WO-05 — Homologar portabilidade e limites Win32 remanescentes](workorders/wo-05.md)

- [ ] **WI-01** — Executar os testes numéricos R0, adapters e integração frame/input nas duas plataformas.
- [ ] **WI-02** — Comparar inventário inicial e closure atual, listando precisamente o código ainda Windows-only.
- [ ] **WI-03** — Conferir que nenhum stub, cast ou macro apagou uma funcionalidade requerida pela fatia.
- [ ] **WI-04** — Publicar resultado de gate com comandos e limites; não declarar portabilidade de todo o cliente.

## [WO-06 — Implementar P1: ABI fixa, matriz clip e cor do contrato R0](workorders/wo-06.md)

- [ ] **WI-01** — Implementar exatamente VertexPC, Matrix4, Clip4, Viewport, DrawView e RenderStatus de R0, com static_assert sizeof/offsetof.
- [ ] **WI-02** — Escrever RED para as fixtures de identidade, translação, profundidade, perspectiva e ARGB já definidas; implementar to_webgl_clip/argb_to_rgba.
- [ ] **WI-03** — Validar contagens, índices, ponteiros não nulos e finitude; preservar output em erro e distinguir ABI interna de wire.
- [ ] **WI-04** — Criar runner C++17 nativo e em++/Node para a mesma suite; ausência de ferramenta ou zero casos deve falhar.

## [WO-07 — Implementar adapter de relógio monotônico injetável](workorders/wo-07.md)

- [ ] **WI-01** — Implementar a assinatura/unidade aprovada em platform-v0, mantendo relógio do host fora do core.
- [ ] **WI-02** — Usar timestamps determinísticos nos testes; tempo de parede não governa simulação.
- [ ] **WI-03** — Tratar timestamp regressivo, não finito e reset conforme o contrato, sem delta negativo silencioso.
- [ ] **WI-04** — Comparar sequência nativo/WASM e documentar precisão e limite de conversão.

## [WO-08 — Implementar validação de caminhos lógicos e fronteira de locale](workorders/wo-08.md)

- [ ] **WI-01** — Implementar caminhos lógicos UTF-8 conforme platform-v0; nunca aceitar path do host como identidade de asset.
- [ ] **WI-02** — Rejeitar traversal, path absoluto, drive/UNC, NUL e nomes acima do limite aprovado.
- [ ] **WI-03** — Preservar identidade e política de case do contrato; conversão GBK pertence ao pipeline de assets.
- [ ] **WI-04** — Testar vetores válidos, separadores e segmentos adversariais nas duas plataformas.

## [WO-09 — Implementar snapshots de input e reset de foco](workorders/wo-09.md)

- [ ] **WI-01** — Implementar mapa fechado de teclas/botões e coordenadas de canvas aprovado pelo contrato.
- [ ] **WI-02** — Separar eventos DOM de snapshot consumido pelo tick e definir consumo de eventos por frame.
- [ ] **WI-03** — Liberar teclas/botões no blur e remover listeners no dispose; não capturar input fora da área autorizada.
- [ ] **WI-04** — Testar resize/DPR, foco perdido e dispose; anexar execução no browser.

## [WO-10 — Implementar leitura limitada de buffers já carregados](workorders/wo-10.md)

- [ ] **WI-01** — Implementar leitura/seek de bytes já disponíveis, com tamanho e ownership explícitos.
- [ ] **WI-02** — Validar offset e length por subtração segura, antes de memcpy ou avanço do cursor.
- [ ] **WI-03** — Preservar cursor/output em falha e distinguir EOF de leitura completa conforme o contrato.
- [ ] **WI-04** — Testar arquivo vazio, último byte, truncamento e leitura após liberação através do owner válido.

## [WO-11 — Definir ownership e política de alocação para a fatia](workorders/wo-11.md)

- [ ] **WI-01** — Mapear owners dos buffers de arquivo, mesh, renderer e runtime sem criar allocator customizado.
- [ ] **WI-02** — Definir cópia/borrow/lifetime na fronteira wasm/JS e tratamento de falha de alocação.
- [ ] **WI-03** — Proibir retenção de views JS após memory.grow e de ponteiros do caller após consumo síncrono.
- [ ] **WI-04** — Definir casos de teste de dispose repetido, erro parcial e crescimento de memória para e04.

## [WO-12 — Auditar closure portátil com compilação e sanitizers](workorders/wo-12.md)

- [ ] **WI-01** — Compilar os módulos novos com warnings-as-errors e gerar dependências pelo compilador.
- [ ] **WI-02** — Rodar ASan/UBSan nativo nos cenários de buffer, índice e transformação, sem desabilitar diagnósticos para passar.
- [ ] **WI-03** — Comparar includes alcançados com o inventário e registrar qualquer fronteira Windows remanescente.
- [ ] **WI-04** — Guardar comandos e resultado, distinguindo sanitizer indisponível de suite sem erros.

## Tarefas humanas e gate

(HUMAN) Aprovar autorização antes de integrar código legado; P1 e adapters próprios não precisam copiar referências. Threads/áudio sem contrato permanecem fora da fatia.

Todos os WOs precisam de evidência homologada; não preencher completed-workorders.json
por inferência da POC. Nenhuma aprovação passada é automaticamente prova da revisão 1.1.
