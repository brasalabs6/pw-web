# Arquitetura — GNET, protocolo de mundo e gateway WSS seguro

## Contexto no monorepo
- `gateway/` — módulo de responsabilidade de implementação/controle
- `engine/network` — módulo de responsabilidade de implementação/controle
- `tests/protocol` — módulo de responsabilidade de implementação/controle
- `docs/` — módulo de responsabilidade de implementação/controle

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
### 1. PacketReader/Writer
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 2. ProtocolVersion
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 3. ClientSession
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 4. WsGateway
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 5. PacketTrace
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
- Medir fixtures de ida/volta byte-a-byte por cenário, browser, commit e build, preservando a evidência
- Medir max payload definido por cenário, browser, commit e build, preservando a evidência
- Medir gateway sem exposição do barramento interno por cenário, browser, commit e build, preservando a evidência
