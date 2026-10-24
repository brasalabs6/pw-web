# Arquitetura — Renderer WebGL2 da engine Angelica

DrawView validado → backend C++ GLES3/Emscripten → WebGL2 → readPixels. R0 define unlit/indexed; R1 fecha textura/sampler/targets antes de implementação. Caller mantém dados durante consumo; backend possui recursos GPU e os invalida em context loss.

## Donos e consumidores

Backend sintético e, condicionado aos gates, primeiro draw original comparado; e07 só consome WO-05 homologada. O pw-web é dono dos novos contratos de plataforma; o servidor
é dono do wire format; referências são evidência, não dependências automáticas do
build público. Recursos possuem owner/lifetime explícitos; erro não é sucesso vazio.

## Caminhos de falha

- WO-01: Estado observado fora da fatia não é implementado por especulação, nem ignorado se for alcançado.
- WO-02: Transposição dupla, BGRA tratado como RGBA e inversão global de Y são detectáveis pelas fixtures.
- WO-03: Viewport zero, índice fora do range ou shader inválido não produzem PASS; contexto indisponível é falha.
- WO-04: Estado vazando entre draws ou textura liberada é detectado pelo teste, não mascarado com reset global arbitrário.
- WO-05: Ausência da baseline nativa ou diferença sistemática de transformação impede PASS.
- WO-06: Nenhuma feature ausente pode virar no-op bem-sucedido ou fallback visual silencioso.
- WO-07: Dados insuficientes, dimensão acima do limite e formato ausente têm erro específico.
- WO-08: Dimensão zero, payload curto e uso pós-dispose falham sem leitura fora do buffer.
- WO-09: Enum desconhecido retorna UnsupportedFeature; alpha-test não é tratado como blend equivalente.
- WO-10: Attachment incompatível ou framebuffer incompleto nunca é considerado Ready.
- WO-11: Combinação não suportada e compile/link failure não reutilizam silenciosamente shader anterior.
- WO-12: Resize inválido preserva o recurso válido anterior e reporta erro.
- WO-13: Se a entrada arrasta mundo inteiro/ActiveX, reduzir a fatia ou registrar bloqueio; não criar stubs.
- WO-14: Qualquer símbolo sem implementação ou feature usada ausente bloqueia; não retornar sucesso falso.
- WO-15: Restore parcial ou callback após dispose não marca Ready nem vaza recursos.
- WO-16: Métrica indisponível não vira zero e resultado de software não fundamenta promessa de FPS em hardware.

## Dependências reais

Ver dependencies dos JSONs e o grafo em [port-program.md](../../port-program.md).
Não há requisito global de compilar todo Windows antes de R0 sintético. Integração
original e paridade final exigem a baseline, closure e autorização correspondentes.

## Contratos canônicos

- [.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md](../../../.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md)
- [docs/DECISIONS/ADR-0004-incremental-port-method.md](../../../docs/DECISIONS/ADR-0004-incremental-port-method.md)
