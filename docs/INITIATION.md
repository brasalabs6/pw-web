# Plano completo de iniciação (project charter + execução)

## North star
Abrir um URL e jogar uma sessão real de Perfect World, com render 3D local por GPU, C++/WASM onde cabível, streaming de recursos e protocolo de mundo conectado a servidor dedicado. **Sem streaming de vídeo ou Windows remoto.**

## Mandato e escopo
- IN: auditoria técnica, proveniência/licenciamento, port da engine e partes autorizadas do cliente, assets pipeline, host browser, gateway, protocolo, observabilidade e testes.
- OUT da fase inicial: monetização, distribuição pública de assets/código de terceiros, gameplay anunciado como pronto, escalabilidade antes de uma sessão funcional.

## Responsabilidades sugeridas (não são pessoas atribuídas)
- Tech lead: arquitetura, triagem de riscos e decisões ADR.
- Engine engineer: C++/Win32/Direct3D, compatibilidade WASM.
- Graphics engineer: WebGL2/formatos/shaders/animação.
- Backend engineer: protocolo, gateway, integração com servidor.
- QA/research: comparação de builds, critérios de aceite, evidências.
- Responsável jurídico/IP: autorização de código, assets, marcas e redistribuição.

## Sequência de onboarding dos agentes
1. Ler AGENTS, documentação e limites legais.
2. Checar estado do repo e últimas ADRs (não presumir decisões anteriores).
3. Reproduzir `verify.sh`, `dev.sh`, `build-wasm.sh` quando disponível.
4. Formular hipótese específica e teste de falsificação.
5. Registrar fontes e commit upstream em `docs/RESEARCH.md`.
6. Executar menor experimento útil, medir e comunicar resultados.
7. Escrever PR com prova e atualizar plano de riscos.
8. Não escolher próxima etapa antes de avaliar gate atual.

## Workstreams e entregáveis
| Trilha | Entregável verificável | Dependência |
| --- | --- | --- |
| Licenciamento | matriz de autoria e permissões por fonte/asset | autorização registrada em `docs/LEGAL.md` |
| Cliente Windows | executável reproduzível + logs + funcionalidade testada | toolchain, recursos autorizados |
| Inventário de dependências | mapa de Win32, D3D8/9, arquivos, som, threads, sockets, asm | código de cliente selecionado |
| Core WASM | build reprodutível com teste C++ → browser | emsdk |
| Gráficos | cena com asset autorizado, shaders/LOD e comparação visual | formatos/autorizações |
| Rede | handshake WSS, decoder e testes de conformidade | versão/protocolo alvo |
| Gameplay | login → personagem → mundo → movimento multiuser | todas as anteriores |
| Produção | CDN/cache/telemetria/testes cross-browser | fatia jogável |

## Bootcamp primeira semana (ordenar por evidência, não por prazo)
- Dia A: configurar ferramentas, verificações automáticas e browser demo.
- Dia B: auditar código 1.5.3 vs 1.5.2 (build/headers/render/network).
- Dia C: validar build nativa/Windows em laboratório com dependências autorizadas.
- Dia D: compilar um componente portátil de arquivo/matemática real em WASM.
- Dia E: decidir variante preferida por matriz técnica e jurídica; publicar ADR.
Se o build Windows não ocorrer, registrar bloqueio e executar P0 alternativo de parsers isolados.

## Kickoff checklist
- Objetivo e arquitetura lidos.
- Repos e commits de referência fixados.
- Proprietário da decisão legal designado.
- Critérios de go/no-go aceitos.
- Issue tracker priorizado.
- Ambiente de testes separado.
- Manifesto de evidências e benchmarks definido.

## Governança
- Um PR/issue por hipótese. Status: `proposto`, `em análise`, `provado`, `rejeitado`.
- Sem métricas inventadas. Relatar FPS, memória, tamanho inicial do download, latência e erros com condições da medição.
- Mudanças de versão do cliente, engine, protocolo ou licença passam por ADR.
- Revisão de milestone precisa de logs/testes, não screenshots isolados.

## Critério de sucesso inicial
Uma build Emscripten reproduzível com módulo útil da engine escolhida e diagnóstico WebGL2 em browser suportado. Não equivale a port pronto.
