# Mobile nativo — Android e iOS (somente futuro)

> **Status: NÃO INICIAR.** Cliente **realmente nativo** compilado a partir do núcleo C++ Angelica, distinto do APK/WebView do [WEBASSEMBLY.md](WEBASSEMBLY.md). Ativação somente após o gate G0 de [ARCHITECTURE.md](ARCHITECTURE.md): Perfect World WASM efetivamente jogável no browser e decisão humana de abrir nova frente.

## 1. Objetivo e fronteira

Construir um único core de gameplay, animação, mundo, assets e GNET recompilado para **Android ARM64** e **iOS ARM64**, com hosts nativos leves para janela/surface, entrada, áudio, storage, transport e render. Jogador acessa as **mesmas contas, personagens e servidor/realm** da versão web, sob regra de compatibilidade explícita por versão. Nenhuma regra de combate, inventário, missões ou serialização deve ser reimplementada em Kotlin/Swift.

**Não objetivos imediatos:** abrir PRs de cliente nativo, migrar D3D8 agora, substituir a WebView de distribuição inicial, publicar em lojas, prometer FPS para todos os telefones ou usar uma nova engine de terceiros. Um APK que abre URL/WASM **não** satisfaz este documento.

## 2. Arquitetura de execução proposta

```text
              gameplay Angelica C++ compartilhado
         animação • mundo • GNET • AUI lógico • VFS
                            |
                   Mobile Platform API
          +-----------------+------------------+
          |                                    |
     Android ARM64                          iOS ARM64
     NDK / CMake                            Xcode / clang
     Activity / Surface                     UIApplication / UIView
     JNI mínimo                             ObjC++ bridge mínima
     GLES3 inicialmente                     Metal (adapter a aprovar)
     AAudio/Oboe (avaliar)                   AVAudioEngine (avaliar)
     armazenamento app                       sandbox app / On-Demand Resources*
     sockets sob contrato                    Network.framework/BSD sockets
          |                                    |
             transport GNET real + realm de jogo
```

* Não presumir equivalência de assets entre lojas, permissões ou disponibilidade de downloads em background. Escolher APIs após ADR e protótipo.

## 3. Requisitos comuns (contracts futuros)

| Subsistema | Comportamento requerido | Regra de propriedade |
|---|---|---|
| `MobileLifecycle` | create, surface-ready, foreground, background, suspend, resume, terminate | host pausa ticks/áudio, serializa sessão segura quando aplicável e libera GPU sem vazamentos |
| `MobileInput` | multitouch, gestures, teclado/IME, gamepad, orientação e safe areas | normalizar eventos em fila, nunca invocar regras de jogo na thread da UI |
| `MobileGraphics` | buffers, textures, shader permutations, pass/depth/blend, context/device-loss | buffers e texturas recriáveis a partir de assets verificados |
| `MobileAudio` | mixagem, permissão/rota, interrupção por chamada, volume | pausar/resumir com controles de plataforma |
| `MobileStorage` | cache por hash, quotas, escrita atômica, migração e limpeza | arquivos do app são descartáveis, persistência de personagem no servidor |
| `MobileTransport` | confiável, ordenado, timeout, reconexão, backpressure, TLS | preservar wire GNET; sessão/auth no servidor |
| `MobileTelemetry` | frame-time CPU/GPU, RAM, thermal state, battery drain, downloads | dados diagnósticos mínimos, sem segredos/PII em logs |

### Modelo sugerido de eventos

```text
COLD -> INITIALIZING -> LOGIN -> IN_WORLD
                |           |
                +-> SUSPENDED <--- BACKGROUND
                |           |
                +-> RESUMING -> LOGIN/IN_WORLD (reauth/reconnect)
                |
              TERMINATED / FAILED
```

O app **não** assume que a sessão de rede sobreviverá a background, perda de rádio, memory pressure ou kill pelo SO. O servidor permanece autoritativo; reconexão precisa de contrato explícito, anti-duplicação e expiração. `surfaceDestroyed` e recriação de dispositivo não podem destruir o estado de sessão antes do protocolo permitir.

## 4. Android: sequência de implementação futura

1. **Toolchain:** Android Studio/SDK, NDK versionado, Clang, CMake com toolchain NDK; alvo inicial `arm64-v8a`; variantes ABI extras somente por demanda medida. Separar bibliotecas C++ comuns do host Kotlin/Java.
2. **Host:** Activity com Surface/SurfaceView ou SDL3 validado; ponte JNI mínima para lifecycle, IME, permissões e store; render thread possui o contexto GPU.
3. **Renderer:** primeiro protótipo OpenGL ES 3.x se houver paridade com os shaders WebGL2; Vulkan somente após benchmark, compatibilidade e ADR. Não assumir compartilhamento binário de shaders GLSL sem transcodificação.
4. **Audio/Input:** áudio low latency (AAudio/Oboe ou SDL3 após prova), toque multi-dedo, pinch/câmera, botões reconfiguráveis e gamepad. UI legada AUI precisa de escala, safe areas e legibilidade; adaptar apresentação, **não lógica de jogo**.
5. **Assets:** manifest/content hashes comuns, armazenamento privado e download incremental; avaliar Play Asset Delivery somente se for compatível com licença, tamanho e distribuição; offline completo não é pré-requisito de um MMO.
6. **Distribuição:** AAB/assinatura, permissões mínimas, crash reports seguros, rollback e atualização com conteúdo/cliente de versões compatíveis.

Critérios de aceite Android: app nativo usa `libclient_core`/equivalente sem WASM/Chromium/WebView; executa login, mundo, animações, controles, áudio, reconexão e duas sessões, em **dispositivos físicos** de classes diferentes.

## 5. iOS: sequência de implementação futura

1. **Toolchain:** macOS com Xcode/clang, SDK iOS, assinatura/provisioning; arquitetura inicial arm64; separar core C++ de host Swift/Objective-C++.
2. **Host:** UIView/UIViewController, lifecycle de cenas, foco/teclado, safe area, rotação e callbacks de suspensão. Decidir se SDL3 fornece host suficiente ou se camada Apple dedicada é necessária.
3. **Renderer:** **Metal** como backend alvo preferencial; adaptar o subconjunto observado da Angelica (render state, buffers, shaders, materiais, skinning). Não depender de OpenGL ES legado/deprecated para viabilidade de longo prazo; considerar tradução/abstração somente com evidência de custo.
4. **Áudio:** AVAudioEngine/AVAudioSession ou SDL3 após benchmark; interrupções, fones/Bluetooth, background; respeitar regras e políticas da plataforma.
5. **Rede/assets:** cache sandbox, quotas, verificação por hash, download/retry e compatibilidade de release; acesso a conta/realms igual ao browser; autenticação e segurança por infraestrutura existente.
6. **Distribuição:** assinatura, TestFlight, App Store review, políticas de conteúdo/licenciamento e assets, requisitos atuais de SDK/privacidade a verificar **na época da execução**.

Critérios de aceite iOS: app C++ nativo renderiza via backend iOS em iPhones/iPads reais; mantém paridade de pacotes/fixtures e fluxo de gameplay; não depende do Safari embutido para executar o game.

## 6. Controles e experiência mobile

- Definir `InputAction` semântica (move, look, attack, target, interact, jump/fly, skill_slot, ui_accept/cancel), mapeada de mouse/teclado para touch/gamepad; preservar keybinds existentes na versão web.
- Interface escalável por densidade e aspecto, safe areas, tamanho tocável, menus/tooltip; UI não pode esconder elementos críticos nem consumir taps de gameplay indevidamente.
- Perfil gráfico adaptativo por classe de GPU e custo térmico real: resolução dinâmica, LOD, anisotropia, sombra, transparência e limite de efeitos **após baseline**, sem quebrar combate/visibilidade.
- Acessibilidade básica: contraste, ajuste de escala, remapeamento de controles, alternativa a gestures obrigatórios; não impedir input por IME ou teclado conectado.

## 7. ABI, dependências e segurança

**Não reutilizar `-fshort-wchar` cegamente.** O cliente antigo supõe UTF-16/WIN32; iOS/Android não possuem ABI de Windows e dependências de SDK podem esperar larguras diferentes. Criar conversões explícitas, testar CP936/UTF-8/UTF-16, structs `#pragma pack` e ponteiros `uintptr_t`. Não passar classes com exceções C++ diretamente pelas bridges JNI/Swift.

Middleware original (DirectSound/FMOD/SpeedTree/COM/ActiveX) exige substituição real ou exclusão de funcionalidade com justificativa e teste; evitar stubs que alegam sucesso. Nunca embutir secrets de servidor, aceitar pacotes sem validação, executar scripts de assets não confiáveis ou expor serviços internos. Reavaliar `docs/LEGAL.md` para publicação em lojas e formatos de distribuição.

## 8. Roadmap futuro por evidência

| Fase | Resultado observável |
|---|---|
| M0 (dependência) | jogo WASM do PR #23 com gameplay e servidor real; conjunto de fixtures congelado |
| M1 | build de bibliotecas comuns em ARM64 Android + iOS; testes de matemática, arquivo, skeleton, GNET |
| M2 | aplicação vazia + GPU draw real + input/áudio, sem código duplicado de gameplay |
| M3 | personagem animado e mundo renderizados, context loss/suspend/resume |
| M4 | login, entrar no mundo, movimento e 2 jogadores sincronizados no realm original |
| M5 | UI completa, combate/skills/inventário/voo/quests e compatibilidade de assets |
| M6 | testes de RAM/VRAM/thermal/bateria, offline/reconnect, distribuição e rollback |

### Matriz mínima de QA

Android intermediário e topo de linha, versões de WebView irrelevantes ao app nativo; iPhone e iPad reais com diferentes gerações de GPU; redes Wi-Fi/4G/5G e troca de rede; cold/warm start, screen rotate, chamada recebida, background, interrupção de áudio e instalação sem cache. Registrar SHA/realm/asset hash/dispositivo/OS/GPU/driver/frame-time/crash. **Não inventar metas fixas de FPS antes da baseline.**

## 9. Gatilho e saída

**Trigger:** humano aprova nova ADR e inicia esta frente somente após G0; criar workorders específicas sem tocar o PR #23. **Definition of Done:** Android e iOS executam **cliente C++ nativo** com paridade funcional relevante, testes de hardware, assinatura, atualização segura e autorização de distribuição. Se o custo inviabilizar paridade ou comprometer o browser, manter somente os wrappers WebAssembly e retornar a proposta ao backlog.

