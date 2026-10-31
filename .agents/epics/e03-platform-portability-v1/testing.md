# Testes — Desacoplamento de Win32, SDK e Direct3D

Positivo e negativo por workorder são obrigatórios. Ferramenta ausente significa
BLOCKED; teste executado com falha significa FAIL. Build sozinho não é prova runtime.

## WO-01

- Positivo: O inventário identifica acessos D3D fora de A3DDevice e não trata contagem textual como call graph completo.
- Negativo: Arquivo duplicado ou #if desativado não recebe classificação de código ativo sem evidência.
- Comando/ação: `python3 -m unittest discover -s tests -p 'test_platform_inventory.py' -v`.

## WO-02

- Positivo: WO-07 a WO-10 recebem assinaturas/fixtures fechadas e falhas explícitas; não existe retorno success sem efeito.
- Negativo: Pedido de socket raw, registry ou ActiveX é marcado unsupported/fora da fatia, não simulado.
- Comando/ação: `Revisão semântica de cada operação + ./scripts/verify.sh; não há código de plataforma a executar nesta WO.`.

## WO-03

- Positivo: Bytes carregados antes do tick produzem o mesmo resultado nativo/WASM e recursos são liberados ao finalizar.
- Negativo: Leitura fora do intervalo ou caminho rejeitado interrompe a operação sem dados parciais tratados como sucesso.
- Comando/ação: `./scripts/test-platform.sh (criar nesta WO; integrar runners existentes sem substituir suites)`.

## WO-04

- Positivo: Pressionar, perder foco e retornar gera snapshots e deltas definidos, com um único tick por frame.
- Negativo: Eventos atrasados após dispose e tecla solta fora da janela não mantêm input preso.
- Comando/ação: `node --test tests/platform/frame-input.test.mjs (criar) e autoteste browser do runtime`.

## WO-05

- Positivo: Gate wo-05 libera apenas a plataforma testada e registra capacidades não suportadas com evidência.
- Negativo: Um único caso obrigatório pulado impede PASS agregado.
- Comando/ação: `./scripts/test-render-clip.sh; ./scripts/test-platform.sh; ./scripts/verify.sh`.

## WO-06

- Positivo: Todos os valores esperados em R0 coincidem com tolerância 1e-6; índices/cores são exatos e layout tem 16 bytes.
- Negativo: NaN, overflow da transformação, count inválido, viewport zero ou índice fora do limite retornam InvalidInput sem escrita parcial.
- Comando/ação: `./scripts/test-render-clip.sh (criar nesta WO); ./scripts/verify.sh`.

## WO-07

- Positivo: A sequência conhecida de timestamps produz os deltas exatos e o reset especificado.
- Negativo: Relógio regressivo/NaN é rejeitado; não se corrige usando Date.now sem autorização do contrato.
- Comando/ação: `./scripts/test-clock.sh (criar nesta WO)`.

## WO-08

- Positivo: Caminho lógico permitido mantém identidade estável; nenhuma entrada consegue escapar do namespace de assets.
- Negativo: A/../B, C:\x, UNC e NUL retornam erro explícito, sem normalização permissiva.
- Comando/ação: `./scripts/test-paths.sh (criar nesta WO)`.

## WO-09

- Positivo: Uma sequência press/move/release gera snapshot esperado sem duplicação entre frames.
- Negativo: Blur com botão pressionado limpa estado e eventos pós-dispose são ignorados.
- Comando/ação: `node --test tests/platform/input.test.mjs (criar) + cenário browser de foco`.

## WO-10

- Positivo: Leituras parciais válidas retornam bytes/cursor esperados nativo/WASM, sem fetch dentro do parser.
- Negativo: Offset+length que estoura inteiro ou ultrapassa o buffer falha antes de acessar memória.
- Comando/ação: `./scripts/test-memory-file.sh (criar nesta WO)`.

## WO-11

- Positivo: Cada recurso tem um único owner e momento de liberação; nenhuma API depende de estado global oculto.
- Negativo: Borrow sobrevivendo ao dispose ou ao memory.grow é explicitamente inválido e tem teste especificado.
- Comando/ação: `Walkthrough de ownership nas assinaturas de platform-v0/R0; ./scripts/verify.sh`.

## WO-12

- Positivo: A closure nova compila sem Windows/D3D e as suites não produzem violações de memória/UB.
- Negativo: Caso sentinela em teste isolado demonstra que o harness detecta erro; sentinela não entra em produção.
- Comando/ação: `./scripts/test-platform-sanitizers.sh (criar nesta WO)`.

## Evidência

Registrar comando, versão, SHA, fixture, resultado e limites em docs/evidence;
logs/artifacts locais não substituem um resultado verificável. Comandos “criar” são
entregáveis futuros; registrar zero casos ou screenshot isolada não satisfaz aceite.
`./scripts/verify.sh` continua obrigatório e não valida o cliente completo.
