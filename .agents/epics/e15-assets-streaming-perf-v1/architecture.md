# Arquitetura — Streaming, CDN, cache OPFS e desempenho do navegador

## Contexto no monorepo
- `web/storage` — módulo de responsabilidade de implementação/controle
- `web/streaming` — módulo de responsabilidade de implementação/controle
- `tools/assets` — módulo de responsabilidade de implementação/controle
- `engine/resources` — módulo de responsabilidade de implementação/controle

## Fluxo de dados e controles
```text
workorder versionado → contrato/interface → implementação isolada
       ↓                     ↓                  ↓
   gates direitos      testes unitários    testes integração
       └─────────────→ relatório CI ←────────────┘
                           ↓
                      PR para revisão
```

## Interfaces propostas e invariantes
### 1. ManifestVersion
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 2. ChunkRequest
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 3. AssetPriorityQueue
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 4. CacheEntry
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 5. PerfSample
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

## Fronteiras
- O browser só tem WebAssembly/JS/WebGL2/Web Audio/WebSocket/Storage, nunca `Windows.h` ou DirectX nativo.
- Serviços de jogo continuam autoritativos no backend; decisões do cliente não mudam economia/sessões sem confirmação.
- Recursos externos entram por parsers com limites; manter trilha de versão, hash e autorização.
- A plataforma e os contratos são de código nosso; fontes upstream ficam em `references/` e não entram em CI.

## Versionamento e fallback
- Compatibilidade entre cliente, engine, asset e realm deve constar em manifesto/matriz, não por aproximação.
- Toda mudança pública de contrato exige teste backwards + ADR.
- Se inviável ou sem direitos suficientes, manter contrato e substituir implementação por módulo original autorizado.

## Observabilidade
- Medir bytes até primeiro frame por cenário, browser, commit e build, preservando a evidência
- Medir p95 frame time durante streaming por cenário, browser, commit e build, preservando a evidência
- Medir taxa hit cache, espaço consumido por cenário, browser, commit e build, preservando a evidência
