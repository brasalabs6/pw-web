# Testes — Renderer WebGL2 da engine Angelica

Positivo e negativo por workorder são obrigatórios. Ferramenta ausente significa
BLOCKED; teste executado com falha significa FAIL. Build sozinho não é prova runtime.

## WO-01

- Positivo: Inventário estabelece subconjunto necessário e identifica toda dependência que impede o primeiro draw.
- Negativo: Estado observado fora da fatia não é implementado por especulação, nem ignorado se for alcançado.
- Comando/ação: `Revisão do call path e log do laboratório quando disponível; ./scripts/verify.sh`.

## WO-02

- Positivo: Nenhuma chamada D3D usada pela fatia fica sem semântica, erro ou adaptação explícita.
- Negativo: Transposição dupla, BGRA tratado como RGBA e inversão global de Y são detectáveis pelas fixtures.
- Comando/ação: `Revisão de tabelas e fixtures contra baseline; ./scripts/test-render-clip.sh`.

## WO-03

- Positivo: Fixtures R0 6/7 desenham cores/profundidade esperadas; pixel central e canto são conferidos por readPixels.
- Negativo: Viewport zero, índice fora do range ou shader inválido não produzem PASS; contexto indisponível é falha.
- Comando/ação: `./scripts/build-render-probe.sh (criar) + autoteste browser; ./scripts/test-render-clip.sh`.

## WO-04

- Positivo: Cena com opaco, alpha e offscreen produz pixels previstos e mantém contagem estável de recursos após reload.
- Negativo: Estado vazando entre draws ou textura liberada é detectado pelo teste, não mascarado com reset global arbitrário.
- Comando/ação: `Runner browser de cena criado nesta WO; ./scripts/build-render-probe.sh; ./scripts/verify.sh`.

## WO-05

- Positivo: Relatório demonstra paridade da primeira fatia Angelica, com referência Windows e limites verificáveis.
- Negativo: Ausência da baseline nativa ou diferença sistemática de transformação impede PASS.
- Comando/ação: `Runners de WO-04/14/16 + captura Windows de e02/wo-04; comandos reais no relatório.`.

## WO-06

- Positivo: Caller sabe exatamente o que o backend suporta e como a ausência é reportada.
- Negativo: Nenhuma feature ausente pode virar no-op bem-sucedido ou fallback visual silencioso.
- Comando/ação: `Walkthrough de erros R0 e inventário; ./scripts/verify.sh`.

## WO-07

- Positivo: R1 fecha todo dado necessário a um upload e draw texturizado, com resultado de pixels previsto.
- Negativo: Dados insuficientes, dimensão acima do limite e formato ausente têm erro específico.
- Comando/ação: `Revisão de fixture 2x2 e lifecycle; ./scripts/verify.sh`.

## WO-08

- Positivo: Quadrantes têm cores esperadas e bytes inválidos não alteram uma textura válida existente.
- Negativo: Dimensão zero, payload curto e uso pós-dispose falham sem leitura fora do buffer.
- Comando/ação: `./scripts/test-render-texture.sh (criar; autoteste browser)`.

## WO-09

- Positivo: Estados mapeados reproduzem a semântica aprovada sem vazamento entre draws.
- Negativo: Enum desconhecido retorna UnsupportedFeature; alpha-test não é tratado como blend equivalente.
- Comando/ação: `./scripts/test-render-states.sh (criar; autoteste browser)`.

## WO-10

- Positivo: WO-12 recebe contrato fechado com estado anterior preservado em falha de criação/resize.
- Negativo: Attachment incompatível ou framebuffer incompleto nunca é considerado Ready.
- Comando/ação: `Revisão dos estados e fixture offscreen; ./scripts/verify.sh`.

## WO-11

- Positivo: Cada combinação suportada tem fixture de imagem e chave única; não há programa reutilizado incorretamente.
- Negativo: Combinação não suportada e compile/link failure não reutilizam silenciosamente shader anterior.
- Comando/ação: `./scripts/test-shader-variants.sh (criar; autoteste browser)`.

## WO-12

- Positivo: Offscreen e readback retornam dimensões/cores esperadas após resize válido.
- Negativo: Resize inválido preserva o recurso válido anterior e reporta erro.
- Comando/ação: `./scripts/test-render-target.sh (criar; autoteste browser)`.

## WO-13

- Positivo: Closure tem fronteira auditável e não depende de suprimir funcionalidades usadas para compilar.
- Negativo: Se a entrada arrasta mundo inteiro/ActiveX, reduzir a fatia ou registrar bloqueio; não criar stubs.
- Comando/ação: `Build de pesquisa isolado com flags/dependency files reais no relatório.`.

## WO-14

- Positivo: Um draw executado pelo caminho original adaptado coincide com os valores/pixels da baseline.
- Negativo: Qualquer símbolo sem implementação ou feature usada ausente bloqueia; não retornar sucesso falso.
- Comando/ação: `Runner específico da closure definido em WO-13, com build Windows e browser.`.

## WO-15

- Positivo: Draw em Lost retorna ContextLost; após recriação válida o pixel de referência volta a ser desenhado.
- Negativo: Restore parcial ou callback após dispose não marca Ready nem vaza recursos.
- Comando/ação: `./scripts/test-render-context.sh (criar; autoteste browser com WEBGL_lose_context)`.

## WO-16

- Positivo: Benchmark informa condições, amostras e métricas reais; cenários de paridade permanecem verdes.
- Negativo: Métrica indisponível não vira zero e resultado de software não fundamenta promessa de FPS em hardware.
- Comando/ação: `node tests/render/benchmark.mjs (criar runner real de browser) + suite gráfica de WO-04`.

## Evidência

Registrar comando, versão, SHA, fixture, resultado e limites em docs/evidence;
logs/artifacts locais não substituem um resultado verificável. Comandos “criar” são
entregáveis futuros; registrar zero casos ou screenshot isolada não satisfaz aceite.
`./scripts/verify.sh` continua obrigatório e não valida o cliente completo.
