# Arquitetura técnica — proposta, não implementação concluída

O [método de port incremental](ANGELICA-PORTING-METHOD.md) define as fronteiras,
o contrato R0 e a próxima tarefa P1 para executores. A [POC-001](POC-ANGELICA.md)
validou apenas um componente original isolado; o diagrama abaixo continua sendo
a arquitetura-alvo, não uma integração de cliente já concluída.

## Fluxo-alvo
```text
Browser
  web shell / loading / status
      │
      ├─ Emscripten WASM ← engine/core + subsistemas de jogo permitidos
      ├─ WebGL2 adapter → GPU (WebGPU futuro por ADR)
      ├─ Web Audio adapter → áudio
      ├─ OPFS/IndexedDB/cache → arquivos indexados e streaming
      └─ WSS binary transport
                ↓
         Gateway de borda (Rust ou outro, validar)
                ↓
           pw-link → pw-gs → persistência
```
O servidor opera remotamente, mas renderização, input e simulação cliente ocorrem **localmente** no browser. Não há streaming de vídeo.

## Módulos e fronteiras
- `engine/core`: tipos/algoritmos C++ sem API Win32, teste nativo e emscripten.
- `engine/platform` (futuro): abstração de relógio, threads, arquivo, memória, entrada, socket e áudio.
- `engine/render` (futuro): comandos abstratos de draw, material, texture, blend, cull, depth; backend WebGL2.
- `engine/formats` (futuro): parsers determinísticos, limites, verificação de entrada, testes golden autorizados.
- `web/`: bootstrap, canvas, adaptador de módulo wasm e diagnósticos browser.
- `gateway/` (futuro): **WSS** com autenticação server-side, backpressure, limites e tradução de transporte; NÃO expor serviços internos.
- `references/`: checkout opcional dos repositórios externos por commit. Não integra CI ou build padrão.

## Decisões iniciais
1. WebGL 2 no primeiro vertical slice; WebGPU só com benchmark e ADR.
2. WASM para C++ portátil; JavaScript para host e browser APIs.
3. WebSocket em vez de tentar TCP raw do browser; preservar framing GNET e gamedata com suíte byte-a-byte.
4. Streaming incremental com manifesto/hashes e armazenamento local; não pré-carregar pacotes grandes.
5. Uma matriz versão de cliente ↔ assets ↔ layout de protocolo ↔ server realm; diferença é bloqueio.
6. Port incremental da Angelica, não big-bang recompilando todos os arquivos do Windows em um passo.

## Problemas concretos a medir
- D3D8 e D3DX8, dispositivo e fixed function; inventário dos shaders/estados antes de decidir tradução.
- Windows API: `Windows.h`, `HWND`, registry, message pump, threads, sockets e paths ANSI/GBK.
- ABI/arquitetura: estruturas `#pragma pack`, bits e endian; não assumir compatibilidade entre versões.
- Arquivos PCK/PKX (alguns pacotes ultrapassam 2GB), texturas/malhas/ossos, dependências de recursos.
- Main loop sem bloqueios de UI, Worker/thread restrictions, COOP/COEP se pthreads entrar.
- Audio/inputs UI, performance de upload de buffers, contexto WebGL perdido e cache.
- Segurança: nunca confiar no cliente; gateway autentica e rate-limita.

## Testabilidade
Unit native C++ sem asset; build WASM e import pelo navegador; contrato de protocolo e fixtures permitidas; screenshot diffs com versão Windows autorizada (`docs/LEGAL.md`); perf medido (FPS/frametime, memória, bytes de rede, tamanho de transferência).

## Futuro de engenharia
`engine`, `web`, `gateway` serão monorepo separados por contratos, com CI em cada módulo e manifests de assets assinados. Hoje, apenas a prova genérica da plataforma existe.
