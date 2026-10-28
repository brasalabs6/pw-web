# Arquitetura multiplataforma — Angelica / Perfect World

> **Status: PROPOSTA FUTURA — não executar agora.** Documento de design, não ADR aprovada, não workorder e não solicitação de modificar o port ativo. Criado em 2026-10-28 para preservar decisões e permitir implementação posterior. **Prioridade absoluta atual: PR #23 — cliente 1.5.3 WASM jogável no navegador; PR #17 — servidor.**

## 0. Escopo e índice

Este conjunto contém **exatamente quatro** especificações complementares:

| Arquivo | Responsabilidade |
|---|---|
| [ARCHITECTURE.md](ARCHITECTURE.md) | Contratos, arquitetura comum, isolamento de plataformas, migração e gates |
| [WEBASSEMBLY.md](WEBASSEMBLY.md) | WebAssembly/browser como origem e distribuição via launcher WebView/Chromium |
| [MOBILE-NATIVE.md](MOBILE-NATIVE.md) | **Futuro** cliente compilado para Android/iOS, não apenas WebView |
| [WINDOWS-LINUX-NATIVE.md](WINDOWS-LINUX-NATIVE.md) | **Futuro** cliente C++ compilado para Windows/Linux |

**Não confundir três produtos:**
1. **Browser WASM (em execução de port):** mesmo cliente C++ compilado via Emscripten, render WebGL2 no navegador.
2. **Launcher/APK WEB (entrega inicial):** contêiner Chromium/WebView que abre o **mesmo** cliente WASM; não é um cliente nativo de jogo.
3. **Cliente NATIVO (somente futuro):** código C++ recompilado para ABI de desktop/mobile e backend gráfico do SO, sem runtime WASM.

A especificação atual do port permanece em [ANGELICA-PORTING-METHOD.md](../ANGELICA-PORTING-METHOD.md), [CLIENT153-PORT.md](../CLIENT153-PORT.md), [ARCHITECTURE.md](../ARCHITECTURE.md), [ROADMAP.md](../ROADMAP.md), [ADR-0004](../DECISIONS/ADR-0004-incremental-port-method.md) e [ADR-0005](../DECISIONS/ADR-0005-cloudflare-delivery.md). **Em caso de conflito com o port em andamento, esses contratos atuais e o PR #23 prevalecem.**

## 1. Objetivo arquitetural (apenas após G0)

Evoluir a base **Angelica2 + ElementClient 1.5.3** já portada em C++ para multiplataforma, com **um único núcleo de regras/gameplay** e hosts independentes. Preservar comportamento comprovado, formatos PCK/PKX/MOX/DDS/SKI/BON, layout de protocolo GNET e referência por versão/realm. Não reescrever o jogo, não inventar outro protocolo e não clonar a engine por plataforma.

### Diagrama lógico alvo (não implementado)

```text
                    Perfect World: Client Core C++17/20
          regras • ECS legado • mundo • animação • UI lógico
          rede GNET • loaders • colisão • scripts • simulação
                              |
                 Interfaces de plataforma (seams)
       +----------------------+-----------------------+
       |                      |                       |
  IRuntimeHost           IRenderDevice           IAssetStore
  IInputHost             IAudioDevice            ITransport
  IClock                 IStorage                IConcurrency
       |                      |                       |
       +----------+-----------+-----------------------+
                  |
    +-------------+----------------------+-----------------+
    |                                    |                 |
 BrowserHost                         DesktopHost       MobileHost
 Emscripten                          SDL3/Win/Linux    NDK/iOS shell
 WebGL2 GLES3                        OpenGL inicial    GLES3/Metal*
 WebAudio                            áudio nativo     áudio nativo
 Fetch+OPFS/IDB                      arquivos locais  sandbox/asset packs
 WSS/GNET                            TCP/GNET**        TCP/GNET**
    |                                    |                 |
 Browser/PWA + launcher web         Win/Linux .exe    APK/AAB / iOS app
 + APK WebView (mesmo WASM)          ou binário ELF    (cliente C++ nativo)
```

* Seleção de backend gráfico mobile depende de prova de plataforma; Metal é a direção preferida para iOS, não se presume que GLES3 funcione de forma ideal.
** TCP/transport nativo é hipótese técnica, sempre atrás do protocolo/contrato autenticado e do gateway/servidor; não expor serviços internos nem mudar serialização GNET.

## 2. Princípios de decisão

1. **Browser-first.** Nenhuma mudança estrutural para suportar nativo antes do gate G0. Não alterar a branch/planejamento do PR #23.
2. **Um código de jogo, múltiplas implementações de host.** Reutilização significa compartilhar *fontes e comportamento*, não forçar identidade de ABI ou binário entre WASM e plataformas nativas.
3. **Separar comportamento de dispositivo.** CECGame/Angelica não deve conhecer DOM, HWND, Activity, UIView ou chamadas D3D8 fora de seus adaptadores.
4. **Substituição gradual com rastreabilidade.** Preservar APIs originais quando possível; alterar semântica somente com fixtures e testes de regressão contra a baseline conhecida.
5. **Backends reais, sem false-success.** Recurso não suportado falha explicitamente, nunca retorna sucesso sem desenhar, tocar áudio, ler arquivo ou enviar pacote.
6. **Um realm, versões compatíveis.** A tupla `client source revision + assets hash + protocol/wire revision + server realm` é validada antes do login.
7. **Sem duplicação de conteúdo ou regras.** CDN e cache versionados fornecem assets; desktop/mobile podem persistir localmente; validação de integridade é a mesma.
8. **Medição antes de otimizar.** Repositório `.agents/optimization/` é backlog pós-port, não condiciona G0.
9. **Direitos e distribuição:** observar [LEGAL.md](../LEGAL.md); autorização documentada se aplica ao escopo contratado, sem generalizar automaticamente para novas lojas, territórios ou produtos.

## 3. Fronteiras e contratos propostos (não são APIs implementadas)

Definir contratos em arquivos separados **somente na ativação da feature**. Os nomes abaixo são identificadores sugeridos, para orientar a implementação e evitar dois motores divergentes.

| Domínio | Interface proposta | Semântica mínima / erros |
|---|---|---|
| Ciclo de vida | `IRuntimeHost` | `init/tick/render/suspend/resume/dispose` sem loop bloqueante; recursos possuem dono |
| Relógio/concorrência | `IClock`, `ITaskScheduler` | relógio monotônico, política de espera, callback, cancelamento e shutdown |
| Renderização | `IRenderDevice` | criar/destruir buffers/texturas/shaders, estados cull/depth/blend/fog, draw, restore/lost device |
| Entrada | `IInputHost` | keyboard/text IME/pointer/touch/controller, foco e fila de eventos por frame |
| Áudio | `IAudioDevice` | streaming, mixagem, pause/resume, device-lost, volume; gestos web respeitados |
| Arquivos/assets | `IAssetStore`, `IAssetProvider` | VFS, async request, range, hash, cancel, quota, cache por conteúdo; não baixar todo PCK |
| Rede | `ITransport` | sequência de bytes confiável, framing do GNET preservado, backpressure, reconexão e auth |
| Telemetria | `IPerfProbe` | frame CPU/GPU, RAM/VRAM, IO, bytes download/cache, métricas com build/release/driver |

Exemplo **sem compromisso de ABI**, intencionalmente conceitual:

```cpp
struct FrameArgs {
  double monotonic_seconds;
  double delta_seconds;
  uint32_t viewport_width;
  uint32_t viewport_height;
};
struct PlatformServices {
  IClock& clock;
  IAssetStore& assets;
  IInputHost& input;
  IAudioDevice& audio;
  ITransport& network;
  IRenderDevice& renderer;
};
// Na implementação real, fechar ownership, thread affinity,
// erros/cancelamento, versão do contrato e pontos de sincronização.
```

**Propriedade / fluxo de chamadas:** o host cria os services e o cliente; o cliente solicita recursos às interfaces; callbacks assíncronos são enfileirados e consumidos de modo determinístico no tick; o dispositivo é destruído após parar callbacks e jobs. Não despachar DOM/SDL/UIKit para lógica de jogo diretamente. Não tentar usar C++ exceptions ou wchar_t como ABI público C de plugin; fronteiras binárias usam tipos de largura fixa.

## 4. Compatibilidade legada essencial

| Área | Risco existente | Regra de migração futura |
|---|---|---|
| `wchar_t`/`-fshort-wchar` | original Win32 usa UTF-16 de 16 bits; Linux e runtime libc frequentemente usam outra largura | manter conversão explícita para formatos/wire; testar chamadas `wcs*` e bibliotecas externas; não tratar `-fshort-wchar` global como solução universal |
| Estruturas 32-bit | `DWORD`, ponteiros convertidos para int, `#pragma pack` | limites por tipo, `uint32_t/uint64_t/uintptr_t`, comparações de layout e golden bytes |
| D3D8 fixed-function | FVF, render states, materials, shaders assembly | mapear **só** estados observados; teste de frames; nada de emular DirectX inteiro |
| Recursos de middleware | FMOD, SpeedTree, ActiveX/COM | validar direitos/SDK/real necessidade; substituir por implementação comprovada, não stubs de sucesso |
| Fontes/locale | CP936/GBK, UTF-16, UTF-8 | normalização na borda com testes de nomes e UI |
| Threading/event loop | Windows waits/render threads | semântica equivalente por plataforma; tarefas cooperativas e affinity explícitos |
| Protocolo GNET | versão 1.5.3 versus PWBR v145 e servidor derivado | golden packets e rejeição de incompatibilidade, sem alterar regras para “fazer funcionar” |

## 5. Compartilhamento de módulos e build

**Estrutura lógica futura sugerida** (não mover arquivos agora):

```text
src/client153/               # Angelica / ElementClient adaptados (fonte canônica)
engine/                       # contratos e serviços já existentes/evoluídos
platform/web/                 # Emscripten, DOM, WebGL2
platform/desktop/             # SDL3, Windows/Linux, OpenGL inicial
platform/mobile/android/      # NDK, Activity, lifecycle, GLES/Vulkan opcional
platform/mobile/ios/          # App lifecycle, input, Metal adapter
apps/web/                     # shell/PWA/browser
apps/launcher-web/            # shell Chrome/WebView -> MESMO WASM (fase inicial)
apps/desktop-native/          # binário futuro
apps/mobile-native/           # binários futuros
tests/portability/            # fixtures comuns; smoke por host
```

Esse mapa representa módulos responsáveis, não uma ordem de reorganização obrigatória. Adaptar ao monorepo real sem duplicar os atuais `src/client153/platform/common`, `src/client153/platform/web` e `web/client153`. Preferir interfaces finas à criação precoce de novos pacotes. CMake separa bibliotecas reutilizáveis dos targets de plataforma; nenhum `#ifdef` deve esconder sucesso falso. Presets e toolchain files serão adicionados só quando o host correspondente for trabalhado.

## 6. Versões, releases, conteúdo e operação

Manifest versionado conceitual: `release_id`, `client_revision`, `protocol_revision`, `realm_compatibility`, `asset_manifest_sha256`, `platform_target`, `abi`, `renderer_features`, `content_base_url`, `minimum_host_version`. Validar o schema com os contratos existentes de e15/e17 antes de implementá-lo; **não criar um segundo formato de release concorrente**.

- Cliente WEB: distribuir um único WASM e conteúdo por hashes, através da arquitetura Cloudflare já especificada; PWA/launcher web/APK web consomem a mesma release.
- Cliente NATIVO: pode compartilhar *identidades* de assets/manifest, mas possuir executáveis/patches distintos e cache local; assinatura e distribuição por plataforma.
- O servidor não deve confiar em “platform=desktop/mobile/web” para autorizar gameplay. Controle de sessão, rate limiting e anti-abuso no backend.

## 7. Roadmap **dormant** com gates de ativação

| Gate | Dependência e evidência para avançar | Resultado |
|---|---|---|
| **G0 — ATIVAÇÃO (obrigatório)** | PR #23 oferece cliente WASM realmente jogável: login, personagem, mundo, movimentação, duas sessões no navegador, testes desktop e mobile; dono aprova iniciar multiplataforma | libera pesquisa/ADR nova, não merge automático |
| G1 — Baseline | fixar SHA, realm, assets, screenshots, pacotes, perf e conjunto de regressão do browser jogável | baseline comparável |
| G2 — Contratos | mapear chamadas dependentes de host, desenhar adapters mínimos e ownership; aprovar ADR de renderer/ABI | interfaces revisadas |
| G3 — Dual-build | compilar core/jogo no WASM e num target nativo sem mudar comportamento; executar fixtures comuns | paridade dos sistemas compartilhados |
| G4 — Render/IO | mesmo cenário + personagem e UI com paridade aceitável; áudio e redes reais, comparação de frame/sessão | primeiro cliente nativo vertical |
| G5 — Produtos | desktop e mobile independentes passam cold/warm start, suspend/resume, atualização e distribuição legal | candidatos a release |
| G6 — Release | testes em hardware real, assinatura, permissões, direitos, métricas e rollback | publicação humana |

Nenhuma data ou porcentagem de conclusão é assumida. Se G0 falhar, **toda esta feature segue congelada**.

## 8. Qualidade e aceite

- Cada novo host tem suíte de smoke de login → personagem → mundo → movimento com servidor real; **não** é validado só por parser, splash ou tela do launcher.
- Fixtures comuns para matemática, skeleton, animação, serialização GNET, arquivos PCK/PKX e limites de memória. Confirmar diferenças intencionais e bugs legados corrigidos.
- Testar falhas reais: perda de GPU, falta de rede, troca de versão, falta de espaço, suspend/resume, asset corrompido, transporte congestionado, interrupção de download, locale.
- Medir percentis de frame-time em hardware fixado, RAM/VRAM, stutter, cold/warm load, bateria/temperatura em celular, bytes entregues; **não prometer FPS sem benchmark**.
- Definição de pronto: a feature entrega cliente **de jogo** nativo multiplataforma sem duplicar o core nem quebrar a versão web, mais pacotes assinados e plano de rollback.

## 9. Não objetivos e governança

- Nada deste documento muda `AGENTS.md`, ADR-0004, PR #23 ou PR #17.
- Nada aqui autoriza iniciar subagentes/workorders, criar runtime alternativo nem reescrever módulos em execução.
- Não substituir o launcher web inicial por cliente nativo antes de G0.
- PR desta pasta é **docs-only / draft / futuro**; execução depende de decisão humana posterior, ADR(s) específicas e novo escopo aprovado.

