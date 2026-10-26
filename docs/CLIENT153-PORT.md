# Cliente 1.5.3 — execução do port

Objetivo: cliente integral no navegador com os recursos reais PWBR v145 e a
source `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`. O cliente Windows/Wine e o
servidor são validados por agentes paralelos. Este incremento cobre o primeiro
carregamento e desenho de objeto original; o jogo completo continua pendente.

## Implementação atual

- `tools/assets/pck153.py`: leitura limitada e somente leitura dos pacotes
  observados `0x20002`, incluindo PCK/PKX dividido em `0x7fffff00`. Dados
  comprimidos usam zlib com tamanho/EOF verificados; nomes CP936 preservados.
- `src/client153/Angelica2/Angelica3D/port`: adaptação da leitura binária de
  `A3DFrame`, `A3DMesh`, material e strings, preservando geometria, hierarquia,
  transforms, propriedades, LOD e referências de textura. DDS DXT1/3/5 mantém
  os blocos e todos os mipmaps originais para upload por S3TC.
- `src/client153/platform/web`: dispositivo WebGL2, buffers e upload de DDS.
  A cena atual usa frame zero, câmera de inspeção e shader texturizado unlit;
  não oferece paridade com iluminação/renderer completos da Angelica.
- `web/client153`: fetch com tamanho/SHA-256, resolução de textura CP936,
  interação de câmera e reconstrução de recursos após perda de contexto.
- CMake compila arquivos explícitos, sem submódulos, Win32 ou SDK binário. As
  exceções C++ são preservadas no WASM; testes truncados não podem abortá-lo.

Proveniência e hashes upstream: [source-manifest](../src/client153/source-manifest.json).
Autorização: [decisão do operador](LEGAL.md). Assets ficam fora do Git.

## Reproduzir

Ative Emscripten 6.0.13; disponibilize CMake >=3.25, Ninja e Python >=3.10.
A execução local foi feita com CMake 4.4.4 e Ninja 1.13.2.

```bash
cmake -S . -B build/client153-native -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/client153-native
ctest --test-dir build/client153-native --output-on-failure
python3 -m unittest discover -s tests/assets -p 'test_*.py'

emcmake cmake -S . -B build/client153-wasm-tests -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/client153-wasm-tests
ctest --test-dir build/client153-wasm-tests --output-on-failure

# Diretório app/ do cliente autorizado fornecido pelo operador:
export PW_CLIENT153_ROOT=/caminho/para/app
./scripts/build-client153.sh
python3 -m http.server 8497 --bind 127.0.0.1 --directory build/client153-wasm/browser
```

Em outro terminal, com Chrome/Playwright instalados externamente:

```bash
CHROME_BIN=/caminho/chrome PW_BROWSER_MODULE_DIR=/pai/de/node_modules \
  node tests/client153/browser-scene.cjs
```

O servidor HTTP expõe somente o candidato gerado. O preparador não escreve
no cliente original e não serve pacotes inteiros. A cena inicial transfere uma
MOX de 1.199 bytes e uma DDS de 87.536 bytes, além do runtime e shell; isso não
é orçamento de loading do jogo completo. Evidências ficam em `artifacts/`.

## Sequência de migração e critérios pendentes

| Área | Estado comprovado / próximo aceite |
| --- | --- |
| Pacotes reais | 15 índices PCK/PKX lidos; criptografia e outras versões não implementadas |
| Frame/MOX binária | 428 recursos carregados no corpus nativo; MOXT e envelope de building ainda pendentes |
| Primeiro objeto real | Armário original texturizado no Chrome; sem comparação Wine ainda |
| Render completo | Iluminação, material, alpha/cull original, skinning, efeitos, terreno e comparação baseline pendentes |
| Personagem | SKI/BON/STCK e integração da lógica original pendentes |
| Mundo | Mapas/células, colisão, visibilidade e transição de instâncias pendentes |
| Cliente | UI/login/input de jogo/áudio/loop e demais sistemas da baseline pendentes |
| Rede | Consumir contrato/evidência do agente servidor; WSS, GNET/gamedata, login/mundo e duas sessões pendentes |
| Entrega | Pipeline completa e streaming/cache/CDN seguem PR #16; este build não publica nada |

Corpus observado: 101 MOX de `models` (61 binárias aceitas, 40 MOXT ainda
rejeitadas); 368 de `grasses` (367 binárias aceitas, 1 MOXT rejeitada);
242 de `building` usam um envelope de building diferente de A3DFrame e são
rejeitadas. Não converter esse resultado em alegação de compatibilidade integral.

A migração agora segue a compilação das bibliotecas originais e do cliente,
preservando seus sistemas. `client153_common` contém a classe original `AString`
e as APIs `ATime` adaptadas e testadas em nativo/WASM; não representa toda a AngelicaCommon.
SKI/BON/STCK, building e os demais recursos continuam necessários dentro dessa
integração. Ver [prova do build original](evidence/client153-full-build.md).


O host da cena agora monta temporariamente o modelo verificado no VFS do WASM
e o abre pela interface original `AFile`, removendo-o após a leitura, inclusive
em falhas. As texturas continuam pelo upload de bytes verificados. O cliente
completo e o cache/streaming com orçamento de memória ainda não estão integrados.


O target `client153_math` compila os tipos originais de vetor/matriz/quaternion,
e a hierarquia da cena usa `A3DMATRIX4`. A prova e as diferenças da source estão
em [matemática original](evidence/client153-original-math.md). Skeleton, tracks
e skinning ainda precisam ser integrados ao cliente.


`client153_animation` agora compila as trilhas originais `A3DTrackData` de vetor,
quaternion e float. O BON real do lobo passou no carregamento de 84 trilhas e
31.122 amostras nativas, inclusive sob sanitizadores. Isso ainda não conecta
esqueleto e malha ao renderer. Ver [evidência das trilhas](evidence/client153-original-tracks.md).


A migração de skeleton fechou mais dependências de Common: arrays/listas,
logging e `A3DObject` agora compilam com testes nativos/WASM. O skeleton completo
continua pendente; ver [evidência dessas dependências](evidence/client153-common-containers.md).

O fechamento do skeleton avançou nas dependências de suporte: helpers escalares
(`A3DFuncs`), limites geométricos (AABB/OBB), hierarquia de coordenadas,
controllers, interfaces IK e globais da engine (`A3DPI`) agora compilam em
nativo/WASM com sanitizadores, com defeitos originais corrigidos e documentados.
Bones, joints e o skeleton em si permanecem para o próximo lote. Ver
[evidência do suporte ao skeleton](evidence/client153-skeleton-support.md).
