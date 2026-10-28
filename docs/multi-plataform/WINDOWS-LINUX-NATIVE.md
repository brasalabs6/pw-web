# Windows e Linux nativos — executável C++ de alta performance (somente futuro)

> **Status: NÃO INICIAR.** Documento para um **segundo tipo de distribuição futura**: cliente Perfect World realmente compilado em executável nativo. Não confundir com o launcher Windows/Linux que abre o **mesmo WASM** do browser. Sem alterações no PR #23, no backend WebGL2, no servidor PR #17 nem no rollout inicial. Veja [ARCHITECTURE.md](ARCHITECTURE.md).

## 1. Escopo e produto

Reaproveitar o código Angelica/ElementClient portado e testado no browser, recompilando para `x86_64-pc-windows` e `x86_64-unknown-linux` (ARM64 desktop apenas por demanda), preservando a lógica de gameplay, protocolo GNET, assets e a compatibilidade de servidor/realm.

- **Launcher WEB inicial (pré-requisito independente):** aplicativo Chromium/Electron/PWA abre o WASM remoto, com um único build web. **Não é parte deste cliente C++ nativo.**
- **Launcher NATIVO futuro:** instala/atualiza binários C++, valida assinatura/hash, aplica patches transacionais e inicia `pw-client.exe` (Windows) ou `pw-client` (Linux).
- **Game nativo:** inicializa Angelica, janela, GPU, áudio, arquivos e transporte de forma local ao SO; sem Emscripten, sem JavaScript obrigatório, sem engine duplicada.

## 2. Arquitetura alvo

```text
                  Angelica + ElementClient C++ compartilhado
          A3D engine / CECGame / AUI / AFile / animação / GNET
                                    |
                     seams de plataforma & render
             +----------------------+---------------------+
             |                      |                     |
       WindowsHost             LinuxHost            BrowserHost*
       SDL3/Win32              SDL3/X11/Wayland      Emscripten
       Win file IO             POSIX file IO        Fetch/VFS
       OpenGL inicial          OpenGL inicial       WebGL2
       WASAPI/SDL audio        ALSA/Pulse/PipeWire  WebAudio
       TCP/TLS transport**     TCP/TLS transport**  WSS relay
             |                      |
           pw-client.exe         pw-client (ELF)
             |                      |
       launcher/updater       launcher/updater
```

* BrowserHost continua sendo a baseline **existente e prioritária**.
** O formato dos pacotes GNET e regras do servidor não mudam; transporte/segurança dependem do contrato de autenticação real, sem pressupor acesso público a daemons internos.

## 3. Fronteiras / componentes

| Módulo lógico | Compartilhado com WASM? | Necessidade do desktop |
|---|---|---|
| Matemática, animação, skeleton, skinning | sim — mesmo código C++ | validar diferenças de FPU/SIMD, floats, endian/layout e frames |
| Mundo, NPC, gameplay, AUI lógico | sim — mesmas regras | desacoplar Win32, pump e render legacy sem cópia funcional |
| PCK/PKX, MOX, SKI, BON, DDS e streaming | sim — parsers / hashes / manifest | provider de arquivos locais + download incremental |
| GNET/protocolo | sim — serialização testada por golden bytes | transport de socket com framing, backpressure e TLS/autenticação |
| Render device | contrato e semântica | backend OpenGL nativo inicial, Vulkan opcional por ADR e benchmark |
| Janela/input | semântica de ações | SDL3, mouse/teclado/IME, gamepad, fullscreen, DPI/multi-monitor |
| Áudio | eventos e mixagem comum quando possível | SDL3 ou backends nativos; recuperar device e troca de saída |
| Launcher/patcher | release manifest/identidade | host desktop assinado, paths seguros e rollback |

**Não adotar D3D8 original como API obrigatória.** O trabalho WebGL2 deve identificar semântica suficiente para backend moderno. Para o primeiro vertical slice nativo, avaliar OpenGL/GLES compartilhável sem introduzir um segundo render pipeline inteiro; Vulkan somente se benchmarks mostrarem benefício que compense manutenção. Windows pode usar D3D11/12 futuramente, mas isso requer ADR separada, não pré-condição.

## 4. Design do runtime

### Lifecycle proposto

```text
NEW -> CREATE_WINDOW -> INIT_GPU -> LOAD_MINIMAL -> LOGIN -> IN_WORLD
                     |                  |            |
                     +---- DEVICE_LOST <-+            |
                     +---- SUSPENDED / PAUSED <--------+
                     |
               SHUTDOWN -> RELEASE_GPU -> RELEASE_IO -> EXIT
```

- SDL3 ou host escolhido cria janela, input e eventos; jogo expõe entrada/tick/render/shutdown, sem loop legado de Windows obrigatório.
- O host processa eventos, tempo monotônico e resize, DPI/alt-tab, minimized/suspend, contexto perdido e hotplug de áudio/controle.
- O render device conserva thread-affinity e lifecycle; o core não acessa `HWND`/`HDC`/`GLX`/`EGL`/`VkDevice` diretamente.
- Inicialização de áudio, rede, assets e engine tem ordem documentada com rollback em falha; evitar waits bloqueando UI e aborts em thread de render.
- Threads podem existir quando justificadas e medidas, mas o comportamento compartilhado deve permanecer equivalente ao WASM.

### Build e toolchain (exemplos futuros, NÃO são comandos disponíveis hoje)

```bash
# Somente após criar toolchains, presets e targets aprovados:
cmake --preset windows-x64-release
cmake --build --preset windows-x64-release --target pw_client_native

cmake --preset linux-x64-release
cmake --build --preset linux-x64-release --target pw_client_native

ctest --preset linux-x64-release --output-on-failure
```

Separar `client_core`/`angelica_shared` como bibliotecas de código de `pw_client_native`, sem reimplementar matemática/arquivo/GNET. Toolchain mínima CMake/Ninja + MSVC/clang-cl (Windows) e GCC/Clang (Linux), SDL3 versionado; distribuição de runtime/ABI e dependências deve ser reproduzível. Testar Windows 10/11 e distribuições Linux selecionadas; Wine **não** substitui a validação Windows.

## 5. ABI e compatibilidade de código original

**O maior risco não é a capacidade de compilar C++:** é que Angelica foi escrita para Windows 32-bit, D3D8, `wchar_t` de 16 bits, COM/ActiveX e tipos de ponteiro/`DWORD` legados.

- Criar fronteiras explícitas de UTF-16/UTF-8/GBK e não propagar `-fshort-wchar` para toda biblioteca nativa Linux; validar ABI da libc e de SDKs.
- Converter casts de ponteiro para inteiros para `uintptr_t`/`intptr_t` e verificar serialização do wire legado de 32 bits.
- Revisar `#pragma pack`, alinhamento, bitfields, enum, varargs e correção de undefined behavior sob ASan/UBSan; golden fixtures nativo↔WASM para geometria, tracks, arquivos e protocolos.
- Substituir operações WinINet, DLL dinâmicas, DSound, registry, named pipes, ATL/COM e media legado por equivalentes reais e testados apenas onde necessários.
- Shaders D3D assembly `vs/ps` não executam diretamente como GLSL; portar o subconjunto observado a partir do renderer web, manter testes de visual/estado e não declarar paridade por simplesmente compilar.

## 6. Pacote desktop e launcher

### 6.1 Cliente

- Windows: executável assinado e runtime bundled quando necessário; investigar MSIX versus instalador tradicional após requisitos de patching e distribuição. Suporte a fullscreen/borderless, DPI, múltiplas GPUs e controles.
- Linux: ELF x86_64 com runtime/libs sob política definida; estudar AppImage/Flatpak/pacote nativo como opções, sem presumir uma única forma que funcione em todas distros. Validar Wayland e X11, mesa/NVIDIA/AMD/Intel e drivers.
- Artefatos separados por `platform` e `architecture` com SHA-256, assinatura, licença e compatibilidade de realm; builds reprodutíveis sem carregar segredos.

### 6.2 Launcher nativo (não confundir com o primeiro launcher WEB)

```text
Install/Update
  -> consultar release assinado
  -> comparar plataforma/ABI/versão/realm
  -> baixar delta/chunks para staging
  -> verificar hash + assinatura + limites + paths
  -> promoção atômica ou rollback
  -> iniciar processo C++ com argumentos controlados
  -> health/diagnóstico sem segredos
```

- Usar mesmo **identificador de release e assets** que a web quando aplicável, mas binários nativos distintos; não executar update em cima de processo aberto; cache e conteúdo deduplicado por hash.
- Não permitir remote code execution via manifest adulterado, path traversal, download HTTP inseguro ou execução de código do servidor. Launcher não embute tokens de produção e não desabilita validação TLS.
- Login/credenciais residem em fluxo seguro com servidor; launcher e cliente devem delimitar tokens, logs e compartilhamento de sessão.

## 7. Estratégia de render/alto desempenho

**Primeiro paridade visual, depois performance.** Validar um mesmo corpus de cenas e personagens entre WebGL2 e OpenGL native com tolerâncias declaradas. Analisar CPU frame time, GPU time, draw calls, mudança de estado, streaming, VRAM, texturas, skinning e stutter. Não afirmar ganhos percentuais antes de benchmark em mesmo hardware, driver, qualidade e cena.

Possibilidades **após medir**: shader cache, pipeline/state cache, batching/material sorting, occlusion/frustum culling, LOD, GPU skinning, uploads assíncronos e cache on-disk; otimizações devem permanecer em `.agents/optimization/` e ser avaliadas por custo/risco. Backend Vulkan pode permitir ganhos em determinados cenários; não é automaticamente mais rápido.

## 8. Matriz de testes e critérios

| Gate | Teste observável |
|---|---|
| D0 | G0 browser jogável do [plano comum](ARCHITECTURE.md) foi aprovado |
| D1 | core/link nativo x64 compila com toolchain limpa em Windows/Linux; CTest/golden fixtures compartilham comportamento WASM |
| D2 | runtime abre janela real e renderiza personagem animado + mundo com renderer nativo; screen comparison e input |
| D3 | autentica com realm real, lista personagens, entra no mundo, move e sincroniza 2 clientes |
| D4 | UI/skills/inventário/voo/combate/quests equivalentes à baseline selecionada, áudio, eventos de janela e sessão |
| D5 | cold/warm start, cache/patch, falha de hash, rollback, lost device, alt-tab, reconnect e log de crash |
| D6 | matriz real de GPU/driver/OS; assinatura, direitos e plano de publicação verificados por humanos |

**Exemplo de QA:** Windows 10/11 (Intel/AMD/NVIDIA) e Linux (Mesa + driver proprietário quando aplicável), com GNOME Wayland/X11, modo janela/fullscreen, mouse/IME/gamepad, telas 16:9 e ultrawide. Medir e documentar hardware e driver reais, não confiar somente em CI/headless.

## 9. Riscos, dependências e regra de parada

| Risco | Mitigação / decisão |
|---|---|
| Refatorar demais e quebrar browser | branch isolada, testes de paridade, rebase cuidadoso, não mudar backend WebGL2 ativo |
| ABI de 32 bits e `wchar_t` | mapeamento explícito, fixtures e sanitizers por host |
| Render D3D8 legado | reaproveitar semântica web validada, não criar emulador total de DirectX |
| Middleware/licenças | auditar dependências e direitos antes de empacotar |
| Linux drivers/distro matrix | escopo progressivo e fallback medido |
| Launcher e cadeia de atualização | assinatura, hashes, isolamento de manifests e rollback |
| Desvio do objetivo atual | congelado até G0 + aprovação humana |

**Definition of Done:** cliente `pw_client_native` jogável e conectado em Windows e Linux, com jogo real, paridade de conteúdo/protocolo, distribuição assinada, testes e observabilidade; launcher inicial WASM continua existindo como alternativa leve. Se comprometer o browser, interromper a frente nativa e manter o port web como fonte de verdade.

