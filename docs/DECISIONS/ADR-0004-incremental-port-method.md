# ADR-0004 — Port por fatias com fronteiras de plataforma explícitas

Status: **aceito para pesquisa e planejamento da implementação**.
Data: 2026-10-24. Autorização: pedido de prova e método executável para Grok/GLM.

## Contexto

A POC-001 executou `ARandomGen` original em wasm32 sem alteração e confirmou
paridade com o nativo. A tentativa direta do renderer parou em `Windows.h`.
`GetD3DDevice()` expõe D3D a múltiplos módulos; trocar somente `A3DDevice` não fecha
a fronteira. Os épicos existentes nomeiam interfaces, mas não especificam toda
a semântica necessária para implementação sem decisões adicionais.

## Decisão

1. Preservar a semântica de cena, materiais, animação e gameplay quando validada;
   adaptar plataforma por fatias verticais e fechamento explícito de dependências.
2. Usar as interfaces Angelica como pontos de entrada para o port, incluindo os
   acessos diretos a D3D necessários à fatia. Não construir uma emulação geral de
   Windows/COM/DirectX nem reescrever todo o cliente.
3. Para o primeiro backend integrado, preferir C++/OpenGL ES 3 compilado por
   Emscripten para WebGL 2. JavaScript possui canvas, eventos e ciclo do browser.
   A escolha mantém WebGL 2 da ADR-0002; a prova atual tem renderer JS independente.
4. Começar com uma malha sintética indexada, não com inicialização do cliente inteiro.
   Features fora do contrato retornam erro explícito; não são stubs que fingem sucesso.
5. Cada entrega carrega baseline, contrato, teste positivo/negativo, comandos,
   limitações e diff revisável. O modelo executor não escolhe layouts desconhecidos.
6. Manter referências congeladas e artefatos derivados locais. A integração em
   código distribuído segue a autorização registrada em `LEGAL.md` (2026-10-25).

## Alternativas e consequências

- Emulação ampla de D3D8/Win32 preservaria mais chamadas, mas amplia a superfície
  para APIs sem equivalente no browser e torna falsos stubs difíceis de detectar.
- Reescrever cena/gameplay liberaria a arquitetura, mas perderia a referência de
  comportamento e aumentaria substancialmente o trabalho.
- Port por fatia exige medir e adaptar acessos diretos a D3D. O custo é explícito
  e verificável, em vez de presumir que uma fachada já isola toda a engine.
- Não introduzir protocolo binário de draw commands, ECS, plugins, WebGPU ou
  multithreading antes de uma necessidade medida. Emscripten fornece a ligação GL.

Versão final do cliente continua condicional na ADR-0003. Transporte WSS, servidor
autoritativo e propriedade dos assets permanecem conforme as decisões existentes.

Contrato inicial, tarefas, pontos de parada e prompt de execução:
[ANGELICA-PORTING-METHOD.md](../ANGELICA-PORTING-METHOD.md).
