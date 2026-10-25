# Pacote de execução — ChatGPT Web / PR #15

Pedido do operador (2026-10-25): o ChatGPT Web escreve e testa a maior parte do
código; Codex prepara dependências com internet e depois revisa, executa WASM/
browser, corrige e integra. Isto é execução manual assistida, não mudança do
executor Grok dos workflows. O objetivo continua sendo o cliente PW 1.5.3 real
no navegador, com renderer local e protocolo; não um clone visual.

## Prompt de entrada para o ChatGPT Web

Trabalhe na branch `feat/angelica-port-method-poc` do repositório
`brasalabs6/pw-web`, PR #15. Leia o HEAD atual antes de editar. Não substitua a
branch por um snapshot antigo. Preserve trabalhos paralelos e mantenha draft.
Se não puder escrever no repositório, entregue patches unificados aplicáveis ao
SHA que você leu, com arquivos completos para arquivos novos e comandos de teste.
Se não conseguir ler o repo, peça ao operador o snapshot desta branch; não
reconstrua o projeto de memória.

Leia `AGENTS.md`, `.agent/skills/create-pr/SKILL.md`, `docs/LEGAL.md`,
`docs/ANGELICA-PORTING-METHOD.md`, `docs/VALIDATION.md` e
`.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md`. Os contratos e as
workorders existentes prevalecem sobre exemplos desta mensagem. Não reescreva o
planejamento WIP de e02–e06 nem altere o catálogo/scheduler para marcar progresso.

Não pressuponha internet, emcc, Node, Chrome, Playwright, VS2005 ou submódulos
presentes. Faça inventário de ferramentas primeiro. **Ausência de ferramenta é
BLOCKED/NOT RUN para aquele teste, nunca PASS.** Continue as tarefas independentes
com contratos completos; não baixe dependências nem invente substitutos. Fontes
em gitlinks não fazem parte de um checkout offline comum.

## Estado entregue

- POC-001 anterior: `ARandomGen` original 1.5.3 recompilado para WASM e comparado
  no browser; não é renderer original nem gameplay.
- P1: `engine/render/clip.{h,cpp}`, `tests/render_clip.cpp`,
  `scripts/test-render-clip.sh`; 14 grupos passaram nativos, também com ASan/UBSan.
  `scripts/verify.sh` agora inclui esses testes nativos.
- Integração com a branch remota: **31 testes Python + core C++ + 14 grupos R0
  passaram localmente**. Estes resultados não são CI do novo commit.
- WASM de P1: ainda não executado neste estado; SDK oficial 6.0.13 em download
  no ambiente Codex. Não conte com esse download no seu ambiente.
- P2: `tests/probes/render.html` e `scripts/build-render-probe.sh` existem, mas
  **`engine/platform/web/render_probe.cpp` ainda não existe**. Harness preparado
  não equivale a backend implementado ou pixel comprovado.
- Baseline Windows, primeiro draw original, assets, login/mundo e multiplayer
  continuam sem prova. Evidência detalhada: `docs/evidence/render-r0.md`.

## Tarefas para implementar agora, em ordem

### WEB-01 — confirmar P1 e sua integração

- Leia a implementação e compare com R0; não a reescreva sem defeito demonstrado.
- Execute `./scripts/verify.sh`; execute `./scripts/test-render-clip.sh` apenas
  com C++, em++ e Node disponíveis. Registre resultados separados por alvo.
- Se encontrar bug, primeiro acrescente a regressão que falha e então corrija.
  Preserve saída em erro, limites 65536 vértices/196608 índices, viewport real,
  matriz row-major e ARGB por shifts; não use DWORD/unsigned long na fronteira.
- Atualize o estado factual de P1 em `docs/evidence/render-r0.md`, no método e no
  contrato R0; não declarar paridade com o renderer original.

### WEB-02 — implementar P2: GLES3/WebGL2 em C++/WASM

Dono: novo `engine/platform/web/render_probe.cpp`, com headers auxiliares apenas
se reduzirem complexidade real; script e harness P2 existentes são consumidores.
Use C++17, GLES3/Emscripten, sem camada de emulação Win32/D3D e sem ponte JSON de draws.

- Crie contexto WebGL2 com antialias desabilitado e depth buffer; consulte limites
  reais do contexto. Inicialização só promove Ready depois de shaders, programa,
  VAO e buffers válidos. Libere alocações parciais em qualquer erro.
- Implemente shaders explícitos de posição/cor: valores row-major enviados com
  `transpose=false`, multiplicação GLSL correta e clip `[x,y,2*z-w,w]`. Extraia
  canais ARGB corretamente; não reinterpretar seus bytes como RGBA.
- Antes de upload/draw, chame `validate_draw` de P1; consuma arrays sincronamente.
  Triangle list + uint16, depth LEQUAL/write/clear 1, blend/cull desligados.
  Textura/luz/fog/skinning/alpha-test ausentes retornam UnsupportedFeature.
- Implemente estados Uninitialized/Ready/Lost, callbacks reais de perda e
  restauração, draw perdido => ContextLost, reinicialização explícita após
  restauração e dispose idempotente. Recrie objetos; não use objetos GL antigos.
- Propague erro de shader/link/GL; nunca retorne Ok sem execução. Diagnósticos
  devem identificar operação/limite sem conteúdo privado.
- Exports do harness: `probe_init()`, `probe_draw(int fixture)`, `probe_clear()`,
  `probe_pixel(int x,int y)` e `probe_dispose()`. Use retornos RenderStatus de R0
  (Ok=0, InvalidInput=1, UnsupportedFeature=2, ContextLost=3, GpuError=4).
  `probe_pixel` retorna uint32 RGBA empacotado R<<24|G<<16|B<<8|A; é API de teste,
  não exportar DrawView/ponteiros arbitrários para JS/rede.
- Fixtures de `probe_draw`: 0 triângulo branco; 1 vermelho perto depois verde
  longe; 2 ordem inversa; 3 índice inválido [0,1,3]; 4 viewport zero; 5 pedido
  de textura; 6 cor 0x80402010; 7 triângulo branco transladado +0.5 em X.
  Dimensão do canvas 64x64; coordenadas da fixture R0; perto z=.25, longe z=.75.
  Erro não limpa nem altera o framebuffer. `probe_clear` limpa preto opaco/depth1.
- Preserve os 10 grupos do HTML: centro/canto, profundidade nas duas ordens,
  ARGB, translação, rejeição sem draw, loss/restore e dispose/reinitialize.
  Leia o HTML para coordenadas e valores exatos, inclusive pixels 48,32 e 20,32.
- Corrija o harness para registrar FAIL também em falha de import/inicialização,
  não ficar RUNNING indefinidamente. Preserve controle negativo `?negative=1`.
- Aceite: build `./scripts/build-render-probe.sh`; browser reporta exatamente os
  grupos esperados e pixels com tolerância <=2/canal; negativo reporta FAIL.
  Sem browser disponível, entregue código/testes e marque execução BLOCKED.

### WEB-03 — automatizar a prova P2 positiva e negativa

Dono: novo teste browser de R0 e script opt-in; reuse o padrão de
`tests/probes/browser-proof.cjs` e `scripts/probe-angelica-browser.sh`, sem alterar
ou chamar a prova original para fingir cobertura do renderer.

- Reuse `playwright-core` instalado externamente por `PW_BROWSER_MODULE_DIR` e
  Chrome por `CHROME_BIN`; não instalar pacotes automaticamente nem adicionar
  dependência de produção. Falta de ferramentas/artefatos precisa falhar claramente.
- Sirva somente localhost, em porta própria disponível (8482 para R0); não usar
  8080, não tomar processo existente. Encerre só o servidor criado pelo script.
- Execute caso positivo e `?negative=1`, exija JSON válido, contagem exata,
  pixels, ausência de erros JS e detecção do negativo. Não aceitar simples
  screenshot, HTTP200 ou palavra PASS solta como prova.
- Grave resultado/versão browser/renderer/screenshot em `artifacts/render-probe/`,
  ignorado pelo Git. Informe SwiftShader como software, não benchmark GPU física.
- Acrescente testes relevantes do runner (inclusive saída incompleta/falha) e
  mantenha a prova de source/asset opt-in fora do bootstrap/CI público.

### WEB-04 — revisar a entrega e preparar a próxima fronteira gráfica

- Revise o diff P1/P2 contra R0: erro antes de upload, lifetime, partial init,
  restauração explícita, cleanup, packing, matemática e falsos positivos do teste.
- Execute checks disponíveis e registre limitações. Não marque workorders como
  concluídas por inferência nem altere `completed-workorders.json` sem seus gates.
- Após P2, desenvolva **a especificação R1 e suas fixtures** nos contratos de e06
  WO-07/09/10/11: RGBA8 2x2, dimensões/stride/byte length, UV/origem, sampler,
  filtros/wrap, alpha/blend/cull/depth e ownership/update/dispose/context loss.
  Escolha só o subconjunto necessário e enumere erros. Não implementar formato
  de asset proprietário por suposição, nem tradutor geral de shader DirectX.
- Implemente R1 apenas depois do contrato/fixtures fechados; upload/variante GLSL
  e readback precisam provar todos os quadrantes, orientação, estados e invalidação.
  Entregue como incremento separado, sem misturar com protocolo/gameplay.

## Tarefas condicionais — executar quando Codex fornecer as entradas reais

### WEB-05 — adaptar a primeira chamada original da Angelica

Fontes: client-153, commit `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`.
Consulte A1/A2 do método e e06 WO-13/14. Se faltarem os arquivos, entregue uma
lista mínima dos paths necessários, sem stubs. Codex prepara o bundle autorizado
com hashes, headers de copyright e compilação/include trace para revisão.

Inspeção já realizada: `A3DMesh::RenderDirect` chama `A3DStream::Appear` e
`A3DDevice::DrawIndexedPrimitive`; `A3DStream` escapa por SetStreamSource,
SetIndices e FVF, e sua inicialização registra objetos/contadores na engine.
`A3DMesh::Render` também envolve LOD, materiais e texturas. `A3DVERTEX` possui
posição/normal/UV; `A3DLVERTEX` possui diffuse/specular/UV. **Nenhum é VertexPC
intercambiável por cast.** Essa é triagem lexical, não closure completa.

- Com fontes disponíveis, feche includes/símbolos/libs/configuração e escolha o
  caminho real mínimo, sem UI/rede. Isole plataforma sem trocar lógica de jogo.
- Adapte só a closure comprovada para o backend; preserve semântica e registros
  necessários. Não ativar HWI/PURESERVER/no-op para obter falso sucesso de draw.
- Teste o mesmo input e compare pontos/cores/profundidade com baseline Windows.
  Se a baseline não existir, registrar integração experimental e paridade BLOCKED.

### WEB-06 — assets e streaming reais

Após receber fontes/fixtures autorizadas correspondentes a 1.5.3: e05 (PCK/PKX,
malha/textura/material/esqueleto/animação), e15 (manifesto/hashes, carregamento
assíncrono, cache e cancelamento). Preserve offsets/endianness/layouts da versão,
limites e validação de truncados/overflow. Fetch/cache disponibiliza bytes antes
que o núcleo síncrono os consuma. Nada de inventar ECM/SKI/BON pelo nome.

### WEB-07 — loop, input, cena e personagens

Após backend e formatos reais: e03/e04 e e07–e09. Porte tick/frame sem bloquear
UI, foco/tab oculta/delta limitado, câmera, terreno/visibilidade, avatar/skinning,
animação, interface e Web Audio após gesto. Use lógica original fornecida,
fixtures determinísticas e ciclo de vida explícito. Não substituir gameplay por
uma demo procedural. Sem contratos prontos, escreva-os a partir das fontes antes
de codificar a parte dependente.

### WEB-08 — gateway, sessão e mundo

Após Codex confirmar realm/contratos/capturas: e10/e11, depois e12–e14.
Preserve framing GNET/gamedata e bytes da versão, transporte WSS, limites,
backpressure, reconexão, autenticação, personagens e entrada no mundo. Não
presumir compatibilidade do servidor Rust com 1.5.3. Depois: entidades/movimento,
voo, combate, skills, NPC, missões, inventário/economia e social/multiplayer.
Teste 2 clientes com estados convergentes e pacotes reais. Segurança autoritativa
permanece no servidor. Fixtures inventadas não comprovam interoperabilidade.

### WEB-09 — fechar qualidade e entrega

e15–e19: streaming/cache/performance medidos, observabilidade, segurança e
deployment, desktop/mobile/acessibilidade, lifecycle/reconnect/regressão.
Critérios G0–G6 de `docs/VALIDATION.md` continuam obrigatórios. Port completo exige
sessão real e funcionalidades do objetivo verificadas; resultados sintéticos ou
build verde não bastam. Merge/publicação/releases seguem revisão humana.

## Trabalho reservado ao Codex com internet/laboratório

1. Concluir provisionamento oficial de Emscripten e prover versões/dependências
   necessárias, sem assumir acesso do ChatGPT Web a `/tmp` desta máquina.
2. Fornecer snapshots mínimos autorizados das referências/fixtures quando
   necessários; preservar SHA/proveniência e não enviar credenciais/assets enormes.
3. Validar baseline Win32/VS2005 + SDKs reais; documentar bloqueios de middleware.
4. Rodar builds WASM e provas Chrome reais, negativos, restore e testes mobile;
   verificar UI/GL/erros e guardar evidência local sanitizada.
5. Revisar patches do ChatGPT Web, aplicar no HEAD atual sem sobrescrever outros,
   reproduzir falhas, corrigir com regressão e atualizar o PR. Não alegar execução
   de um teste só porque seu código foi escrito pelo outro agente.

## Formato obrigatório de devolução

Para cada WEB-XX: status IMPLEMENTED/TESTED/BLOCKED/NOT RUN, arquivos alterados,
SHA de base, hipótese, testes RED/GREEN realmente executados, comandos exatos,
resultados/contagens, limitações e dependência seguinte. Entregue commits/patches
pequenos por incremento. Liste arquivos/fontes/ferramentas que faltam em vez de
fabricar APIs ou enfraquecer testes. Ao terminar o lote executável, deixe a
lista objetiva para Codex revisar e testar. Não declarar o objetivo completo.
