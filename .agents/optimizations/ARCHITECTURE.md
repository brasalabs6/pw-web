# Arquitetura de otimização

## Objetivo
Preservar o comportamento Angelica 1.5.3 e trocar implementações custosas por subsistemas modernos sem introduzir divergência de gameplay.

## Fronteiras propostas
```text
Angelica original / regras / formatos / protocolo
                 |
        Compatibility adapters (C++/WASM)
                 |
      +----------+----------+-------------+
      |          |          |             |
 Render API   Asset API  Simulation   Transport API
      |          |          |             |
 WebGL2 /    Fetch/VFS/  Workers /    WS / futuro
 WebGPU       Cache/OPFS  WASM         WebTransport
                 |
          Device capability policy
```

## Contratos
- Renderer: create/destroy resources; submit scene; set quality profile; handle context loss; expose GPU/CPU timing and draw calls.
- Assets: manifest version/hash, request priority, cancellation, residency and eviction, integrity, offline fallback.
- Simulation: fixed timestep, deterministic ordering where required, snapshot for rendering, bounded message queue.
- Transport: framing, ordering, reliability, reconnect, authentication boundary, backpressure, telemetry.
- Capability policy: feature detection, explicit fallback, device thermal/memory budget.

## Migração incremental
Primeiro estabelecer equivalência visual e funcional no WebGL2 existente; depois implementar otimizações por feature flag, comparar cenas de referência e reverter automaticamente se regressões excederem limites. WebGPU é backend opcional e deve provar ganhos em hardware real. Evitar duas implementações de lógica de jogo.

## Não objetivos imediatos
Reescrita integral em Rust, ECS universal, troca integral da engine ou obrigatoriedade de WebGPU.
