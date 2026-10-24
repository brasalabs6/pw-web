# Arquitetura — Operações contínuas, versionamento, conteúdo e manutenção

## Contexto no monorepo
- `ops/` — módulo de responsabilidade de implementação/controle
- `infra/` — módulo de responsabilidade de implementação/controle
- `gateway/` — módulo de responsabilidade de implementação/controle
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
### 1. VersionMatrix
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 2. IncidentRecord
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 3. PlayerSupportCase
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 4. MaintenanceWindow
Entrada tipada e validada antes de mutações; saída determinística sempre que possível, com estado explícito para sucesso/erro/cancelamento. Conservar versão, fonte do dado, correlação e ordem de operações. Invariantes devem ser traduzidas em testes de contrato: não emitir buffers fora de limite, não vazar recursos, não confirmar operação sem ack quando aplicável.

### 5. FeatureFlag
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
- Medir rollbacks sem perda indevida por cenário, browser, commit e build, preservando a evidência
- Medir tempo de resposta incidente medido por cenário, browser, commit e build, preservando a evidência
- Medir compatibilidade por release por cenário, browser, commit e build, preservando a evidência
