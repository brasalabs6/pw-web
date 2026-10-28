# Handoff — cliente 1.5.3 no navegador (Grok → Codex, 2026-10-26)

Estado completo do port e instruções para continuidade. Leia junto com
`AGENTS.md`, `docs/CLIENT153-PORT.md` e as evidências em `docs/evidence/`.

## Onde você está

- **Worktree**: `/home/nonexphere/workspace/pw/pw-browser-port`, branch
  `feat/client153-common-containers`, tudo commitado e pushed (árvore limpa,
  exceto `tests/client153/ski_loader.cpp` untracked — WIP antigo seu que
  referencia um `ski.h` inexistente; decida manter ou remover).
- **Entrega**: PR #23 (brasalabs6/pw-web, base `main`) — **PR único do
  cliente**. Regra do `AGENTS.md`: máximo de 2 PRs abertos (servidor #17 +
  cliente #23); um milestone = um commit autônomo com evidência; merge final
  é humano.
- **Goal**: o texto completo está no histórico do PR #23 e no card do agente
  Grok (`~/.agents/running/`). Critério final: jogo real no browser — login,
  personagem animado, entrada no mundo, movimento, duas sessões convergentes.

## Milestones já entregues (commits no PR #23, todos CI-verdes)

| Commit | Milestone | Prova real |
| --- | --- | --- |
| `31e0d2e` | Camada de suporte ao skeleton (AAssist, APoint/ARect, AMemory, ACounter, A3DTypes/A3DPI/A3DFuncs/A3DGeometry/A3DController/A3DCoordinate/IK) | 9/9 nativo/WASM/ASan |
| `d661924` | Loader real PCK/PKX + família de arquivos (AFilePackage, AFilePackGame/Man, AFileImage, AIniFile, AScriptFile, AFI/AFPI, abase::vector/hashtab, ATempMemMan, ACSWrapper) | `facedata.pck` 2005 entradas = leitor Python validado; 64 shared files verificados; 10/10 |
| `7b49cd9` | Fechamento do skeleton (A3DConfig, A3DJoint, A3DBone, A3DBoneController, A3DTrackMan, A3DSkeleton) | BON real do lobo: 42 joints, 42 bones, 5 hooks via `A3DSkeleton::Load` original; 11/11 |

Anteriores (histórico do PR): runtime browser (MOX/DDS via download verificado
→ VFS → AFile → loader → WebGL2), AString/ATime, AFile, matemática, trilhas
(BON: 84 trilhas, 31.122 amostras), containers/logging/A3DObject, método
ADR-0004, plano Cloudflare (ex-#16 mergeado).

## Próximo milestone: skin/skinning (personagem animado no browser)

1. Portar `A3DSkin.h/.cpp`, `A3DSkinMesh*`, `A3DSkinModel*`, `A3DSkinMan`,
   `A3DMorphSkinMesh`, `a3dskinrender` (referência:
   `references/client-153/Angelica2/Angelica3D/{Header,Source}/`).
2. **Reconectar os seams pendentes do skeleton** (documentados em
   `A3DSkeleton.cpp`): a notificação `m_pSkinModel->OnBoneWholeScaleChanges()`
   (removida porque inalcançável com `m_pSkinModel == NULL`) e o
   `SetA3DSkinModel`. Os includes de `A3DEngine` omitidos eram só
   instrumentação de performance — não reponha.
3. Validar com os assets reais já extraídos:
   `artifacts/client153-input/wolf.ski` + `wolf.bon` (o mesmo BON do teste
   do skeleton). O teste de skinning deve dirigir as classes portadas
   (pesos SKI, malha deformada), nunca cópias.
4. Renderizar o personagem animado no browser (WebGL2 host existente em
   `web/client153`), com prova de pixel positiva e negativa.
5. Manifest (`source-manifest.json`), evidência (`docs/evidence/`), update do
   `docs/CLIENT153-PORT.md`, commit autônomo, push, conferir CI.

Depois: shell do cliente (loop/input/tempo/arquivo/áudio → tela de login),
rede (contrato do PR #17, sem simular regras do servidor), terreno/mundo +
streaming, entrega Cloudflare, varredura final. O checklist completo está no
plano do goal.

## Método validado (use exatamente este)

1. Copiar verbatim do `references/client-153` (transcodificar comentários
   GB18030→UTF-8 quando necessário; ver padrão nos commits anteriores).
2. Adaptar **só no erro real de compilação**; documentar cada adaptação no
   próprio arquivo (comentário `//\t...`) e no manifest.
3. Teste que falha antes da correção quando for defeito original.
4. Validar nas **três configurações**:
   ```bash
   export PATH=/home/nonexphere/.local/share/pw-toolchains/cmake-4.4.4/bin:$PATH
   cmake --build build/client153-native --parallel 2 && ctest --test-dir build/client153-native --output-on-failure
   source /home/nonexphere/.local/share/pw-toolchains/emsdk/emsdk_env.sh
   emcmake cmake -S . -B build/client153-wasm-tests -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build/client153-wasm-tests --parallel 2 && ctest --test-dir build/client153-wasm-tests --output-on-failure
   cmake --build build/client153-sanitize --parallel 2 && ctest --test-dir build/client153-sanitize --output-on-failure
   ```
5. Provas com dados reais: `artifacts/client153-input/` (wolf.bon, wolf.ski,
   cabinet.mox/dds) e o PCK autorizado via
   `/home/nonexphere/workspace/pw/clients/client-1.5.3-v145-pwbr/app/`.
   Assets nunca entram no Git.
6. Commit autônomo + push + `gh pr checks 23`.

## Defeitos originais já corrigidos (não reintroduza)

- Handles de 32 bits: `ALISTPOSITION` no lugar de `DWORD` (A3DCoordinate,
  A3DTrackData::m_dwDelFlag1, shared-file handles → `uintptr_t`).
- `memset`-sobre-classe → zeramento explícito (HOOKDATA, BONEDATA, matrizes).
- Ordem de inicialização de membros (hook, bone, controllers, AABB, ATempMemMan).
- AString em varargs `%s` → cast explícito `(const char*)` (UB no original).
- `stricmp`/`strlwr`/`GetTickCount`/`_fileno`/`_chsize`/`ZeroMemory`/`LPBYTE`
  → equivalentes portáveis (padrão já estabelecido nos commits).
- Artefato incompilável do `AFilePackage.h` (FILEHEADER duplicado →
  FILEHEADER64; layout 32-bit é o ativo do v145).
- Constantes de formato provadas contra pacotes reais: guard0 `0xFDFDFEEE`,
  guard1 `0xF00DBEEF`, mask `0xA8937462`, check `0x59374231`.

## Armadilhas conhecidas (aprendidas nesta sessão)

- **Base dir**: `AFileImage`/`AFI` resolvem caminhos via `g_szBaseDir`
  (vazio até `af_Initialize`/`af_SetBaseDir`). Testes chamam
  `af_SetBaseDir(".")`. Caminhos absolutos POSIX são reconhecidos.
- **Lock re-entrante**: `CRITICAL_SECTION` do Windows é re-entrante; o seam
  usa `std::recursive_mutex` (o track manager aninha o lock — `std::mutex`
  plano **deadlocka**).
- **Emscripten zlib**: `-sUSE_ZLIB=1` no CMakeLists raiz (não `-lz`).
- **`#define new A_DEBUG_NEW`**: clang exige `-Wno-error=keyword-macro`
  (já nos 3 targets que compilam fontes originais).
- **Truncamento de caminho**: `-Wno-error=format-truncation` no
  `client153_file` (joins limitados por design — o original transbordava).
- **`-pedantic`** é omitido nos targets com unions anônimas MSVC
  (vetores/matriz originais), igual ao `client153_math`.
- **Relógio**: origem estática (primeira chamada = 0); teste só afirma
  monotonicidade, nunca magnitude.
- **CRC32 do AAssist**: tabela verificada byte a byte contra a referência —
  IDs de arquivo dependem dela.

## Coordenação

- Servidor (PR #17) e validação Wine: worktrees de outros agentes — não
  modifique; consuma contratos read-only.
- Card do agente em `~/.agents/running/`: atualize o seu ao assumir.
- Sem force-push, sem self-merge; `main` é ação humana ao final.
- Orphan `tests/client153/ski_loader.cpp`: seu WIP antigo, decide você.

---

# Atualização do handoff — Grok → Codex (2026-10-26, segunda rodada)

Estado até `489fa86`. A campanha compile-first avançou de 66 para **109/787 TUs
compilando para objetos WASM** (probe `objects-cycle8.json`).

## O que mudou desde a primeira rodada do handoff

- **`80616e3`** — fix do track-set compartilhado do skeleton + `AWString`
  original completo (char16_t → depois trocado para `wchar_t` com
  `-fshort-wchar`).
- **`0b78386`** — **source adaptada integral** (2065 arquivos do jogo em
  `src/client153/Element/`, hashes no manifest), probe com `--adapted-root`
  + includes espelhados, e os seams: `-fshort-wchar` (CRT wide 16-bit de
  propriedade do port — o CRT do sistema lê 32 bits; `static_assert`
  garante o flag), `Windows.h` real (tipos, `CRITICAL_SECTION` re-entrante,
  `Interlocked`, ticks, `MSG`/`WM_*`/`VK_*`, handles GDI), `d3d8.h`/`d3dx8.h`
  (assinaturas documentadas das interfaces), forwarders de caixa
  (`StdIO.h`, `basetsd.h`, `EC_Observer.h`, `ExpTypes.h`, `aarray.h`,
  `Vector.h`, `a3dtypes`...), `sdk_forward.h` (A2DSpriteItem,
  A3DTerrain2Render), transcodificação GB18030 de 764 arquivos,
  typedefs/asm dos headers do jogo corrigidos.
- **`489fa86`** — seams de rede/sincronização: `winsock2.h` mapeado ao layer
  POSIX do Emscripten (WebSocket-backed) e `winsync.h` com as primitivas
  reais de espera (eventos cv, semáforos contadores, threads com join
  cronometrado). `ws2tcpip.h` para a camada de defesa.

## Estado atual do probe (cycle 8, 109/787)

### Retomada: diagnóstico da rede

O adaptador Winsock anterior redeclarava funções POSIX e usava constantes
Winsock diretamente nas chamadas POSIX. Isso causava erros de compilação e
opções de socket incorretas. O adaptador agora usa as constantes do sistema e
desambigua as chamadas; o teste nativo `client153_winsock` verifica troca de
bytes, `SO_TYPE` e leitura não bloqueante. Os 17 testes nativos e os 16 testes
WASM existentes passaram; a sintaxe do adaptador foi verificada com `em++`.
Isso não é uma prova de rede no navegador. O `select(0, ...)` original foi
corrigido para incluir os descritores POSIX e `client153_gnet_poll` confirma
eventos de leitura em sockets nativos; seu mecanismo de despertar ainda usa
UDP local. A adaptação do loop e o gateway WebSocket permanecem pendentes.

O probe agora seleciona a cópia adaptada também para `--entry`, com fallback
para a referência quando ela não existe. Antes disso, esse modo podia reportar
erros de um arquivo diferente daquele testado com `--all`.

Na última varredura anterior à correção do GNET, 109 de 787 unidades passaram
na checagem sintática; 146 paravam em `CompactUINT`. Sua definição agora
precede o uso e o byte swap usa instruções portáveis com larguras explícitas.
Uma unidade desse grupo avançou até os erros seguintes de threads e
configuração. O teste `client153_gnet_marshal` confere os bytes de uma
codificação e o round-trip de uma sequência em nativo e WASM; ele não é uma
validação do protocolo ponta a ponta. Os cabeçalhos `TPBase.h` e `TypeList.h`
também receberam correções de sintaxe, cobertas por `client153_typelist`.
Após essas mudanças, passaram 19 testes nativos e 18 WASM. A nova varredura
completa ainda precisa ser consultada para medir o impacto total.

Na varredura seguinte passaram 110 de 787 unidades sintáticas. A remoção do
erro de `CompactUINT` revelou outros bloqueios: codificação (182), referências
de tamanho incompatíveis (123) e `CreateEvent` (83). O primeiro erro de
codificação inspecionado vem de um cabeçalho AUI da referência, em GB18030.
Os 48 cabeçalhos AUI foram copiados da mesma revisão fixada com hashes no
manifesto; sete foram transcodificados de GB18030 para UTF-8. O prelude do
probe prioriza os cabeçalhos adaptados. A unidade que reproduzia o erro de
codificação agora avança até erros de tipos de tamanho; a contagem global após
essa mudança ainda não foi medida.
Os testes de serialização foram ampliados para números de ponto flutuante,
strings, UTF-16 e octetos; correções de acesso e largura passaram com ASan e
UBSan, além dos testes nativos e WASM. Ainda não há conexão validada.

Depois da adaptação AUI e dos contadores locais de itens, a varredura
sintática passou em 112/787; 283 unidades passaram a parar em `min` sem
namespace e 86 em `CreateEvent`. Essas duas causas foram adaptadas em seguida,
mas a nova contagem global ainda está em andamento. O GNET usa eventos e
semáforos existentes e cria threads destacadas; `client153_gnet_thread` testa
eventos e semáforo nativamente. O pool, o despertar via UDP e o
transporte WebSocket não foram validados no navegador. Passaram 20 testes
nativos e 18 WASM; as regras de itens alteradas ainda não foram testadas no
gameplay.

O ciclo seguinte passou em 120/787 unidades na verificação sintática; os
maiores grupos eram nomes de diálogos (175) e configuração GNET (158). Foram
adicionadas declarações antecipadas das classes já existentes e adaptada a
leitura de configuração; `client153_gnet_conf` verifica a leitura real e a
busca sem distinguir maiúsculas. Agora passam 22 testes nativos e 18 WASM.
O ciclo atual compila objetos WASM para medir falhas posteriores à sintaxe;
esses números não representam um executável ligado nem um cliente jogável.

A primeira compilação de objetos deste ciclo gerou 122/787 objetos WASM;
280 unidades paravam na ambiguidade de `recv`. As chamadas GNET foram
adaptadas, e `client153_gnet_stream` usa sockets locais e a sessão original
para testar envio e recebimento em nativo, também com ASan/UBSan. O mesmo
teste foi compilado para objeto WASM, sem execução de transporte no browser.
O probe usa o zlib do Emscripten em vez do cabeçalho do SDK Windows. Passaram
23 testes nativos e 18 WASM; outra compilação de objetos está em andamento.

Essa compilação gerou 141/787 objetos; 119 unidades paravam primeiro nos
mapas de comandos e eventos AUI. `scripts/qualify-client153-aui-maps.py`
qualifica os métodos com a classe de cada mapa, preserva comentários e pode
ser reaplicado sem alterações. A adaptação abrangeu 2.023 entradas em 336
arquivos; a unidade usada para reprodução avançou para `a_sprintf`. Uma nova
compilação de objetos está em andamento para medir o efeito global. A
adaptação dos mapas ainda não foi validada em gameplay.

Após os mapas, 192/787 unidades geraram objetos WASM. O erro mais frequente
era a sintaxe de iteradores dependentes em `ec_observer.h`; ela foi corrigida
e `client153_observer` verifica registro, notificações e remoção em nativo e
WASM. Foram ajustados nove includes que dependiam de resolução sem distinção
de maiúsculas do Windows. Passaram 24 testes nativos e 19 WASM; o build de
objetos seguinte está em andamento.

Na compilação seguinte foram gerados 218/787 objetos WASM. Os primeiros
erros de 77 e 47 unidades apontavam para declarações ausentes de `CECScene`
e `A3DAdditionalView`; elas agora referenciam as classes reais existentes.
O layout de `D3DCAPS8` também foi corrigido e conferido contra o header local
do Wine: `client153_d3d8_caps` verifica tamanho e offsets em nativo e WASM.
A unidade verificada avançou até outro include com caixa diferente
(`A3DGdi.h`). Passaram 25 testes nativos e 20 WASM; a integração gráfica
real e a contagem após essas correções ainda estão pendentes.

O ciclo seguinte gerou 239/787 objetos WASM. Para o erro de cópia de planos,
`AArray::SetAt` aceita uma referência constante e o teste cobre a cópia de
uma coleção constante em nativo e WASM. Foram adicionados `PVOID`, uma
declaração opaca de `LPEXCEPTION_POINTERS` e comparação de strings sem
distinguir maiúsculas, além da correção de cinco includes `A3DGDI.h`. A
declaração de exceção não implementa SEH. Passaram 28 testes nativos e 23
WASM; a próxima medição de objetos está em andamento.

Para reproduzir os testes, configure os builds nativo e WASM conforme o
onboarding e execute `cmake --build <build>`, seguido de
`ctest --test-dir <build> --output-on-failure`. O teste Winsock é nativo;
ele não comprova transporte WebSocket.

Grupos restantes, em ordem: `IOMap`/`SOCKET` na cadeia GNET (os headers
GNET usam `GNET::SockAddr` e `IOMap` próprios — ver `gnpollio.h`), `stricmp`
(→ `strcasecmp`), `_stat` incompleto (→ `struct stat`), `size_t` vs
`unsigned int` (114), `typename` no TypeList.h:173 (31 — defeito
dependent-name no mesmo arquivo), `D3DCAPS8` (20 — já adicionado ao seam,
recompilar), `A3DFrustum::PLANE` (15 — temporário ligado a referência não-
const, extensão MSVC), `WTYPES.h` (10 — tipos OLE), `eh.h` (9 — SEH).

## Próximos passos sugeridos (na ordem)

1. **Cadeia GNET** (~142 TUs): os headers GNET (`gnpollio.h`, `gnnetio.h`,
   `gnactiveio.h`, `gnmarshal.h`) precisam dos tipos de socket do seam
   (SOCKET/sockaddr já em `winsock2.h` — verificar o que cada header usa;
   `IOMap` é tipo próprio do GNET). `stricmp` → `strcasecmp`, `_stat` →
   `struct stat`. A análise winsock completa está no histórico do
   subagente e em `docs/evidence/client153-adapted-tree.md`.
2. **size_t vs unsigned int** (114 TUs): assinaturas que passam `unsigned
   int` onde `size_t&` é esperado — provavelmente `a_malloc`/containers;
   encontrar o site exato com o probe (`-ferror-limit` alto).
3. **TypeList.h:173** (31 TUs): `typename` faltando em dependent names
   (`Observers::iterator`) — fix direto na cópia adaptada.
4. **A3DFrustum::PLANE** (15 TUs): temporário ligado a referência não-const
   — adaptar os call sites ou a assinatura na cópia adaptada.
5. **WTYPES.h/eh.h** (19 TUs): tipos OLE e SEH — ciclos próprios.
6. Continuar os ciclos: probe → classificar → corrigir a dependência mais
   bloqueadora → recompilar. Cada milestone = um commit com evidência.

## Infraestrutura e armadilhas (atualizado)

- **Builds no volume de clients** (quota do workspace em 96%):
  `PW_BUILD=/home/nonexphere/workspace/pw/clients/build/pw-browser-port`;
  builds: `client153-native`, `client153-wasm-tests`, `client153-sanitize`,
  `client153-original-objects` (objetos do probe).
- **`-fshort-wchar` obrigatório** em todos os targets e no probe (o
  `static_assert` em `utf16.h` falha alto sem ele). O CRT wide do SISTEMA
  está errado para 16 bits — usar o seam `widestr` (`wide_strlen`,
  `wide_vformat` etc.), nunca `wcs*` da libc.
- **Include shadowing**: TUs da referência resolvem includes no próprio
  diretório primeiro — por isso a árvore adaptada inteira foi copiada e o
  probe espelha os includes. Novos fixes vão na cópia adaptada
  (`src/client153/Element/`), nunca na referência.
- **Probe**: `scripts/probe-client153-project.py --source
  /home/nonexphere/workspace/pw/pw-web/references/client-153 --emxx
  .../em++ --port-root src/client153/Angelica2 --adapted-root
  src/client153 --output artifacts/... --all --jobs 8 --object-dir
  $PW_BUILD/client153-original-objects`. O prelude força `Windows.h`,
  `sdk_forward.h` e os widgets AUI (`AUIListBox.h`, `AUIComboBox.h`,
  `AUIEditBox.h`, `AUILabel.h`, `AUIStillImageButton.h`, `AUICheckBox.h`,
  `AUIImagePicture.h`, `AUIDialog.h`) — a cadeia de fontes D3D fica fora
  do prelude de propósito.
- **Subagentes funcionam bem** (análise read-only + implementação com o
  modelo DeepSeek `cf-deepseek-v4-flash`): usar para transcodificações,
  adaptações mecânicas e investigações; o thread principal cuida dos seams
  de plataforma (arquivos quentes).
- **Servidor (PR #17)**: o agente paralelo confirmou servidor completo no
  ar (8 daemons + gs com mundo v145) e handshake do cliente fixado
  (versão 0x10503 + edition fixada) — consumir o contrato dele no
  milestone de rede.
- **Quota de disco**: workspace em 96% (liberado com `git gc`); builds
  grandes vão para o volume de clients.

---

# Atualização do handoff — Grok → Codex (2026-10-27, terceira rodada)

Estado até `98eb2a0c` (tudo pushed, árvore limpa). Campanha compile-first:
**572/787 TUs do jogo compilando para objetos WASM (72,7%)** — desde a
segunda rodada do handoff (281), passando por 492/504/562.

## Commits desta rodada

- `483a1cb9` — SEH→try/catch (12 arquivos; subconjunto de exceções C++
  preservado, hardware faults trapam — limitação documentada), seam `eh.h`
  (layouts Win32 de exception record), fixes de grupos pequenos
- `2681ce95` — ctor/operator exato de cor no GFX_PROPERTY (o `unsigned long`
  do original era 32-bit; A3DCOLOR=unsigned int ganha match exato — 32 TUs
  de ambiguidade resolvidos)
- `a9722728` — **sources da lib AUI** copiadas+transcodificadas (94
  arquivos; o inventário de LIGAÇÃO provou que o jogo precisa delas — 12
  símbolos AUI indefinidos), seams tlhelp32/vdmdbg/unknwn/CreateFileA,
  probe com `--extra-project`, fixes de dependent-names na família SDB
  (subagente glm-5.3 com verificação por objeto real)
- `98eb2a0c` — for-scoping leaks (7 TUs), temporários dos providers wiki
  (9 TUs — **achado verificado: os setters clonam sincronamente**, então
  locals nomeados são o padrão correto, membros não), constantes de
  disposition de arquivo, prelude preferindo as cópias adaptadas da AUI

## ACHADO CRÍTICO: o inventário de ligação

Liguei os ~526 objetos (na época) com as libs WASM do port:
**apenas 12 símbolos indefinidos, todos do jogo/AUI** (nenhum de
plataforma — os seams são header-inline). Os 12: métodos de
AUIDialog/AUIImagePicture/AUIManager/AUIObject/AUIStillImageButton (a lib
AUI, agora sob o probe) e CECHostPlayer::SummonPet/RecallPet/IsOperatingPet
(TUs que ainda não compilam). **Conclusão: a ligação do cliente é viável
com os seams existentes** — o próximo marco é ligar o entry + fecho.

## Estado do probe (cycle 39, 572/787)

Grupos restantes, em ordem: `atlbase.h` (10 TUs — **DOIS agentes morreram
por max_tokens nesta tarefa**; o inventário da superfície ATL está no
PWAtl.h — CComPtr/CComBSTR/CAxWindow etc.; fazer INCREMENTAL, um símbolo
por vez, ou inline), varargs AWString (6), `Psapi.h` (6 — o próximo header
da cadeia de defesa; mesmo padrão honesto do tlhelp32), `EL_GrassLand.h`/
`A3DGfxEx.h` (case variants), `AString` em format (3 — cast), temporários
restantes de search commands (o padrão de locals nomeados se aplica —
verificar se o setter clona), `PCMWAVEFORMAT` (struct do dsound.h),
`AList2` template (include order).

**Projeto AUI (extra-project): 0/46** — o prelude foi corrigido para as
cópias adaptadas (commit 98eb2a0c) mas o probe do AUI ainda não re-rodou
com essa correção; o próximo cycle vai reclassificar. Os TUs AUI podem ter
os mesmos padrões do jogo (case/for-scoping/etc.).

## Próximos passos (na ordem)

1. **Probe cycle 41** (com o prelude corrigido) para reclassificar o AUI e
   confirmar os grupos restantes do jogo.
2. **atlbase.h** (10 TUs): fazer incremental/inline — ler o PWAtl.h, um
   símbolo por vez.
3. **Psapi.h** (6): mesmo padrão do tlhelp32 (enumeração de processos →
   falha explícita ERROR_CALL_NOT_IMPLEMENTED).
4. **Ligação** (o próximo MARCO): com 572+ objetos, ligar o
   ElementClient.cpp entry + libs do port + objetos AUI num módulo
   carregável; o inventário de símbolos faltantes guia os próximos
   adaptadores. Os 12 conhecidos somem quando os TUs deles compilarem.
5. Depois: as libs Angelica (engine/mídia) — o probe só cobre ElementClient
   + AUI; o resto do cliente (Angelica2/*) compila via CMake targets
   existentes + os novos projetos vcproj.

## Armadilhas desta rodada (novas)

- **Subagentes de seam**: DOIS morreram por max_tokens (DeepSeek e glm) na
  tarefa do atlbase — tarefas com muita leitura precisam de escopo
  estrito ("leia SÓ este arquivo") ou inline.
- **Meu bug de regex**: um comentário `//` inline engoliu um `);` de
  default-argument (A3DPlants.h) — ao editar inline, prefira linhas
  separadas para comentários de adaptação.
- **Padrão de temporários VERIFICADO**: WikiSearcher::SetContentProvider/
  SetSearchCommand CLONAM sincronamente (WikiSearcher.cpp) — os
  "temporários pendurados" do original eram consumidos na hora; locals
  nomeados são o fix correto, membros quebram (copy assignment deletado).
- **Scan de case variants**: cuidado com a direção — um scan case-
  insensitivo pode criar forwarders apontando para outros forwarders (loop
  circular). Verificar a direção contra o arquivo REAL antes de criar.
- **O volume de clients foi reconstruído** pelo agente do servidor
  (root-owned, sem permissão de escrita, assets autorizados não estão
  mais lá) — builds voltaram para o workspace (8,6G livres); testes que
  dependem de assets pulam por design.

---

# Atualização do handoff — Grok → Codex (2026-10-27, quarta rodada)

Estado até `8636c6de` (tudo pushed, árvore limpa). Campanha: **578/787 TUs
do jogo (73,4%)** + AUI 25/46 no probe com `--extra-project`.

## Commits desta rodada (pós-quota do Codex)

- `02d66f0e` — WIP final do Codex validado (constantes D3DFVF_* com
  static_assert) + o documento de retomada do Grok
  (docs/CLIENT153-RESUMPTION-GROK.md)
- `f16ed51f` — **o seam ATL completo** (a frente de 10 TUs que dois
  subagentes não conseguiram): atlbase.h (CComPtr/CComQIPtr/CComBSTR/
  CComVariant/CComObject/CComObjectRootEx/CComSingleThreadModel/
  IDispatchImpl/IDispEventImpl, macros COM map/sink map, ativação COM via
  registry de adaptadores — classes desconhecidas falham explicitamente),
  atlwin.h (CWindow/message maps/CWindowImpl com o window-proc threading),
  atlhost.h (CAxWindow/AtlAxGetControl), EXDISPID.H/exdisp.h (IWebBrowser2,
  IDispatch, MSHTML document/element/style/input, GUIDs como accessores,
  monikers de download), shlobj/shlwapi/htmlhelp seams. Os 10 TUs passaram
  a barreira: 2 compilam limpos, 8 avançaram para erros individuais.
- `8636c6de` — **o seam Psapi.h** (6 TUs): enumeração de processos falha
  explicitamente com ERROR_CALL_NOT_IMPLEMENTED (o padrão tlhelp32); o bloco
  RDTSC inline-asm reporta zero (API de ciclos pendente). Superfície Win32:
  MEMORYSTATUS, SYSTEM_INFO, OSVERSIONINFO, headers PE, mutexes reais,
  diretório de trabalho, atributos de arquivo, cursor, SendMessage,
  LoadLibrary/GetProcAddress (falha explícita — sem DLLs no browser).

## Próximos passos (na ordem)

1. **Long tail dos 8+5 TUs das frentes atlbase/Psapi** — erros individuais:
   EnumWin32Pro.cpp:84 "expected catch" (um `__except` residual do meu regex
   SEH — verificar a forma), Process.cpp:11 identificador, replaceapi.cpp:15
   GetModuleHandle (a forma sem sufixo — o seam tem GetModuleHandleA/W),
   tracer.cpp:80 mais inline-asm MSVC (adaptar como o RDTSC), shlobj.h
   E_FAIL (a ordem de include — o define E_FAIL precisa vir antes do uso),
   ElementClient.cpp via EC_WinAPI.h (ShlObj.h resolvido, verificar o
   próximo erro).
2. **Grupos restantes do probe** (objects-resume2.json): varargs AWString
   (8+3+3), Wininet.h (4), EL_GrassLand.h/A3DGfxEx.h (case variants),
   A3DVECTOR3 temporários (3), AList2 (3), expected identifier (4).
3. **O PRIMEIRO LINK REAL** (o marco m-link): com 578+ objetos, ligar o
   entry ElementClient.cpp + libs do port (WASM builds em
   build/client153-wasm-tests) + objetos AUI num módulo carregável. O
   trial-link anterior deu 12 símbolos indefinidos, todos do jogo/AUI,
   zero de plataforma. Os 12 somem quando os TUs deles compilarem.
4. Depois: boot no Chromium headless (m-boot), caminho com servidor
   (m-server), entrega (m-delivery).

## Armadilhas desta rodada (novas)

- **Structs antes das definições**: ao adicionar blocos ao Windows.h,
  verificar a posição — LPMSG/PE headers quebraram porque referenciavam
  tipos definidos depois. O Windows.h tem uma ordem interna (tipos →
  structs → funções inline → includes de seams no fim).
- **replace() silencioso**: um `s.replace()` sem match retorna a string
  original sem erro — verificar `assert old in s` ANTES de confiar (o bloco
  CSIDL/shlobj "foi adicionado" duas vezes sem ter sido).
- **Comentário inline engolindo `);`**: de novo no A3DPlants — comentários
  de adaptação SEMPRE em linha separada.
- **Chamada de macro vs função**: os GUIDs como accessores exigem `()` em
  cada call site (IID_IDispatch() etc.) — o Codex vai encontrar os que
  faltam pelos erros "cannot take the address of an rvalue".

## Comparação do contrato RoleInfo

O cliente compilado do projeto 1.5.3 seleciona o `rpcdata/roleinfo` da
referência; o servidor v145 possui outra definição. Para comparar diretamente
os bytes de uma `RoleInfo` padrão, sem copiar dados de usuários ou alterar o
servidor, use os caminhos locais das árvores autorizadas e o Emscripten:

```sh
python3 scripts/compare-client153-role-wire.py \
  --reference-root "$PW_CLIENT153_REFERENCE" \
  --server-root "$PW_SERVER153_ROOT" --emxx "$(command -v em++)"
```

O comando compila a mesma fonte de teste contra as duas definições, usa o
compilador de servidor de 32 bits e Node, e retorna 1 quando os bytes divergem.
Na comparação local de 2026-10-27, o cliente produziu 60 bytes e o servidor
59; os 59 primeiros coincidem, e o cliente acrescenta o `rank` vazio. Esse
teste é um diagnóstico de um registro padrão: não comprova compatibilidade do
restante do protocolo nem criação de personagem ou conexão com o servidor.

---

## Rodada 5 (2026-10-28, Grok → Codex): m-link em andamento

Head `eae4ce02`. Compilação 726/833 (680/787 jogo + 46/46 AUI, probe
cycle45 limpo). Regressões: native 31/31, wasm 26/26, sanitize 31/31,
python 21/21. O comentário de handoff completo com o worklist do link
(58 símbolos decompostos), os comandos de reprodução e os 10
armadilhas conhecidas está no PR #23 (issuecomment-6466418369).

Estado novo nesta rodada que o Codex precisa saber:

1. **O `references/` DESTE worktree é placeholder vazio** — as fontes
   upstream reais estão em
   `/home/nonexphere/workspace/pw/pw-web/references/client-153`. Toda a
   camada GfxCommon (EC_Model 6709 linhas + 7 TUs) e o LuaWrapper
   (ScriptValue.cpp) foram portados de lá nesta sessão; nada é
   binário-only.
2. **Novos targets CMake**: `client153_gfx_original` (9 objetos: os TUs
   Gfx + ScriptValue) e o `client153_engine_original` ampliado (20
   objetos: +A3DGeometry, A3DTerrain2, A3DTerrain2Trace — o
   GetPosHeight mora AQUI, não no A3DTerrain2.cpp —, A3DTerrainWater,
   A3DEngine). O target gfx PRECISA do prelude de widgets completo
   (dedupe das cópias bugadas de AString.h/AArray.h do acsdk).
3. **O link de fecho completo** roda com
   `python3 scripts/link-client153-full.py --error-limit 300` (depois
   de buildar os dois targets). Faltam 58 símbolos: ~20 são objetos
   stale das 4 TUs fechadas após o probe cycle45 (EC_Utility,
   defence/defence, EC_CustomizeMgr, EC_IvtrItem — um re-probe resolve);
   o resto são as próximas TUs (Wiki, Skills, MCDownload, OfflineShop,
   LuaState/LuaUtil — o LuaBind::Call foi adiado de propósito no
   ScriptValue.cpp —, A3DGFXElement.cpp para o trio g_Gfx*, e os bits
   A3DDevice::SetFaceCull/A3DWireCollector::Add3DLine/g_vAxisY).
4. **Armadilhas novas**: a ordem interna do Windows.h mordeu 4× nesta
   sessão (HIMC, ERROR_CALL_NOT_IMPLEMENTED, GUID, LARGE_INTEGER);
   LARGE_INTEGER/ULARGE_INTEGER/SYSTEM_INFO agora usam o layout union
   documentado (struct anônima — pragma pedantic waivado com a
   justificativa no lugar); min/max com tipos mistos é ambíguo com os
   novos overloads (cast explícito); o template swap global legado em
   A3DGFXKeyPoint.h colide via ADL (shim TU-local no A3DGFXExMan.cpp;
   remover no header é o fix de dono).
5. PW23-001/002 (winsync thread lifetime + WinMutex ownership)
   RESOLVIDAS com o teste novo `winsync_semantics` (commit a2a9c251).

### Gerenciador original de shaders — compilação incremental

`A3DShaderMan.cpp` da referência 1.5.3 foi incluído no alvo
`client153_engine_original`, com procedência no manifesto. O cabeçalho
`A3DFuncs.h` volta a declarar os construtores de declarações de vértices usados
pelo gerenciador; suas implementações e as operações gráficas de shaders ainda
precisam ser ligadas e portadas. `MAX_FVF_DECL_SIZE` é 20 para Direct3D 8.

Reprodução: `cmake --build build/client153-wasm --target client153_engine_original -j 4`;
`cmake --build build/client153-native -j 4 && ctest --test-dir build/client153-native --output-on-failure`.
Resultado: objeto WASM compilado e 31 testes nativos passaram. Esta etapa não
comprova link completo, execução de shaders nem jogo no navegador.

Os dois construtores originais de declarações para tipo de vértice e morph
foram restaurados em `A3DFuncs.cpp`; testes nativo e WASM verificam os tokens.
O overload por FVF continua pendente de `D3DXDeclaratorFromFVF`, assim como a
execução gráfica das declarações. O diagnóstico `python3 scripts/link-client153-full.py
--error-limit 0` ainda falhou com 7.264 ocorrências de símbolos indefinidos;
esta contagem inclui referências repetidas e não representa funcionalidades.

### Dependências reais dos shaders

A compilação isolada das fontes originais `A3DVertexShader.cpp` e
`A3DPixelShader.cpp` com o mesmo comando de `client153_engine_original` e o
include adicional `Angelica2/Output/afsdk/include` falhou em APIs ainda não
portadas: criação e destruição de shaders em `IDirect3DDevice8`,
`D3DXCreateBuffer` e `D3DXAssembleShader`; a fonte de vértice também requer
`D3DUSAGE_SOFTWAREPROCESSING`. O gerenciador compilado não oferece essas
implementações. Os shaders do cliente incluem código Direct3D de vértice e
pixel, de modo que executar a engine requer uma estratégia real para seus
programas e recursos no WebGL, não apenas acrescentar assinaturas para o build.
A conversão do overload FVF continua pendente de comportamento Direct3D 8
verificado.

O inventário de assembly executado contra `element/shaders.pck` do cliente
local com `python3 tools/assets/shader_inventory.py <caminho>/shaders.pck`
identificou 155 entradas `vs.1.1`, uma `vs.1.0`, 74 `ps.1.1`, 91 `ps.1.4`
e cinco `ps.2.0`. São contagens de entradas, não programas únicos; o scanner
considera extensões `.txt`, `.vsh` e `.psh` e não prova que todas as entradas
sejam carregadas em gameplay. O inventário revela instruções de textura,
modificadores, coissue, `phase` e declarações que uma tradução real precisa
tratar. Nenhum conteúdo proprietário é extraído nem incluído no repositório.
Reproduzir testes: `python3 -m unittest discover -s tests/assets`.

### Objetos individuais do cliente

O probe aceita `--object-dir` sem `--all`: compila somente `--entry` no
mesmo diretório de objetos usado pelo link, removendo primeiro um objeto
anterior para que falhas não preservem uma compilação obsoleta. Por exemplo,
use os argumentos de probe acima, `--entry EC_Utility.cpp`, `--output` para um
relatório temporário e `--object-dir` para o diretório de objetos. Também
foram compilados `EC_CustomizeMgr.cpp` e `EC_IvtrItem.cpp`. No diagnóstico
local o link passou a incluir 767 objetos e falhou com 4.935 ocorrências de
símbolos indefinidos; essas contagens são referências repetidas, não testes
de funcionalidade. Objetos gerados são locais e não são versionados.
Os testes do probe rodam com `python3 -m unittest discover -s tests/client153
-p test_project_probe.py`.

A unidade original `EC_OfflineShopCtrl.cpp` passou a compilar para WASM após
corrigir o nome de `ALog.h` para a capitalização real do arquivo. Para
reproduzir, executar o probe individual descrito acima com
`--entry EC_OfflineShopCtrl.cpp`. O link diagnóstico local com esse objeto
falhou com 4.898 ocorrências de símbolos indefinidos; a compilação não valida o
funcionamento da loja ou do cliente. `EC_MCDownload.cpp` permanece bloqueado
por APIs Win32 de named pipes e overlapped I/O que não existem no browser.

`A3DOrthoCamera.cpp` original foi incluído na compilação WASM após a
implementação de `D3DXMatrixOrthoOffCenterLH` conforme a fórmula publicada pela
Microsoft (https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixorthooffcenterlh).
Os testes `client153_math` em nativo e WASM verificam os cantos e a faixa de
profundidade da projeção. O link diagnóstico com a câmera ainda falhou; essa
compilação não prova renderização ou inicialização do jogo no navegador.

`A3DMaterial.cpp` e `A3DMathUtility.cpp` originais foram adicionados ao
alvo `client153_engine_original`, com hashes de origem no manifesto. O alvo
compilou em WASM e a suíte nativa passou 31/31. O link diagnóstico com 771
objetos e nove bibliotecas ainda falhou com 4.851 ocorrências de símbolos
indefinidos; não houve prova de renderização ou boot. O compilador sinaliza
`ZeroMemory` sobre `A3DMATERIALPARAM` não trivial no material original; a
semântica deve ser revisada antes de tratar o código como validado em runtime.

A capacidade WebGL2 mínima não exige mais S3TC; o runtime verifica a extensão
quando carrega uma DDS comprimida e informa erro explícito se não disponível.
Em Chromium/SwiftShader, com `getExtension` de S3TC desativado, o visualizador
renderizou o modelo original usando uma DDS BGRA sintética (29.187 cores,
GL_NO_ERROR) e rejeitou a DDS DXT3 original com a mensagem esperada. O teste
`tests/client153/browser-uncompressed-dds.cjs` aceita `PW_DISABLE_S3TC=1` e
`PW_EXPECT_COMPRESSED_SCENE=1` para repetir os dois caminhos. Esse resultado é
do visualizador, não prova boot do jogo ou fallback para DDS comprimida.

A revisão do PR levou a confirmar que PW23-001 e PW23-002 têm resolução no
código e testes em `winsync_semantics.cpp`, embora `.agents/issues/README.md`
ainda os liste como abertos. Na mesma revisão, `_beginthreadex` ignorava
`CREATE_SUSPENDED`, opção usada por `EC_GetHostByName.cpp`; agora aguarda
`ResumeThread`. O teste de regressão falhou antes da mudança e passou nos
builds nativo e ASan/UBSan: `cmake --build build/client153-native --target
client153_winsync_semantics_test && ctest --test-dir build/client153-native -R
'^client153_winsync_semantics$' --output-on-failure`, repetindo com
`client153-sanitize`. Essa prova não valida pthreads nem DNS no navegador.

`Common/GfxCommon/A3DGFXElement.cpp` original agora integra o alvo
`client153_gfx_original`: conversões explícitas em varargs e propriedades e
limites `DWORD` permitem sua compilação WASM. O link diagnóstico deixou de
relatar as três funções `g_Gfx{Apply,Load,Save}PixelShaderConsts`, mas a nova
unidade também introduziu dependências ainda não resolvidas (4.867 ocorrências
no link local com 772 objetos e nove bibliotecas). Build de Gfx e 31 testes
nativos passaram; estes testes não executam os efeitos ou o jogo no browser.
