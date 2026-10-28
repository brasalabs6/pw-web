# Runtime, WASM, memória e concorrência

## Direção
Preservar C++ compatível em WASM e isolar interfaces do navegador. Evitar reescrever módulos por linguagem apenas por expectativa de performance.

## Medidas
- Profiling de main thread, event loop, GC JS, WASM heap, alocações e cópias.
- Pools e arenas para objetos transitórios quando profiler confirmar custo de alloc.
- Typed arrays e buffers reutilizáveis entre JS/WASM; evitar marshalling repetido.
- Fixed-step simulation com limite de catch-up e interpolação de render.
- Workers para decodificação, parsing, jobs de animação e tarefas independentes.
- OffscreenCanvas somente com fallback validado e testes cross-browser.
- SharedArrayBuffer/threads apenas sob requisitos de isolamento de origem e suporte real.
- Budgets explícitos para heap, assets residentes, filas e uploads por frame.
- Tratamento de context loss, page visibility, suspensão de mobile e retorno de background.

## Aceite
Sem vazamento progressivo em sessões longas; medir p95 de pausa, tempo de retomada, uso máximo de memória e determinismo dos testes de simulação. Evitar micro-otimização sem flamegraph.
