# Contrato R0 — primeira malha indexada

**Fonte de verdade.** Contrato interno do pw-web, versão 0.

Este contrato é novo, interno ao `pw-web`, versão 0, para uma prova sintética.
Não altera ABI do cliente nem protocolo externo. Estado: **especificado, não implementado**.
Donos previstos: `engine/render/` (semântica/testes) e `engine/platform/web/` (GL/browser).
Consumidor inicial: harness de teste; depois, um adaptador Angelica autorizado.

### Dados e semântica

- `VertexPC`: `float x,y,z` e `uint32_t argb`. Layout intercalado de **16 bytes**,
  posição no offset 0, cor no offset 12. Validar `sizeof`/`offsetof` em compile time.
  Posições precisam ser finitas; cor é `0xAARRGGBB`.
- Índices: `uint16_t`, topologia exclusivamente triangle list. De 3 a 196.608
  índices, múltiplo de 3; de 1 a 65.536 vértices; cada índice é menor que vertexCount.
  Esses limites pertencem ao harness R0, não aos formatos completos do jogo.
- Matriz: 16 `float` finitos, ordem **row-major**, semântica legado `p_row * M`.
  Um ponto `[x,y,z,1]` produz `clip[j] = sum(p[i] * M[i*4+j])`.
- Conversão de clip D3D → GL: `[x, y, 2*z-w, w]`. Não inverter Y globalmente.
  GLSL usa vetores coluna: a matriz matemática é `transpose(M)`; ao empacotar
  `transpose(M)` em column-major, os 16 valores são a sequência row-major original.
  `uniformMatrix4fv` sempre recebe `transpose=false`. Não transpor duas vezes.
- Cor: extrair ARGB por shifts/masks, produzir RGBA normalizado. Não reinterpretar
  diretamente os quatro bytes de um inteiro little-endian como RGBA.
- Viewport: largura e altura inteiras positivas; consultar e respeitar o limite
  real do contexto antes de configurar. Origin no GL é inferior esquerdo.
- Estado fixo R0: depth test `LESS_EQUAL`, depth write habilitado, clear depth 1,
  blend desligado, culling desligado, sem textura, luz, fog, skinning ou alpha test.
  Culling é deliberadamente desativado no contrato da prova, não ignorado em chamadas
  legadas: pedido de uma feature fora do R0 deve retornar `UnsupportedFeature`.
- Semântica de lifetime: a chamada consome os arrays sincronicamente e não retém
  ponteiros do caller. O dispositivo possui e libera os recursos GPU que criar.
  Não expor ponteiros C++ como um protocolo de dados de arquivos/rede.

### Sucesso, erros e estados

API C++17 de P1, em `namespace pw::render`, a implementar em `clip.h`/`clip.cpp`:

```cpp
struct VertexPC { float x, y, z; std::uint32_t argb; };
using Matrix4 = std::array<float, 16>;
using Clip4 = std::array<float, 4>;
struct Viewport { std::uint32_t width, height; };
enum class RenderStatus { Ok, InvalidInput, UnsupportedFeature, ContextLost, GpuError };
struct DrawView {
    const VertexPC* vertices;
    std::uint32_t vertex_count;
    const std::uint16_t* indices;
    std::uint32_t index_count;
    Matrix4 matrix;
    Viewport viewport;
};
RenderStatus to_webgl_clip(const Matrix4&, const VertexPC&, Clip4& output) noexcept;
std::array<std::uint8_t, 4> argb_to_rgba(std::uint32_t) noexcept;
RenderStatus validate_draw(const DrawView&, Viewport maximum) noexcept;
```

`to_webgl_clip` retorna InvalidInput para posição/matriz não finita ou resultado
não finito por overflow; não muda `output` em erro. A cor não participa do cálculo.
`argb_to_rgba` é total para os 2^32 valores. `validate_draw` valida ponteiros não
nulos, contagens, índices, posições/matriz, resultado da transformação e viewport
em `1..maximum` antes de sucesso. `maximum` com qualquer dimensão zero é inválido.
Essas funções puras só retornam Ok/InvalidInput; os outros estados pertencem a P2
e ao futuro adaptador. Features ausentes não devem ser mapeadas silenciosamente a R0.

Caller C++ possui arrays válidos com os tamanhos informados durante a chamada;
ponteiro arbitrário/dangling não é uma entrada que C++ consiga validar. Não exportar
`DrawView` diretamente para JS ou rede: P1 é um contrato interno, não uma ABI wire.
O backend recebe `maximum` do limite real do contexto; testes usam limites explícitos.

Operação conceitual: `drawIndexed(vertices, indices, matrix, viewport)`.
Validar todos os dados antes de upload/draw. Retorno é um resultado explícito:
`Ok`, `InvalidInput`, `UnsupportedFeature`, `ContextLost` ou `GpuError`.
Não fazer clamp de índices, truncar buffers, preencher conteúdo ausente ou retornar
sucesso sem desenho. Mensagem de diagnóstico identifica a operação/limite, sem dados privados.

| Estado | Evento | Resultado/recuperação |
| --- | --- | --- |
| Uninitialized | initialize com contexto WebGL2 válido | Ready; aloca shaders/buffers |
| Uninitialized | contexto indisponível ou shader inválido | erro; não passa a Ready; libera alocação parcial |
| Ready | draw válido | Ok; desenha; permanece Ready |
| Ready | input inválido/feature ausente | erro antes de draw; recursos existentes preservados |
| Ready | context lost | Lost; proíbe novos draws; invalida recursos GPU |
| Lost | draw | ContextLost; não finge renderização |
| Lost | contexto restaurado | inicialização explícita recria recursos; nenhuma handle antiga é reutilizada |
| qualquer | dispose | Uninitialized; dispose repetido é seguro |

### Fixtures obrigatórias

1. Identidade: ponto `(0,0,0,1)` resulta em clip GL `(0,0,-1,1)`.
2. Profundidade: z D3D 0, 0,5 e 1 resultam em z GL -1, 0 e 1, para w=1.
3. Perspectiva: clip D3D `(1,2,0.75,2)` → GL `(1,2,-0.5,2)`; preservar w.
4. Translação: row-major com `M[12]=3`, `M[13]=4`, `M[14]=5`, identidade no resto;
   ponto `(1,2,3,1)` → clip D3D `(4,6,8,1)` e GL `(4,6,15,1)`.
5. Cor `0x80402010` → RGBA bytes `[64,32,16,128]`.
6. Triângulo válido: posições `(-0.5,-0.5,0.5)`, `(0.5,-0.5,0.5)`, `(0,0.5,0.5)`,
   cor branca e índices `[0,1,2]`. Pixel central branco; canto com a cor de clear.
7. Depth: triângulos sobrepostos, vermelho em z=0,25 e verde em z=0,75;
   vermelho no centro independentemente da ordem de submissão.
8. Inválidos: `[0,1,3]` com três vértices, índice count 4, NaN/Infinity na matriz,
   viewport zero, textura solicitada e draw após context loss. Exigir o erro específico.

Comparação numérica: tolerância absoluta de `1e-6` nos casos float definidos acima;
cor/índices exatos. Testes de pixel usam pontos interiores, longe das arestas,
tolerância de 2 por canal em RGBA8; desabilitar antialias no harness. Não comparar
screenshots inteiras bit a bit entre drivers diferentes.

