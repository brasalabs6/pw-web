# POC-001 — componente original da Angelica em WebAssembly

Data: 2026-10-24. Estado: **PASS no recorte descrito abaixo**.

## Hipótese e resultado

Hipótese: um componente de cálculo da Angelica pode ser compilado sem alterações
para código nativo e wasm32, executar no navegador e produzir resultados equivalentes.

Foi compilado **o arquivo original**
`references/client-153/Angelica2/AngelicaCommon/Source/ARandomGen.cpp`, incluindo
`ARandomGen.h` e `ABaseDef.h` diretamente da referência. Ele consta em
`AngelicaCommon.vcproj`. Nenhum desses arquivos foi copiado ou modificado.

| Verificação executada | Resultado |
| --- | --- |
| `./scripts/bootstrap.sh` | PASS: 18 testes Python e smoke C++ nativo |
| `./scripts/build-wasm.sh` + import no navegador | PASS: versão 1, soma 7+5 e leitura little-endian |
| `ARandomGen` nativo | PASS: repetibilidade e limites de uniforme, inteiro e float; gaussianos finitos |
| `ARandomGen` wasm32 no Chrome | PASS: 4.096 uniformes idênticos ao nativo; 256 gaussianos dentro da tolerância |
| Sementes testadas | 0, 1, 42 e 4.294.967.295; 1.024 uniformes e 64 gaussianos por semente |
| Tolerância dos gaussianos | `abs(native-wasm) <= 1e-12 * max(1, abs(native))` |
| WebGL 2 | PASS: malha procedural de 32×32 pontos, 1.922 triângulos; leitura de pixel confirma desenho |
| Teste negativo no navegador | Amostra nativa adulterada é rejeitada: `uniform: seed 0, sample 0 differs` |
| Testes do comparador | PASS: 4 testes Node, incluindo saída vazia, truncada, NaN e divergente |
| Módulo da prova | 22.280 bytes de WASM nesta compilação; JS e JSON são arquivos adicionais |
| Compilação direta de `A3DEngine.cpp` para WASM | FAIL esperado: `A3DPlatform.h:16: 'Windows.h' file not found` |
| Cliente Windows completo / login / mundo | NÃO EXECUTADOS; não comprovados |

Ferramentas: GCC 16.2.1, Emscripten **6.0.13**, Chrome headless 152 em Linux x86_64.
Referência: `e3d6d0075d0a745c9f9859f2cb25b01d71dec045`.
O manifesto local registra versões e SHA-256 das entradas, baseline e WASM.
O renderer reportado pelo Chrome foi **ANGLE/SwiftShader (software)**, com aviso
de fallback no console; não houve erro JavaScript. Isso valida API/desenho local,
mas não aceleração por GPU física. Não foram medidos FPS, desempenho em GPU física,
outros browsers ou celulares. Resultado desta sessão salvo localmente em
`artifacts/angelica-probe/browser-result.json`; o build não gera esse relatório
automaticamente, pois ele depende da execução efetiva do browser.

## O que a imagem significa

O WASM executa os testes do `ARandomGen` e imprime seus resultados. O host compara
esses números com `native.json`. **A malha usa os números produzidos pelo WASM**,
da semente 42. Um renderer WebGL 2 escrito para este experimento desenha a malha.

Isso comprova execução de um componente original e transporte de seus resultados
até um desenho real no browser. Não comprova o renderer da Angelica, seus formatos,
animação, modelos originais, protocolo ou gameplay. A imagem não é um mapa do jogo.
Os JSONs são evidência deste teste, não o protocolo de renderização proposto para produção.

Há uma diferença de ABI importante: o `DWORD` de `ABaseDef.h` é `unsigned long`.
Neste Linux ele tem **8 bytes**; em wasm32 tem **4 bytes**. Este teste usa sementes
explicitamente limitadas a `uint32_t`, e o componente não serializa estruturas.
A paridade observada não autoriza reutilizar layouts binários Linux em arquivos/rede.

## Reprodução

Pré-condições: referência já obtida conforme `LEGAL.md` (autorização 2026-10-25), compilador C++17, Python 3,
Git, Emscripten 6.0.13 e navegador com WebGL 2. Node é usado apenas nos testes do
comparador. A prova não baixa referências ou ferramentas automaticamente.

Para instalar o SDK isolado, fora do repositório, siga a
[documentação oficial](https://emscripten.org/docs/getting_started/downloads.html):

```bash
git clone https://github.com/emscripten-core/emsdk.git /tmp/pw-web-emsdk
/tmp/pw-web-emsdk/emsdk install 6.0.13
/tmp/pw-web-emsdk/emsdk activate 6.0.13
source /tmp/pw-web-emsdk/emsdk_env.sh
```

O SDK da sessão está nesse diretório temporário; ele não é instalação permanente.
Se já existir, reutilize o checkout em vez de executar `git clone` novamente.

Na raiz do projeto:

```bash
./scripts/bootstrap.sh
./scripts/build-wasm.sh
node --test tests/probes/compare.test.mjs
./scripts/probe-angelica.sh
python3 -m http.server 8481 --bind 127.0.0.1 --directory artifacts/angelica-probe
```

Abrir **http://127.0.0.1:8481/**. O próprio teste da página valida paridade, ABI,
compilação dos shaders e leitura de pixel; só exibe PASS depois dessas verificações.
Falha de import, JSON, comparação ou WebGL resulta em FAIL. Recarregar reinicia a prova.
Para a demo genérica, executar `PORT=8480 ./scripts/dev.sh` em outro terminal.

Asserção automatizável no console/CDP após a conclusão:

```js
const result = JSON.parse(document.querySelector('#result').textContent);
if (document.querySelector('#status').dataset.state !== 'PASS' ||
    result.compared.uniform !== 4096 || result.compared.gaussian !== 256 ||
    result.triangles !== 1922) throw new Error('POC-001 failed');
```

O teste negativo realizado interceptou somente a resposta de `native.json`, somou
0,01 à primeira amostra e exigiu FAIL. Depois a página foi recarregada sem interceptação.
Esse teste não muda os arquivos da referência nem a implementação do gerador.

Para reproduzir o bloqueio do renderer (comando **deve falhar**, não é build completo):

```bash
em++ -std=c++17 -fsyntax-only -ferror-limit=1 \
  -Ireferences/client-153/Angelica2/Angelica3D/Header \
  -Ireferences/client-153/Angelica2/AngelicaCommon/Header \
  references/client-153/Angelica2/Angelica3D/Source/A3DEngine.cpp
```

## Arquivos e isolamento

- `tests/probes/angelica_random.cpp`: harness; inclui a referência, não sua implementação.
- `tests/probes/compare.mjs` e `compare.test.mjs`: comparação e rejeição de falsas equivalências.
- `tests/probes/index.html` e `app.mjs`: host local e renderer procedural do experimento.
- `scripts/probe-angelica.sh`: builds nativo/WASM e manifesto de proveniência.
- `artifacts/angelica-probe/`: executável local, baseline JSON, WASM, host e manifesto.
  Diretório ignorado pelo Git; não publicar, servir externamente nem incluir no CI padrão.

O build público e `web/` continuam contendo somente a prova genérica do projeto.
O script opt-in não é chamado pelo bootstrap nem por workflows. Não houve mudança
em contratos de servidor, protocolo, assets ou referência fixada.

## É possível compilar o cliente inteiro?

**Windows: plausível, ainda não demonstrado. Browser: exige port de plataforma.**
O host desta prova não dispõe do conjunto VS2005 + SDKs Windows/DirectX necessário
para validar o executável original. Wine disponível não substitui essa toolchain.

Evidência estática da configuração `Release|Win32` de `ElementClient.vcproj`:

- 42 bibliotecas no linker; 25 nomes encontrados na árvore; os 17 restantes são
  bibliotecas de Windows/DirectX. Encontrar o nome não valida caminho, ABI ou versão.
- As três soluções principais referenciam 21 projetos existentes; o projeto do
  cliente tem 785 entradas `.cpp`. Isso não prova closure de includes/link.
- `Release` usa Angelica2/D3D8. Outras configurações referenciam `Angelica2.2`,
  diretório ausente: escolher uma configuração coerente antes de corrigir erros.
- O caminho de saída de Release é específico da máquina do autor. Ajustá-lo no
  laboratório, sem alterar a referência congelada.
- `A3DEngine.cpp` chama `Direct3DCreate8`; `A3DDevice` e `A3DStream` usam buffers,
  estados e transformações D3D. O cliente usa Win32, ATL/COM, sockets e código x86.
- Bibliotecas PE/COFF e DLLs Windows não são módulos wasm32. Dependências precisam
  de fonte compilável, alternativa autorizada ou exclusão explícita da funcionalidade.
- Assets correspondentes e entrada no mundo permanecem sem validação.

Próximo passo: seguir o [método de port incremental](ANGELICA-PORTING-METHOD.md).
Ele define o que Grok/GLM podem executar agora e os gates anteriores à integração.

## Reprodução automatizada da etapa de browser (opt-in local)

A prova original **não roda automaticamente na CI pública** porque depende da referência Angelica e de toolchains em ambiente autorizado. A CI verifica o comparador com fixtures sintéticas usando `node --test tests/probes/compare.test.mjs`; isso **não recompila ARandomGen**.

Após `./scripts/probe-angelica.sh` no laboratório autorizado, use:

```bash
# Chrome for Testing e Playwright-core instalados localmente
export CHROME_BIN=/path/to/chrome
export PW_BROWSER_MODULE_DIR=/path/to/pw-browser-deps
bash scripts/probe-angelica-browser.sh
```

O runner mantém a porta **8481** apenas em `127.0.0.1` e grava em `artifacts/angelica-probe/`: `browser-positive.png`, `browser-negative.png` e `browser-proof.json`. O teste positivo exige 4.096 uniformes, 256 gaussianos, 1.922 triângulos, pixel lido e ABI WASM32; o negativo adultera a resposta de `native.json` **em memória no Chrome**, e exige FAIL. Um erro de JS encerra a prova com falha.

O runner não publica binários, fontes, screenshots nem dados derivados no GitHub e não consulta serviços externos. O resultado só pode ser denominado **POC recompilada e verificada** se o comando realmente for executado; indisponibilidade de referência, Emscripten ou Chrome é `BLOCKED`. Para levar essa execução a uma CI privada autorizada, provisionar as ferramentas e a referência sob controle de acesso, definir política de direitos, e nunca transformar o artefato em download público por padrão.
