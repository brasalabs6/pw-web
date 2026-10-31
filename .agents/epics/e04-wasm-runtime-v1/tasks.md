# Workitems — Toolchain C++/Emscripten e runtime WebAssembly

10 workorders, 40 workitems. Checkboxes não concluídas. Follow
@implementation-loop para código e @architecture-spec-authoring para contratos.
Os JSONs são os contratos do dispatcher; cada WI pertence ao seu WO, sem nova
semântica de agendamento. Executar pelo DAG, não por ordenação numérica.

## [WO-01 — Fixar Emscripten e builds reproduzíveis de prova](workorders/wo-01.md)

- [ ] **WI-01** — Fixar e documentar Emscripten 6.0.13 usado pela POC e requisitos de C++/Node/browser.
- [ ] **WI-02** — Criar preflight sem instalar pacotes do sistema ou alterar workflows; emcc/em++ ausente retorna erro.
- [ ] **WI-03** — Registrar flags debug/release, ambiente web versus node e hashes/versões dos artefatos próprios.
- [ ] **WI-04** — Executar core existente nos dois modos e documentar como ativar SDK fora do repositório.

## [WO-02 — Implementar binding ABI com typed buffers e erros explícitos](workorders/wo-02.md)

- [ ] **WI-01** — Implementar apenas operações/exports fechados no contrato runtime-v0, preservando exports atuais do core.
- [ ] **WI-02** — Validar offsets, comprimentos, alinhamento e ownership antes de acessar a memória compartilhada do módulo.
- [ ] **WI-03** — Converter códigos de erro para resultados do host sem capturar exceções como sucesso.
- [ ] **WI-04** — Executar chamadas válidas/invalidas no WASM real e verificar ausência de escrita fora do output autorizado.

## [WO-03 — Implementar loop cooperativo com pause, resume e dispose](workorders/wo-03.md)

- [ ] **WI-01** — Implementar estados e limite de delta definidos em runtime-v0 usando requestAnimationFrame.
- [ ] **WI-02** — Fazer um tick C++ limitado por frame; retornar controle ao browser sem Sleep/busy loop.
- [ ] **WI-03** — No pause/resume, resetar timestamp e impedir agendamento duplicado; dispose cancela callbacks.
- [ ] **WI-04** — Testar sequência determinística de frames e execução real de visibilitychange no browser.

## [WO-04 — Medir heap, exceções e requisitos de threads sem habilitá-las](workorders/wo-04.md)

- [ ] **WI-01** — Medir memória inicial, crescimento e retenção num cenário sintético com tamanho de entrada registrado.
- [ ] **WI-02** — Distinguir memória linear, heap JS e recursos GPU quando disponíveis; não inventar VRAM via heurística.
- [ ] **WI-03** — Verificar política de exceções/abort e a necessidade real de Workers/COOP/COEP para a fatia.
- [ ] **WI-04** — Registrar decisão de manter single-thread e gatilho mensurável para revisitar, sem mudar headers de deploy.

## [WO-05 — Homologar testes WASM e paridade nativo/browser](workorders/wo-05.md)

- [ ] **WI-01** — Consolidar smoke do core, bridge, lifecycle, heap views e casos de falha com contagem não nula.
- [ ] **WI-02** — Separar suite pública sem referências do probe opt-in ARandomGen; reaproveitar POC como evidência parcial.
- [ ] **WI-03** — Reproduzir comparações e divergência injetada, incluindo browser/software/hardware usados.
- [ ] **WI-04** — Emitir gate de runtime com SHA, comandos e capacidades não testadas sem declarar cliente portado.

## [WO-06 — Especificar ABI, lifecycle e erros do runtime browser](workorders/wo-06.md)

- [ ] **WI-01** — Definir operações públicas mínimas, tipos fixos, export names e ownership com compatibilidade dos exports existentes.
- [ ] **WI-02** — Fechar estados loading/ready/paused/failed/disposed e transições de retry explícito sem loops automáticos.
- [ ] **WI-03** — Definir unidade ms, regra de delta, clamp/erro, memory.grow, views e shutdown com casos esperados.
- [ ] **WI-04** — Produzir fixtures de contrato para dados válidos, truncados, init duplicada e módulo indisponível.

## [WO-07 — Recriar views tipadas após crescimento de memória](workorders/wo-07.md)

- [ ] **WI-01** — Identificar troca do ArrayBuffer da memória linear antes de acessar views retidas.
- [ ] **WI-02** — Reconstruir as views segundo o contrato e validar novamente intervalos/alinhamento.
- [ ] **WI-03** — Exercitar crescimento no WASM real, sem monkey-patch de HEAP ou dependência de detalhes privados do emscripten.
- [ ] **WI-04** — Testar conteúdo antes/depois de grow e erro de allocation/limite previsto pelo contrato.

## [WO-08 — Implementar bootstrap assíncrono com falha visível](workorders/wo-08.md)

- [ ] **WI-01** — Carregar módulo antes de liberar consumidor, com estados e ownership do contrato runtime.
- [ ] **WI-02** — Distinguir 404, erro de rede, compilação WASM, ABI incompatível e abort de inicialização.
- [ ] **WI-03** — Prevenir init concorrente duplicada e descartar resultado tardio após dispose.
- [ ] **WI-04** — Verificar rede real local para sucesso/404 e falha explicitamente apresentada ao host.

## [WO-09 — Propagar abort e liberar recursos na falha do runtime](workorders/wo-09.md)

- [ ] **WI-01** — Unificar parada de frame loop, listeners e módulo segundo ownership aprovado.
- [ ] **WI-02** — Propagar abort/erro fatal ao estado failed; não continuar desenhando como se houvesse simulação.
- [ ] **WI-03** — Garantir dispose idempotente e recuperação somente por nova inicialização explícita.
- [ ] **WI-04** — Testar falhas em cada etapa de startup e depois do primeiro frame, sem adicionar APIs de produção só para teste.

## [WO-10 — Automatizar smoke browser com cenários de sucesso e falha](workorders/wo-10.md)

- [ ] **WI-01** — Criar runner browser local com ferramenta disponível ou dependência mínima justificada; registrar versão exata.
- [ ] **WI-02** — Exigir import/execução WASM, resultado numérico, estado final e ausência de erros inesperados.
- [ ] **WI-03** — Cobrir missing wasm, baseline adulterada, init duplicada, pause/resume e dispose.
- [ ] **WI-04** — Emitir JSON de resultado e exit code !=0 para falha/timeout/ferramenta ausente; não aceitar screenshot isolada.

## Tarefas humanas e gate

(HUMAN) Provisionar ferramentas no ambiente de execução se ausentes; mudanças de workflow/provider continuam fora das scopes destes jobs.

Todos os WOs precisam de evidência homologada; não preencher completed-workorders.json
por inferência da POC. Nenhuma aprovação passada é automaticamente prova da revisão 1.1.
