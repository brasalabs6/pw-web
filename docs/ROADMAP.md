# Roadmap de Perfect World Web — gates por evidência

**O marco é uma capacidade observada, nunca uma estimativa temporal.**

| Marco | Meta | Teste de aceitação | Estado |
| --- | --- | --- | --- |
| M0 — Bootstrap | Estrutura, scripts, CI, web host e WASM mínimo | `verify.sh` e build com emcc em ambiente provisionado | em andamento |
| M1 — Triagem do código | Selecionar versão/client/engine utilizáveis | Build Windows reproduzível + inventário de APIs + parecer IP | não iniciado |
| M2 — Núcleo portátil | Compilar subsistema útil para wasm32 | testes nativo e WASM concordam em fixtures permitidas | não iniciado |
| M3 — Render engine | Desenhar mundo e personagens reais | cena com recursos autorizados e comparação semântica | não iniciado |
| M4 — Transporte | Login/protocolo binário compatível com realm | integração e fixtures byte a byte | não iniciado |
| M5 — Fatia MMO | Login → personagem → mundo → movimento multiuser | 2 clientes no navegador, estados convergentes | não iniciado |
| M6 — Funcionalidades | Voo, combate, skills, NPC, inventário, missões | suíte de regressão contra build autorizado | não iniciado |
| M7 — Entrega | Cache/streaming, segurança, testes cross-browser | metas de desempenho medidas, rights cleared | não iniciado |

## Ordem P0
1. Verificar procedência jurídica de fontes e assets; não redistribuir artefatos até autorização.
2. Construir benchmark de compilação Win32 e interface gráfica de candidatos 1.5.3 e 1.5.2.
3. Extrair e categorizar dependências Win32, D3D8/9, SDK, ponteiros, asm, threads, networking.
4. Fazer C++ básico rodar no navegador e depois um módulo real independentemente do render.
5. Escolher versão-alvo em ADR com pontuação técnica e jurídica.
6. Instrumentar provas com logs, arquivos compilados, ferramentas e versões exatas.

## Riscos / mitigação
| Risco | Gravidade | Mitigação |
| --- | --- | --- |
| Direitos de código/arte/áudio não demonstrados | bloqueante para distribuição | parecer/autorização ou implementação/conteúdo autorizado |
| 1.5.3 não compila/loga | crítico | comparar 1.5.2, validar Windows primeiro e protocolo por versão |
| Grande acoplamento D3D/Win32 | crítico | adapter incremental e testes de equivalência |
| Incompatibilidade asset↔engine | alto | fixtures/manifest com identificação exata de versão |
| Protocolo difere do realm Rust | alto | IR do protocolo, golden tests e adaptação controlada |
| Assets pesados e nomes GBK | alto | pipeline PCK, streaming/cache e limite/UTF handling |
| Browser memory/perf | alto | perf budgets e loading sob demanda |
| Security do gateway | alto | WSS, auth, rate limiting e isolamento de rede |

## Critérios de cancelamento/redirecionamento
- Sem permissão suficiente: parar integração de conteúdo proprietário e optar por código/arte autorizados.
- Build de referência inviável após investigação documentada: migrar para cliente novo com engine própria, mantendo compatibilidade de protocolo onde permitido.
- D3D translation inviável: construir renderer próprio do subconjunto necessário; documentar custo.

## Próximas issues sugeridas
- P0-1 Compilar e executar cliente 1.5.3 em laboratório autorizado.
- P0-2 Inventariar APIs do 1.5.3 comparando a 1.5.2.
- P0-3 Verificar e documentar direitos/arquivos-base.
- P0-4 Primeiro parser original/permitido C++ compilado em Emscripten.
- P0-5 Matriz protocolo-cliente-servidor e teste handshake.
- P0-6 Definir adapter gráfico mínimo WebGL 2 com device state trace.
