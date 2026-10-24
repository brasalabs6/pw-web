# Testes — Baseline do cliente Windows e seleção validada da Angelica

Positivo e negativo por workorder são obrigatórios. Ferramenta ausente significa
BLOCKED; teste executado com falha significa FAIL. Build sozinho não é prova runtime.

## WO-01

- Positivo: Inventário identifica os 21 projetos das soluções principais 1.5.3 e explica dependências por configuração, sem inferir sucesso do linker.
- Negativo: Projeto com path ausente, macro e case diferente produz três classificações distintas; nenhum é silenciosamente descartado.
- Comando/ação: `python3 -m unittest discover -s tests -p 'test_client_inventory.py' -v`.

## WO-02

- Positivo: A matriz distingue presença de .lib, compatibilidade ABI, build e execução; nenhum campo pendente aparece como compatível.
- Negativo: A diferença DWORD Linux=8/wasm32=4 da POC permanece explícita; nome da pasta não é evidência de versão.
- Comando/ação: `Revisar cada linha contra inventário e docs/POC-ANGELICA.md; ./scripts/verify.sh`.

## WO-03

- Positivo: Um executável reconstruído de fontes e dependências identificadas tem SHA e build reproduzível em diretório vazio.
- Negativo: Biblioteca ausente ou máquina incompatível produz FAIL/BLOCKED; artefato antigo não conta como build novo.
- Comando/ação: `Executar scripts/research/build-client-windows.ps1 no laboratório e registrar a linha vcbuild/MSBuild escolhida em WO-06.`.

## WO-04

- Positivo: Cada capacidade possui resultado observado e artefato; somente entrada real no mundo satisfaz a capacidade mundo.
- Negativo: Cliente que abre janela mas falha login permanece parcial e não libera seleção definitiva por suposição.
- Comando/ação: `Runbook Windows de WO-03 + passos de runtime versionados neste relatório; não existe comando genérico de login.`.

## WO-05

- Positivo: ADR aceita ou rejeitada contém evidências e decisão identificada; o gate terminal continua significando seleção validada.
- Negativo: Sem build ou direitos, manter BLOCKED/proposto; não marcar wo-05 concluída para liberar outros épicos.
- Comando/ação: `Revisão humana da ADR e matriz; ./scripts/verify.sh`.

## WO-06

- Positivo: Preflight distingue compilador, headers e linker disponíveis e passa no laboratório usado em WO-03.
- Negativo: VS de arquitetura errada ou d3dx8 ausente bloqueia antes do build do cliente.
- Comando/ação: `powershell -File scripts/research/check-windows-toolchain.ps1`.

## WO-07

- Positivo: Mapa de linker permite identificar produtor e arquitetura de cada biblioteca consumida pelo cliente.
- Negativo: Uma lib encontrada pelo nome com CRT/arquitetura incompatível é rejeitada, não reutilizada silenciosamente.
- Comando/ação: `powershell -File scripts/research/build-libraries-windows.ps1`.

## WO-08

- Positivo: Todos os caminhos e símbolos de Release têm resolução demonstrada ou bloqueio nomeado antes de WO-03.
- Negativo: Mistura acidental com Angelica2.2 ou SDK x64 é identificada na revisão.
- Comando/ação: `Executar preflight de WO-06 e inspeção dumpbin/linker com comandos exatos no relatório.`.

## WO-09

- Positivo: WO-04 recebe manifesto sanitizado de recursos e destino de laboratório cuja compatibilidade será testada.
- Negativo: Arquivo com versão ou hash diferente bloqueia o cenário; presença de um .pck qualquer não é suficiente.
- Comando/ação: `Runbook de hashing e conferência definido no relatório, sem copiar assets para Git.`.

## WO-10

- Positivo: Relatório explica custo e evidência de cada alternativa sem declarar 1.5.2 funcional por README.
- Negativo: Headers/libs entre versões nunca são combinados como solução para missing symbols.
- Comando/ação: `./scripts/verify.sh e revisão da matriz por commit; build alternativo só no ambiente registrado.`.

## Evidência

Registrar comando, versão, SHA, fixture, resultado e limites em docs/evidence;
logs/artifacts locais não substituem um resultado verificável. Comandos “criar” são
entregáveis futuros; registrar zero casos ou screenshot isolada não satisfaz aceite.
`./scripts/verify.sh` continua obrigatório e não valida o cliente completo.
