# PW-Web — Programa de otimização

Status: **proposta técnica**, não benchmark validado. Base documental: PR #23 (cliente Angelica 1.5.3, WebAssembly/WebGL2) e PR #17 (servidor C++ comunitário). Atualizar conforme código e testes evoluírem.

## Princípios
1. Fidelidade funcional e compatibilidade de formatos, animações e protocolos são invariantes.
2. Não confundir testes unitários ou demos de modelos com desempenho de gameplay completo.
3. Medir baseline antes de otimizar; toda melhoria requer comparação reprodutível.
4. Preferir mudanças reversíveis, isoladas por interfaces, com fallback.
5. Não reescrever o servidor C++ nem a engine inteira sem evidência quantitativa.
6. Otimizar desktop e mobile com perfis distintos; nunca sacrificar gameplay silenciosamente.
7. Respeitar proveniência e direitos dos arquivos originais; não distribuir assets sem autorização.

## Índice
- [Arquitetura](ARCHITECTURE.md)
- [Renderer](RENDERING.md)
- [Personagens e animações](ANIMATION.md)
- [Assets, streaming e armazenamento](ASSETS.md)
- [Runtime, memória e paralelismo](RUNTIME.md)
- [Rede e servidor](NETWORK-SERVER.md)
- [Mobile](MOBILE.md)
- [Benchmarks e critérios de aceite](BENCHMARKS.md)
- [Roadmap](ROADMAP.md)
- [Riscos e decisões](RISKS.md)

## Estado verificado por metadados dos PRs
PR #23 declara provas de AFile, AString, ATime, matemática, containers, BON e renderização MOX/DDS em WebGL2; skeleton/skinning, mundo, UI, áudio e gameplay permanecem pendentes. PR #17 declara build do servidor 32-bit em Debian 12, mas execução integrada e deploy ainda pendentes. São declarações do PR, não resultados de testes reproduzidos nesta revisão.

## Fontes
- https://github.com/brasalabs6/pw-web/pull/23
- https://github.com/brasalabs6/pw-web/pull/17
