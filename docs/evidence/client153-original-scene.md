# Primeiro objeto original no browser — 2026-10-26

Hipótese: os pacotes PWBR v145 fornecidos pelo operador podem alimentar uma
adaptação limitada do loader Angelica 1.5.3 e um dispositivo GLES3/WebGL2, sem
converter a geometria em dados procedurais nem entregar o PCK inteiro ao browser.

## Código e inputs

Base: PR #15 em `24ac7187ec292a094e8071d467a9a6504c2fbe22`. Alteração em
`src/client153`, CMake, leitor PCK de build e host browser. Os arquivos upstream
e hashes estão em `src/client153/source-manifest.json`; autorização em LEGAL.

- MOX original: `models/home/物品/柜立柜/柜立柜.mox`, 1.199 bytes,
  SHA-256 `f782ad41cbc394eef49145fb3c410e17b01892ede7944447f0c5b6e16cb90453`.
- DDS original: `models/home/物品/柜立柜/textures/镂空网状大衣柜.dds`, 87.536 bytes,
  SHA-256 `2cd92a620a65e808b55463ead0fe8b38858af1d25c7b83a760e3f18d303ca049`.
- Loader: 2 frames, 1 mesh, 24 vértices, 36 índices. Textura DXT3 256×256,
  9 mipmaps originais, upload direto sem transcode/descarte de níveis.

## Execuções

Comandos completos e variáveis de ambiente: [guia do port](../CLIENT153-PORT.md).
Emscripten 6.0.13, CMake 4.4.4, Ninja 1.13.2; GCC 16.2.1 no nativo.
Chrome 152.0.7977.64, WebGL 2.0 / OpenGL ES 3.0 Chromium, ANGLE Vulkan SwiftShader.

- Nativo: CTest MOX e DDS **2/2 PASS**; PCK/staging **5 testes Python PASS**.
- WASM/Node: os mesmos CTests **2/2 PASS**. RED anterior comprovado: ambos
  abortavam na entrada inválida com exception catching desabilitado. GREEN após
  compilar/linkar com `-fexceptions`; truncamentos retornam erro sem abort.
- Chrome desktop 1100×900: pixels texturizados observados, 132.880 pixels
  diferentes do clear, 16.914 cores, 1 draw, zero erro GL e zero pageerror.
- Chrome mobile 390×844 (emulação): 116.938 pixels diferentes do clear e
  16.045 cores. Isto não substitui teste em GPU/dispositivo móvel físico.
- Provas browser: objeto original + mipmaps, rotação muda pixels, contexto perdido
  e recursos reconstruídos, bytes adulterados rejeitados por hash, MOX inválida
  com hash correspondente rejeitada pelo loader WASM sem abort.
- Regressão de alocação GPU: injeção de falha em `createTexture` demonstrou 4
  recursos restantes. A rejeição agora libera objetos parciais e o host chama
  dispose em erro; a prova exige zero recursos restantes.
- `./scripts/verify.sh`: 31 Python + core e 14 grupos R0 PASS.

Manifestos, screenshots e corpus ficam somente em `artifacts/`, ignorado pelo Git.
CI deste incremento executa build/testes sem os dados do cliente; consultar o SHA
exato do PR antes de declarar seu resultado. Não chamar o smoke antigo da demo de
prova desta cena.

## Corpus real e limites

Os 15 índices PCK/PKX foram lidos. O [inventário gerado](client153-assets-inventory.json)
registra tamanhos, contagens, extensões e SHA-256 de cada arquivo físico. O corpus nativo contém 711 arquivos `.mox`:
428 `MOXB`/A3DFrame passaram (61 em models e 367 em grasses); 41 MOXT não estão
implementadas; 242 arquivos de building usam outro envelope e foram rejeitados.
Extensão `.mox` não basta para selecionar um loader.

Esta é uma cena de recurso original com câmera de inspeção, shader unlit e frame
zero. Material/iluminação completos, animação, terrain, UI, áudio, rede, login e
mundo não estão portados. Comparação semântica com a baseline Wine depende da
trilha paralela; G4/G5/G6 continuam abertos. SwiftShader é software, sem medida de
performance de GPU física. O objetivo do cliente integral permanece ativo.
