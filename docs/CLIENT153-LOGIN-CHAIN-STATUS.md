# Estado do port do cliente 1.5.3 para o navegador — login chain

Data: 2026-10-31. Worktree `pw-browser-port`, branch `feat/client153-common-containers` (PR #23).
Este documento registra o estado factual da integração cliente↔servidor no navegador,
com evidência e os próximos passos exatos. Não declara conclusão de milestone do goal.

## O que funciona (com evidência)

### Build e execução
- O cliente real 1.5.3 compila para WASM e boota no Chrome: engine Angelica, assets reais
  dos PCKs autorizados, dados reais (elements.data md5 verificado no boot).
- Pipeline gráfico: shaders de vértice vs.1.1 E pixel ps.1.1/ps.1.4 traduzidos para
  GLSL ES 300 e compilados pelo device WebGL2 — o boot roda com ZERO falhas de shader
  (commits ff9092b1, ce0f3ae8). O corpus: 156 vertex shaders + 165 pixel shaders
  assemblam; os 5 ps_2_0 (efeitos gfx 2.2) falham com classificação honesta.
- A UI de login original renderiza (painéis, botões) e responde ao mouse — o usuário
  clicou botões no jogo real no navegador.
- Suítes: native 39/39, wasm-real 46/46, sanitize 39/39. CI verde nos 4 últimos commits.

### Input
- A ponte de input cobre a superfície que a AUI consome: a posição do cursor alimenta
  GetCursorPos/ScreenToClient (o hit-test da AUI lê o cursor, não a fila de mensagens),
  os botões do mouse alimentam GetAsyncKeyState, teclas imprimíveis postam WM_CHAR para
  as caixas de edição, e o segundo clique de um par vira WM_LBUTTONDBLCLK (o comando
  confirm da lista de servidores dispara nele).
- O duplo-clique na lista de servidores avança o fluxo: a cena troca e o diálogo
  Win_Login aparece (retângulo (472,384)-(952,634) confirmado por diagnóstico AUI).

### Rede — a descoberta desta sessão (commit a72408f0)
- A ponte WebSocket↔TCP (`scripts/client153-ws-tcp-bridge.py`, 29001→29000) conecta o
  navegador ao glinkd local (o stack do servidor no container `pw153-runtime`, PR #17).
- **Causa raiz de todas as falhas de conexão anteriores**: o Emscripten SOCKFS oferece o
  subprotocolo `"binary"` no handshake WebSocket; a ponte não selecionava nenhum — o
  browser falhava o handshake logo após o connect, a máscara de connect-falho reportava
  o socket como gravável, a máquina de estados do GNET acreditava na conexão, e o
  primeiro recv retornava 0 (leitura de par fechado). Toda conexão morria assim.
- **Com o fix do subprotocolo** (a ponte seleciona o subprotocolo oferecido):
  - A sonda WebSocket direta completa o handshake e o glinkd responde ao anúncio de
    versão com o challenge.
  - O trace de rede (ring buffer + export `_pw153_net_trace_dump`) prova a cadeia
    completa no alvo navegador: o ActiveIO registra, o poll reporta o connect, o
    StreamIO assume, e os bytes do challenge chegam (recv=60).
  - O ping da lista de servidores completa o ciclo de vida inteiro pela primeira vez:
    challenge recebido, status lido, fechamento pelo próprio cliente
    (`GameClient::OnRecvProtocol` STATE_PINGING → Close(sid)).

## O que NÃO funciona ainda (os bloqueadores, em ordem)

1. **O login não envia o Response.** A conexão de login (o clique em Entrar) conecta,
   recebe o challenge (recv=60) e fecha sem o glinkd registrar o matrixpasswd. O
   cliente recebe o challenge mas não responde. Hipótese em ordem:
   a. O texto digitado não chega aos campos (o WM_CHAR foi implementado mas não foi
      verificado no diálogo de login — o texto não renderiza, então não dá para ver).
   b. O envio do Response falha silenciosamente — o padrão documentado do GNET:
      `SendProtocol` falha silenciosamente quando o tipo do protocolo está ausente do
      array de estado da sessão (`state.cxx`). O array `_state_LoginClient` contém
      CHALLENGE mas a presença do tipo do Response precisa ser verificada.
   c. O processamento do challenge no cliente (EC_GameSession::OnPrtcChallenge) pode
      estar falhando antes do envio.
2. **O texto da UI não renderiza.** Nenhum texto aparece em nenhum diálogo (a fonte
   carrega, mas o caminho de texto não desenha). Isso impede a verificação visual do
   login e é o motivo da percepção de "o jogo não renderiza".
3. **Sprites com falha de assets não desenham** (a classe B17: os caminhos de textura
   com codificação mista falham; o GetTexture com bounds-check faz o draw falhar
   suavemente em vez de crashar). Os painéis desenham; partes dos fundos não.
4. **ps_2_0** (5 shaders de efeitos gfx 2.2) fora do assembler — classificação honesta.
5. **CECWorld::LoadWorld: File operation error (line 615)** — aparece no boot; a causa
   não investigada.

## Infraestrutura de diagnóstico deixada no lugar

- `src/client153/platform/common/net_trace.h` + o export `_pw153_net_trace_dump`: o
  ring buffer de trace de rede (o registro do PollIO, os eventos do poll, o resultado
  do recv no StreamIO::PollIn). Uso: `Module._pw153_net_trace_dump()` via
  evaluate_script no console da página. O DevTools derruba o histórico do console
  sob a taxa de mensagens do boot — o ring buffer é o canal confiável.
- A ponte: `python3 scripts/client153-ws-tcp-bridge.py 29001 127.0.0.1 29000` (o
  subprotocolo "binary" já selecionado).
- O entry server com a lista local:
  `python3 scripts/client153-entry-server.py 8510 --pair-dir build/client153-wasm/src/client153 --serverlist scripts/client153-serverlist-local.txt`.
- O stack do servidor: o container `pw153-runtime` (boot em
  `/work/scripts/boot-server153.sh --fresh --wait 25` dentro do container; o glinkd
  escuta no host 127.0.0.1:29000). Conta de dev-auth: 1000000/1000000.
- As coordenadas do login (o canvas em getBoundingClientRect(), 1:1 1024x768):
  a lista de servidores Win_LoginServerList (136,154)-(887,614), a primeira linha da
  Lst_Server ≈ canvas (500,270) com duplo-clique; o diálogo Win_Login (472,384)-(952,634):
  a conta (595,435)-(815,467), a senha (595,475)-(815,507), o botão Entrar
  (597,550)-(805,633).

## Próximos passos (a ordem exata de retomada)

1. Verificar se o WM_CHAR chega à caixa de edição do login: com a página no diálogo
   Win_Login, clicar no campo da conta (canvas 700,450), digitar, e inspecionar o
   estado do campo (a AUI EditBox guarda o texto — um diagnóstico no GetText do campo
   ou no OnChar da EditBox confirma). Se o texto não chega, depurar o foco
   (o clique no campo seta o foco?) e o roteamento do WM_CHAR.
2. Verificar o envio do Response: um diagnóstico no SendProtocol do cliente (o
   `pw153::nettrace::push` no caminho de envio) mostra se o Response é construído e
   se o check do array de estado o rejeita. Se o tipo está ausente de
   `_state_LoginClient`, adicionar (o mesmo padrão do fix db85fa5 no servidor).
3. Com o Response chegando ao glinkd, a cadeia documentada da memória
   (topics/pw-server153-login-chain.md) continua: matrixpasswd → gauthd → userlogin →
   rolelist → a seleção/criação de personagem → enterworld.
4. O texto da UI (a fonte): o caminho de texto da AUI não desenha — investigar o
   A3DFTFont/FreeType no alvo navegador (o driver ftdriver_original + freetype210
   estão linkados; o diagnóstico começa em onde a AUI pede o texto e o que o
   renderizador de fonte faz).
