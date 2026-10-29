# Handoff Grok → Codex — 2026-10-28 22:30 (m-boot: cliente original bootando no Chromium)

## ⚠️ INSTRUÇÃO DO OPERADOR (LEIA PRIMEIRO)
O operador exige **loop contínuo de correção**: ficar iterando (fix → rebuild → testar no navegador via Chrome MCP → fix...) **sem parar** até o cliente E o servidor estarem **100% funcionais** no navegador. Não pare em "quase funcionando". Não pare porque "já está bom". Continue até: login visível, personagem criado, entrada no mundo, movimento, pacotes trocados com o servidor real, duas sessões convergentes.

## Estado atual (o que funciona)

### O cliente original EXECUTA no navegador
- O módulo WASM do entry original (`wWinMain`) **linka, carrega e roda** em Chromium headless
- O contexto **WebGL2 é criado e fica current** (handle=1, make_current=0)
- O VFS lê os **pacotes reais do jogo** (17 .pck/.pkx, ~7.5GB) via HTTP Range streaming
- O boot avança até `CheckImportantFiles()` — o cliente executa o caminho real de init
- **52 requests / 48.6MB / 10.5s** com o read-ahead de blocos 1MiB (era ~4.000 requests / 180+s)

### Infraestrutura de probe (tudo commitado no PR #23, head `0d8f802b`)
- `scripts/client153-entry-server.py` — servidor com Range, manifest e roteamento de pacotes
- `scripts/client153-browser-entry.cpp` — harness que cria o WebGL2 e chama `wWinMain`
- `scripts/client153-entry-browser-pre.js` — pre-js com read-ahead 1MiB + cache LRU
- `scripts/client153-entry-worker.js` — worker bootstrap (offscreen canvas)
- `scripts/client153-entry-page.html` — página de probe
- `tests/client153/browser-entry-probe.cjs` — regressão Chromium headless
- `scripts/link-client153-full.py` — ganhou `--browser-probe-pre-js`, `--extra-objects`, `-sMAX_WEBGL_VERSION=2`

### Link do cliente para testar manualmente
- Servidor persistente: **http://127.0.0.1:8500/**
- Rodar a regressão: `node tests/client153/browser-entry-probe.cjs`

---

## O bloqueio atual: encoding GB18030 vs UTF-8 — **PARCIALMENTE RESOLVIDO**

**O fix GB18030 FUNCIONA!** O `.ecm` check passou (era onde parava antes). O boot agora para em `font\FZL2JW.ttf` — as fontes estão em `element/font/` como **arquivos soltos** (não dentro do fonts.pck, que tem só 2 entradas). O servidor de probe precisa servir os arquivos soltos do `element/font/` além dos .pck.

### O problema raiz
O port converteu TODOS os arquivos-fonte de GB18030 para UTF-8 (via `iconv`). Isto quebrou **TODAS as comparações de strings** entre o código (agora UTF-8) e os dados do jogo (ainda GB18030):

| Boundary | O que acontece | Gravidade |
|---|---|---|
| **Índice dos .pck** | O path `Models\Players\形象\白虎\白虎.ecm` em UTF-8 produz hash CRC32 diferente do path em GB18030 no índice → lookup falha | **BLOQUEIA O BOOT** (é onde para agora) |
| **Config files** | Strings lidas de .ini/.data (GB18030) não batem com strings do código (UTF-8) | **BLOQUEIA** (ainda não chegamos aqui) |
| **Protocolo de rede** | O servidor manda GB18030; o cliente compara com UTF-8 | **BLOQUEIA m-server** |
| **Renderização de fonte** | O jogo manda bytes para o font renderer; GB18030 ≠ UTF-8 = glifos errados | **Tela de login quebrada** |
| **Nomes de itens/skills/NPCs** | Milhares de strings chinesas que não batem | **Gameplay quebrado** |

### O mecanismo exato da falha
```cpp
// ElementClient.cpp:393 — a string no código compilado (UTF-8):
"Models\\Players\\形象\\白虎\\白虎.ecm"
// UTF-8 bytes: 4D 6F 64 65 6C 73 ... E5 BD A2 E8 B1 A1 ... E7 99 BD E8 99 8E

// O índice do models.pck/pkx (GB18030):
// GB18030 bytes: 4D 6F 64 65 6C 73 ... D0 CE CF F3 ... B0 D7 BB A2

// a_MakeIDFromLowString() computa CRC32 sobre os bytes crus
// UTF-8 hash ≠ GB18030 hash → GetFileEntry() retorna NULL → boot falha
```

### O que já foi tentado
1. **Script `scripts/revert_strings_gb18030.py`** — converte seletivamente string literals de UTF-8 de volta para GB18030 (mantendo comentários em UTF-8). Aplicado a 3 arquivos críticos (ElementClient.cpp: 812 literals, EC_Player.cpp: 1616, EC_Game.cpp: 61). **ElementClient.cpp compilou sem erros** com os bytes GB18030 crus nos literals.

2. **Verificação byte-a-byte**: o path `.ecm` no arquivo convertido tem os bytes corretos:
   ```
   \xd0\xce\xcf\xf3 = 形象 em GB18030 ✓
   \xb0\xd7\xbb\xa2 = 白虎 em GB18030 ✓
   ```

3. **AINDA NÃO TESTADO**: se o hash agora bate com o índice do pacote (precisa rebuild do entry module + probe Chromium).

### Preocupação do ChatGPT (válida)
Clang pode reclamar de bytes GB18030 inválidos como UTF-8 em arquivos .cpp. Alguns bytes GB18030 (ex: `0xB0`) não são lead bytes UTF-8 válidos. **O ElementClient.cpp compilou sem erros**, mas isso precisa ser verificado em todos os 909 arquivos afetados.

### Alternativa mais segura (sugestão do ChatGPT)
Usar `\xNN` escapes em vez de bytes crus:
```cpp
// Em vez de: "Models\\Players\\形象\\白虎\\白虎.ecm"  (bytes GB18030 crus)
// Usar:      "Models\\Players\\...\xD0\xCE\xCF\xF3\\...\xB0\xD7\xB\xA2\\..."
```
Isto mantém o arquivo .cpp 100% ASCII/UTF-8 e especifica os bytes explicitamente.

### Escala do problema
- **909 arquivos** com string literals que diferem da conversão UTF-8 pura
- **~2.500+ string literals** só nos 3 arquivos críticos já processados
- Estimativa: **dezenas de milhares** de literals chineses em todo o código do jogo

---

## O que falta (ordem de prioridade)

### P0 — Desbloquear o boot (o encoding)
1. **Completar a reversão GB18030**: aplicar o script (ou a variante `\xNN`) a todos os arquivos com strings chinesas
2. **OU** implementar conversão UTF-8→GB18030 no boundary do `GetFileEntry()` (mais cirúrgico, menos invasivo)
3. **Verificar**: o hash do path `.ecm` bate com o índice do models.pck/pkx
4. **Rebuild** do entry module + probe Chromium → o boot deve passar de `CheckImportantFiles()`

### P1 — Chegar à tela de login
5. O boot passa de `CheckImportantFiles()` → o que vem depois? (config loading, device init, UI creation)
6. Cada novo obstáculo será um seam ou encoding issue — iterar
7. A UI de login (AUI) renderiza via WebGL2 → pode precisar do font renderer funcionando com GB18030

### P2 — Cache offline + progresso (o operador pediu)
8. **OPFS**: download integral dos pacotes para Origin Private File System (persistente entre sessões)
9. **Prefetch com prioridade**: na ordem real medida do boot (configs → facedata → gfx → grasses → help/script → interfaces → building → models → models.pkx)
10. **Shell Vite.js** com progresso: tamanho baixado/total por pacote, velocidade, ETA
11. **Service Worker** para cache offline

### P3 — m-server (protocolo real)
12. Transporte GNET → WebSocket (o browser não faz TCP raw)
13. Integração com o servidor PR #17 (read-only em `/home/nonexphere/workspace/pw/pw-source-153`)
14. Auth → personagem → mundo → movimento → pacotes
15. Duas sessões convergentes (evidência final do goal)

---

## Comandos de reprodução

```bash
# Servir o cliente (persistente)
cd /home/nonexphere/workspace/pw/pw-browser-port
python3 scripts/client153-entry-server.py 8500 &
# Abrir http://127.0.0.1:8500/

# Regressão Chromium headless
node tests/client153/browser-entry-probe.cjs

# Rebuild do entry module
python3 scripts/link-client153-full.py \
  --entry-symbol pw153_browser_probe \
  --browser-probe-pre-js scripts/client153-entry-browser-pre.js \
  --extra-objects build/client153-entry-browser/entry.o \
  --output build/client153-entry-browser/entry.js

# Script de reversão GB18030 (dry-run primeiro!)
python3 scripts/revert_strings_gb18030.py --dry-run <arquivos>
python3 scripts/revert_strings_gb18030.py <arquivos>
```

---

## Regras do repo (não esquecer)
- **Nunca** `git add -A` (agentes concorrentes na worktree)
- Push com fetch+rebase (outro agente empurra docs continuamente)
- Proveniência: commit upstream `e3d6d007...` + sha256 no `source-manifest.json`
- Adaptações mínimas marcadas com `//\t`
- Windows.h ordem estrita: types → structs → inline functions → seam includes no fim
- Macros SAL em `strmif.h`, NÃO em Windows.h (colisão com GCC 16 structured bindings)
- `-Wno-template-body` é GNU-only (em++/clang rejeita unknown -Wno-*)

## CI
native-and-wasm: **PASS** no head `0d8f802b` (o último commit meu pushado). Os 5 checks verdes.

### Fonte: rasterização parcial no browser (2026-10-29)

O DC em memória agora aceita criação e seleção de fontes, medição de texto UTF-16 e a combinação `ExtTextOutW(ETO_OPAQUE, rect=null, dx=null)` usada pelo atlas original. No WASM usa `OffscreenCanvas` 2D e escreve BGRA no DIB; testes do bitmap e teste de pixel no Chromium passaram. Com os objetos `A3DFontMan.cpp` e `A3DFont.cpp` reconstruídos, o probe original deixou de registrar falha ao criar a fonte do sistema, mas encerrou mais adiante por `world_targets.txt` ausente no servidor isolado.

Limites: não carrega nem registra os arquivos TTF originais no browser, e a seleção CSS pode usar uma fonte substituta; não há prova de texto na interface real. As métricas são aproximadas e as demais combinações GDI falham explicitamente. O cliente precisa de build limpo que recompile os consumidores dos antigos stubs inline; falta validar o atlas, seus formatos de textura e a UI com os assets reais.
