# WebAssembly — baseline principal e distribuição WEB

> **Status: ESPECIFICAÇÃO DE REFERÊNCIA/FUTURO.** O port real do cliente está **em andamento no PR #23** e é governado pelos documentos atuais. Este arquivo **não cria nova implementação do WASM** nem altera as prioridades do agente local. Ver [arquitetura](ARCHITECTURE.md).

## 1. O que é o produto

O cliente original Angelica/ElementClient 1.5.3 adaptado é compilado com Emscripten para `wasm32`; a CPU executa WASM localmente no navegador e a GPU renderiza via WebGL2/OpenGL ES 3. **Não há streaming de vídeo do jogo.** O host web usa JavaScript/DOM para Canvas, input, bootstrap e serviços assíncronos.

A mesma URL/release e o **mesmo binário WASM** serão usados nas primeiras embalagens:
- **Browser desktop:** Chrome/Edge/Firefox e outros compatíveis.
- **Browser mobile:** Chrome Android e Safari iOS (testes reais obrigatórios).
- **Launcher WEB Windows/Linux:** app fino com Chromium/WebView/PWA abrindo o client web, não um `pw-client.exe` nativo; cache e atualização verificáveis.
- **APK WEB Android:** app WebView mínimo que abre o mesmo cliente; testar capacidade real de WebGL2/WASM, touchscreen, áudio e retomada. iOS inicialmente via Safari/PWA, não prometer APK em iOS.

Não confundir launcher com o cliente C++ nativo definido em [WINDOWS-LINUX-NATIVE.md](WINDOWS-LINUX-NATIVE.md) nem APK WebView com [MOBILE-NATIVE.md](MOBILE-NATIVE.md).

## 2. Fontes de verdade

- [ANGELICA-PORTING-METHOD.md](../ANGELICA-PORTING-METHOD.md) — ADR-0004 e método de port incremental, adaptando somente fronteiras comprovadas.
- [CLIENT153-PORT.md](../CLIENT153-PORT.md), [CLIENT153-HANDOFF.md](../CLIENT153-HANDOFF.md) — código, testes, limitações, handoff do agente.
- [VALIDATION.md](../VALIDATION.md) — gate G4 render / G5 rede / G6 multiplayer.
- [CLOUDFLARE-DELIVERY.md](../CLOUDFLARE-DELIVERY.md) — build, manifest, conteúdo, R2/cache e release.
- PRs #23 (cliente) e #17 (servidor); **nunca** assumir que builds verdes de objetos = jogo real.

No estado verificado durante a criação destas specs, `src/client153/CMakeLists.txt` produz um visualizador WASM a partir de `platform/web/runtime.cpp`; o bootstrap `web/client153/app.mjs` carrega MOX/DDS e desenha câmera orbital, mas **não prova ElementClient completo no browser**. O estado muda rapidamente: revalidar o HEAD do #23 antes de iniciar qualquer tarefa. Valores históricos de TUs e símbolos não devem virar percentual de jogo concluído.

## 3. Pipeline de execução real (alvo do port em curso)

```text
HTTPS shell -> release manifest verificado -> fetch runtime (WASM+JS)
                    |                             |
                    v                             v
          cache por hash/quota             Emscripten bootstrap
                    |                             |
            chunks de assets               CECGame init -> tick/render
                    |                             |
             AFile/VFS loaders <-----------+-----> WebGL2 / WebAudio
                                                   |
                                  input teclado/touch/IME, resize/foco
                                                   |
                           adaptador GNET -> WSS -> gateway -> realm PR #17
```

**Contratos obrigatórios:**
1. **Boot:** estados `NEW → FETCHING → INITIALIZING → LOGIN → IN_WORLD → SUSPENDED → STOPPED/FAILED`; erro com causa rastreável, nunca sucesso falso.
2. **Ciclo:** usar scheduler compatível com `requestAnimationFrame`, permitir callback e carga assíncrona; nenhum `while(true)`/`Sleep` bloqueando a thread principal.
3. **Render:** backend Angelica real (A3DEngine/A3DDevice/A3DStream, materiais, blending, depth, cull, textura, animação) produz pixels; viewer MOX/DDS segue como teste separado.
4. **Arquivos:** loaders originais adaptados lêem bytes verificados em VFS; cache / streaming por chunks versionados com budgets de heap/VRAM; falha recuperável de rede/armazenamento.
5. **Rede:** browser **não** tem TCP arbitrário; usar WSS binário, gateway autenticado com backpressure, preservando framing e golden bytes do protocolo GNET.
6. **Input:** teclado, mouse, texto Unicode/IME, scroll, pointer capture e touch; não deixar tecla presa após perda de foco.
7. **Áudio:** WebAudio com desbloqueio após gesto de usuário, suspensão/retomada e controle de volume; nunca depender de DLL Win32.
8. **Segurança:** limites de parsing, hashes, CORS/HTTPS, COOP/COEP apenas se threading exigir e houver compatibilidade; nenhum token ou URL privada embutidos no código.

## 4. Emscripten e compatibilidade — configurações a avaliar (não aplicar sem teste)

| Capacidade | Primeira implementação | Decisão futura / condição |
|---|---|---|
| Compilador | Emscripten, CMake, Ninja; `wasm32` | fixar versões no CI e release manifest |
| Render | WebGL2/GLES3 | WebGPU somente por ADR e benchmark, sem regressão de suporte mobile |
| Loop | main thread + RAF, operações cooperativas | OffscreenCanvas/Worker só se medido e suportado |
| Threads | mínimo necessário ao port | `-pthread`/SharedArrayBuffer exigem isolamento cross-origin, HTTPS e teste por navegador |
| SIMD | baseline portátil | `-msimd128` após testes de fallback e perfil de devices |
| Memory | linear heap com cap/telemetria | crescimento de memória não dispensa budget e testes em Safari |
| Exceções | preservar comportamento comprovado no port | escolher flags de exceções compatíveis em todos os targets |
| Dados | manifest + chunks + cache | OPFS/IndexedDB somente com política de quota/recovery |
| Fonte/locale | GBK/CP936 ↔ UTF-16 ↔ UTF-8 | golden fixtures de nomes/strings, sem conversões silenciosas |

As flags já presentes no repositório, incluindo `-fshort-wchar` e `ALLOW_MEMORY_GROWTH`, são **baseline a auditar**, não uma receita para outros alvos. Importante revisar ABI, layout e biblioteca `wchar_t` antes de expandir.

## 5. Distribuição: browser, launcher web e APK web

### 5.1 Browser/PWA

- Hospedar shell/release imutável em HTTPS; resolver runtime e conteúdo pelo mesmo `release_id`/hash. Streaming de login → personagem → célula inicial → mundo sob demanda, sem preload dos ~GBs completos.
- Validar `WebGL2`, memória disponível, espaço de cache, áudio, rede; mostrar diagnósticos acionáveis e fallback de qualidade, **não** declarar “todos devices”.
- Preservar progresso e credenciais via backend; caches são removíveis e podem ser evictados pelo navegador.
- Evitar cache de versão mista e rollback não atômico; aproveitar o contrato já especificado em e15/e17, sem inventar segundo updater.

### 5.2 Launcher web Windows/Linux (entrega inicial, não cliente nativo)

- Host Chromium/Electron ou PWA: abre a mesma origem HTTPS do browser; decidir entre URL remota e recursos locais assinados após testes. Não gerar uma build separada de gameplay.
- Separar dados de sessão e cache por perfil, validar TLS, bloquear navegação para origens não permitidas, não habilitar APIs Node arbitrárias no conteúdo remoto.
- Features mínimas: tela de status/versão, atualizar/recarregar release, logs de diagnóstico, botão jogar, modo fullscreen, recuperação de crash.
- **Gate:** provar que versão/bytes do WASM são idênticos à release web e que login/mundo funciona sem código de gameplay exclusivo do launcher.

### 5.3 APK web Android (entrega inicial, não cliente ARM nativo)

- `WebView` em Activity/Kotlin fina (ou host equivalente validado), URL HTTPS idêntica à do navegador.
- Permissões mínimas, configurar WebGL, foco/teclado, áudio, orientação, back gesture, lifecycle e fullscreen; proteger bridge JavaScript e URLs.
- Testar aparelho Android real: suporte `WebGL2`, input touch, teclado, aquecimento, background/foreground, contexto perdido, limpar cache e reabrir, troca de rede; WebView não garante equivalência ao Chrome.
- Versionamento do app separado do `release_id` do WASM; app pode ter atualização de host sem nova versão do jogo.

## 6. Gates de qualidade para o cliente WebAssembly

| Gate | Demonstração exigida | Não aceita como prova |
|---|---|---|
| W0 | WASM íntegro e Canvas WebGL2 abertos com Chrome smoke | somente `em++` compilar |
| W1 | cliente `ElementClient` original linkado e `CECGame::Init` inicializando | viewer MOX/DDS isolado |
| W2 | tela de login AUI original, eventos reais e render de animação | mock HTML ou shader genérico |
| W3 | WSS ↔ GNET ↔ servidor real; login, personagens, EnterWorld | pacote fabricado/local stub |
| W4 | mundo, movimento, NPC, ações básicas e duas sessões convergentes | screenshot estática |
| W5 | desktop + Android Chrome + iOS Safari; storage, memória, áudio e recovery | emulador ou SwiftShader exclusivamente |
| W6 | APK web e launcher web abrem **o mesmo WASM/release** e passam W3/W4 | abrir apenas splash |

Falha de qualquer gate do port fica em `.agents/issues/` do PR #23; propostas opcionais de FPS/arquitetura ficam em `.agents/optimization/` para depois. **Este documento não deve introduzir tarefas novas nem reordenar milestones do agente atual.**

## 7. Diagnóstico, observabilidade e reprovação

Para toda evidência coletar: commit, build/release, origem de assets, versão do protocolo/realm, navegador/versão, GPU/driver quando possível, estado de WebGL, log `init/tick/draw`, bytes/erros de rede, percentis de frame-time, memória e cache. Reproduzir cenário frio e quente; testar erro de hash, WebGL context loss, reconexão, falta de quota, renderer extension ausente e suspend/resume.

**Definição de pronto do port atual:** jogo funcional no browser com autenticação real, personagem animado, mundo jogável, controles e duas sessões verificadas. **Definição futura do produto web distribuído:** mesma experiência aprovada acessível pelo navegador, launcher web e APK web, sem duplicação do núcleo.

