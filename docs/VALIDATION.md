# Matriz de validação e critérios go/no-go

## Regra
Nenhum cliente foi aprovado definitivamente. Candidata 1.5.3 tem Angelica2 com D3D8; variante 1.5.2 moderna tem Angelica2.2/D3D9. Favoritismo inicial NÃO substitui teste.

## Matriz de seleção (evidência obrigatória por campo)
| Critério | 1.5.3 Angelica2 | 1.5.2 Angelica2.2 | Como validar |
| --- | --- | --- | --- |
| Código da engine presente | confirmado por árvore pública | confirmado | compilar módulos e rastrear símbolos |
| Renderizador | D3D8/D3DX8 | D3D9/D3DX9 | A3DPlatform.h, backend/estado/shader |
| Toolchain declarado | VS2005 | VS2022 | build limpo em VM |
| Binário Windows rodando | não demonstrado | não demonstrado | vídeo/log/versão/hash |
| Login em servidor alvo | não demonstrado | não demonstrado | captura de handshake |
| Assets correspondentes | não demonstrado | não demonstrado | manifesto e hash |
| Licença válida do cliente | não demonstrada | não demonstrada | chain of title autorizada |
| Esforço wasm | não medido | não medido | métricas de build/tracing |

## Gates
- **G0 plataforma:** `scripts/verify.sh` verde, WebGL2 no browser, build WASM trivial por `build-wasm.sh`.
- **G1 código original:** build Windows limpo sem hacks não documentados e inicialização até a cena.
- **G2 licenças:** comprovar autorização para cada parte antes de embutir/distribuir.
- **G3 engine WASM:** pelo menos um módulo útil real compilado e testado com entradas permitidas.
- **G4 render:** malha/tex/material/animação com desenho coerente vs referência.
- **G5 rede:** handshake, autenticação, lista de personagens e entrada no mundo.
- **G6 multiplayer:** 2 abas com personagens se vendo/movendo; sem streaming remoto.

## Template de evidência
- ID, data, dono, commit, plataforma, versão cliente, versão protocolo.
- Comando exato, input autorizado, log/output real.
- Resultado e interpretação, limites, `pass/fail/blocked`.
- Próxima hipótese testável.

## Critério técnico de escolha
Pontuação ponderada sugerida: reprodutibilidade de build 30%, esforço de adaptação browser 25%, compatibilidade com assets e protocolo 25%, documentação/depuração 20%. **Autorização jurídica é gate independente e obrigatório, não pontuação compensável.**
