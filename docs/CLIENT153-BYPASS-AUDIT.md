# Auditoria dos desvios de diagnóstico — port PW 1.5.3 browser (r2)

**Data:** 2026-10-30 · **HEAD:** `91232d3c` · **Critério de classificação:** o
cliente Windows original TOLERA a falha (fallback legítimo — manter) ou
ABORTA (desvio de diagnóstico — restaurar a semântica fatal quando a causa
fechar)? A referência é o código original fixado em
`e3d6d0075d0a745c9f9859f2cb25b01d71dec045`.

## 1. Desvios de diagnóstico — RESTAURAR para fatal quando a causa fechar

Estes `continue` foram introduzidos para o boot alcançar a tela de login
durante a campanha de diagnóstico. **Não podem virar comportamento da build
funcional.** Cada entrada lista a causa raiz que precisa fechar antes da
restauração.

| # | Local | O que mascara | Causa raiz a fechar | Restauração |
|---|---|---|---|---|
| 1 | `EC_Game.cpp:566-574` — `elements.data` load falha e continua | O cliente roda sem dados de itens/elementos (inicialização parcial) | Parser binário: layout MSVC 32-bit vs wasm32 (PW23-040 — `sizeof/alignof/offsetof` das structs, contagens/offsets, endianness) | Restaurar `return false` (o original mostra MessageBox e aborta) quando `load_data` carregar com valores plausíveis |
| 2 | `EC_World.cpp:578` — falha ao abrir `Maps\Login\Login.ecwld` e continua | O mundo de login carrega sem a cena (sem objetos/terreno visual) | O arquivo agora é servido (loose_files); a falha restante é de conteúdo/parse da cena | Restaurar o abort quando a cena carregar e renderizar |
| 3 | `EC_World.cpp:898` — `A3DTerrain2` falha e continua | O terreno do login não renderiza | A cadeia de arquivos .trn2/.t2bk/.t2lm + o parser do terrain2 | Restaurar o abort quando o terreno carregar |
| 4 | `globaldataman.cpp:750-781` — entradas gshop2 ilegíveis são puladas | A loja carrega parcialmente/placeholder | O mesmo parser binário do items.data (o gshop2 usa a mesma família de structs) | Restaurar o abort quando o parser for válido |
| ~~5~~ | ~~`EC_Instance.cpp:143-153`~~ — **FECHADO 2026-10-30 (U04)**: a causa era o `swscanf` do sistema, que não roda sob `-fshort-wchar` (reproduzido: `swscanf(L"160", L"%f")` → -1/0.0; sem a flag funciona). O `GetNextTokenAsFloat` agora usa `wide_tof` do port (commit `736d5a0c`); os `return false` originais foram restaurados verbatim com teste de regressão positivo/negativo dirigindo o parser real (commit `d446c98a`); o instance.txt carrega com **zero skips** e a condição `< 10` nunca dispara com o parse correto. |

**Regra de restauração:** cada item recebe um teste de regressão que prova o
carregamento correto ANTES de o `continue` virar `return false` novamente —
a restauração sem a prova reesconde o problema.

## 2. Fallbacks legítimos do ORIGINAL — MANTER

O cliente Windows original tolera estas falhas (o código original registra e
continua). O port preserva o comportamento — não são desvios.

| # | Local | Evidência do original tolerar |
|---|---|---|
| 1 | `ElementClient.cpp:1290-1300` — `Load GT failure` | O original loga `Load GT %s` (success/failure) e continua; o GTOverlay é o browser in-game, opcional no boot |
| 2 | `A3DSkin.cpp` — texturas ausentes na lista do .ski | O original comenta `// Ignore the error. pTexture = NULL;` — o sistema de rosto carrega as texturas de `facedata/` em runtime |
| 3 | `A3DSysCursor::Load` — cursores ausentes | O original loga e continua (o cursor do sistema substitui) |
| 4 | `A3DTerrainWater::Init` — `water_vs.txt` ausente | O original loga e continua (a água renderiza sem o shader customizado) |
| 5 | `CECFullGlowRender::Init` — target sem A8R8G8B8 | O original loga e continua (o bloom fica desabilitado) |

## 3. Seams explícitos — MANTER (falham alto por design)

| # | Seam | Mensagem de boot |
|---|---|---|
| 1 | defence (anti-cheat) | "the client-side anti-cheat (defence) is not available in the browser build" |
| 2 | DirectSound | "DirectSound is not available in the browser build" |
| 3 | Force feedback (Immersion) | "force feedback ... is not available in the browser build" |
| 4 | SpeedTreeRT | "SpeedTreeRT is not available in the browser build; forest rendering is disabled" |
| 5 | ftdriver (stub) | o probe do ftdriver valida a mensagem "login UI cannot initialize" |
| 6 | D3D gamma ramp | "D3D gamma ramp is not implemented" |

## 4. Instrumentação temporária — REMOVER quando o gate estabilizar

| # | Instrumento | Local |
|---|---|---|
| 1 | `BOOT_A3D*` / `BOOT_A3DDEV_*` markers | `A3DEngine.cpp`, `A3DDevice.cpp` |
| 2 | `PWDBG AFileImage::Init` path tracing | `AFileImage.cpp` |
| 3 | `PWASSET record` per-failure printf | `asset_diag.cpp` (o dump em `/Logs/pwasset.txt` permanece — é o artefato da taxonomia) |
| 4 | `pw153: N browser frames` liveness markers | `ElementClient.cpp` |

## 5. Pendências de verificação contra a referência

- ~~`EC_Instance.cpp:143-153`~~ — **verificado 2026-10-30**: é desvio (o
  original faz `return false`); movido para a tabela 1, item 5.
- `CECServerList::Init` — "server list file error" (linha 179): verificar o
  comportamento do original (o arquivo `userdata\server\serverlist.txt` é
  servido; a falha restante é de parse/encoding).
