---
id: PW23-043
title: "Boot WASM falha em A3DEngine::Init; identificar primeiro false e corrigir contratos D3D8"
priority: P0
status: open
kind: browser-boot-blocker
first_observed: 2026-10-29
verified_against: fc2105438eaf982812a7488ab7b65f0b9dfe51bd
epic: e06-render-webgl2-v1
scope: pr-23
---

# PW23-043 — Boot WASM falha em A3DEngine::Init

**Estado:** `open`. O log do operador comprova falha externa no init; o primeiro `return false` interno **ainda precisa ser identificado no ambiente local**. Não tratar hipóteses de causa como fato reproduzido.

## Evidência de boot
O operador observou:
```text
pw153_browser_probe: context handle = 1
pw153_browser_probe: make_current = 0
BOOT_CHECK_FILES_OK: all important files opened
BOOT_WWINMAIN_CHECK_FILES_PASSED
CECGame::InitA3DEngine: Failed to call function. (line: 1485)
CECGame::Init: Failed to call function. (line: 382)
wWinMain: Failed to call function. (line: 935)
pw153_browser_probe returned -1
```
Logo `Element/CElementClient/EC_Game.cpp:1483-1487` retornou false porque `m_pA3DEngine->Init(...)` falhou, e as duas falhas seguintes são propagação. O canvas final verde é um clear JS de diagnóstico **após** o probe; não indica game frame.

O arquivo `/font/FZL2JW.ttf` existe e todas as verificações importantes passaram; `/font/fzl2jw.ttf` com case errado é diagnóstico secundário. Defaults do `SystemSettings.ini`, aviso GT, falha esperada do DirectSound e force-feedback **não provam** que são a causa do retorno.

## Achado imediato: caminho de log testado com casing errado
`Angelica2/AngelicaCommon/Source/ALog.cpp:51` inicia `ALog::m_szLogDir = "Logs"` e `ALog::Init` grava em `Logs/`. O probe verifica `/logs/AF.log` e `/logs/EC.log` em minúsculas e **não procura `/Logs/A3D.log`**, arquivo do init gráfico (`Angelica3D/Source/A3DEngine.cpp:162`). Isso não comprova ausência de log. Conferir `ALog::GetLogDir()`, `FS.readdir('/Logs')` e ler A3D/AM/EC; caso não existam, medir `ALog::Init`, errno e cwd diretamente no console.

## Achados do código do adaptador (não necessariamente a primeira falha nesta execução)
1. `platform/web/d3d8_device.cpp::Device::GetDeviceCaps` devolve `E_NOTIMPL` embora `Angelica3D/Source/A3DDevice.cpp::ConfigDevice()` dependa dele (L480+). Em `A3DDevice::Init` há chamada a `ConfigDevice()` (cerca L437) **sem checar o bool de retorno**. Implementar caps **reais** ou falhar claramente; nunca ignorar falha de config.
2. `Device::GetRenderTarget` e `GetDepthStencilSurface` definem `*out=nullptr` e devolvem `D3D_OK`; `GetBackBuffer` repassa. `A3DDevice::Init` somente checa o HRESULT e guarda os ponteiros (L397-413). Corrigir semântica: objetos de superfície válidos com lifetime/refcount/desc e representação apropriada do default framebuffer, ou falha explícita até implementar. Nunca `D3D_OK + nullptr`.
3. `SetRenderState`, `SetTextureStageState`, luzes, materiais, índices, streams, shaders/draws continuam parcialmente `E_NOTIMPL`. Esses são bloqueios subsequentes para render real (correlacionar com PW23-014), não afirmar que cada um já causou o `-1`.

## Plano de reprodução e correção
1. Capturar o HEAD e fazer rebuild limpo do entry WASM em Chromium.
2. Inspecionar `/Logs/A3D.log`, `/Logs/AM.log`, `/Logs/EC.log` ANTES de encerrar o módulo (não presumir `/logs`). Confirmar que `g_A3DErrLog.Init("A3D.log")` passou.
3. Instrumentar `A3DEngine::Init` por estágio: ALog, AMEngine, InitSoundEngine, InitVideoEngine, Direct3DCreate8, A3DDevice, LightMan, TextureMan, FontMan, GFXMan, SkinMan, demais managers. Registrar primeiro estágio que devolve false e HRESULT em hex quando disponível, usando stderr/console independente de ALog.
4. Caso falhe no dispositivo, instrumentar D3D8 factory `CheckDeviceType/GetDeviceCaps/CreateDevice` e device `GetRenderTarget/GetDepthStencilSurface/GetBackBuffer/GetDeviceCaps/ConfigDevice`, anotando retorno e validade do objeto.
5. Corrigir contrato real, executar test de regressão unitário e boot Chromium, preservar código original Angelica, sem `return true` de fachada.
6. Publicar no PR evidência do primeiro erro, patch, comando e último marker executado.

## Critérios de aceite
- O primeiro `return false` está identificado com prova (logs ou markers) no mesmo HEAD e build limpo.
- Chamadas D3D8 indispensáveis não reportam sucesso com recurso nulo, e erros em `ConfigDevice` não são mascarados.
- `A3DEngine::Init` retorna verdadeiro por um caminho válido; em seguida `CECGame::Init` progride com evidência do próximo gate no navegador. A conclusão **não depende** de canvas verde JS ou timeout de execução.
- Atualizar status e `resolved_at`, commit, testes nativo/WASM/Chrome e screenshots quando fechado; não apagar o histórico.

## Rastreabilidade
- [Comentário diagnóstico no PR](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6498118668)
- [PW23-014 — Renderer Angelica D3D8](PW23-014-angelica-d3d8-webgl2-not-integrated.md)
- [Épico e06 — WebGL2](../epics/e06-render-webgl2-v1/README.md)
- Baseline: `fc2105438eaf982812a7488ab7b65f0b9dfe51bd`. Revalidar no HEAD mais recente do executor.

## Complemento de triagem (29/10) — hipótese de GetAdapterDisplayMode e escopo do primeiro erro

O relatório do executor afirma que os novos marcadores `BOOT_A3DDEV_*` estão sendo adicionados mas **ainda não foram rodados no Chromium**. O primeiro `return false` de `A3DDevice::Init` permanece desconhecido.

**Nova hipótese verificável:** em `platform/web/d3d8_factory.cpp:84-89`, `Factory::GetAdapterDisplayMode` consulta `GL_VIEWPORT` e depois chama `glGetError()`, interpretando **qualquer erro GL pendente** como erro dessa operação. Chamadas anteriores de `renderable_color()`/framebuffer podem deixar um erro no contexto; medir estado GL antes e depois, viewport e HRESULT no probe. Não concluir que esse é o erro de boot sem os marcadores.

**Contrato de render:** `Device::GetDeviceCaps` deve deixar de devolver `E_NOTIMPL`, enquanto `Factory::GetDeviceCaps` precisa ser consistente; checar `ConfigDevice()` dentro de `A3DDevice::Init`, atualmente ignorado. Entretanto essa falha ignorada e `GetRenderTarget` que devolve `D3D_OK + nullptr` **não explicam diretamente** a cascata de `return false`, sem evidência adicional.

Prova de aceite da próxima rodada: commit/SHA do entry + `/Logs/A3D.log`, markers de FMT16, adapter display, factory caps, CreateDevice e superfície; apontar **o primeiro HRESULT incorreto**, corrigir e retestar. [Orientação aprofundada no PR](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6499021552).
