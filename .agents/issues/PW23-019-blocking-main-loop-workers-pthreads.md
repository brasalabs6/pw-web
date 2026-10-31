---
id: PW23-019
title: "Game loop, render/defence threads e waits ainda sem modelo browser validado"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e04-wasm-runtime-v1
scope: pr-23
---

# PW23-019 — Game loop, render/defence threads e waits ainda sem modelo browser validado

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`ElementClient.cpp` usa wWinMain, loop de mensagens e threads; `docs/evidence/client153-full-build.md` lista main/render/defence; `app.mjs` roda apenas viewer RAF. Não há prova de Worker/pthreads/COOP/COEP.

## Impacto / por que importa
Loops síncronos e espera Win32 podem congelar UI. Thread affinity e lifetime dos shims podem quebrar renderer/GNET.

## Solução proposta
Definir ciclo init/tick/render/dispose não bloqueante com RAF/Worker e filas; ativar pthreads somente com isolamento e testes quando necessário.

## Testes e critérios de aceite
Chrome mantém frames e inputs responsivos sob carga, rede e animação; context-loss, shutdown e 2 sessões sem deadlock.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e04-wasm-runtime-v1`](../epics/e04-wasm-runtime-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).

## Revalidação 2026-10-29 — execução síncrona no main thread (blocker de apresentação/entrada)

**HEAD auditado:** `d7be65777b43cefee59fe5ad14465db15d5e36ee` ([commit](https://github.com/brasalabs6/pw-web/commit/d7be65777b43cefee59fe5ad14465db15d5e36ee)). A mudança recente transferiu o probe do Worker/OffscreenCanvas para o **main thread**, mas ainda invoca o cliente original **sincronamente**:

- [`scripts/client153-entry-browser-pre.js:8-17`](https://github.com/brasalabs6/pw-web/blob/d7be65777b43cefee59fe5ad14465db15d5e36ee/scripts/client153-entry-browser-pre.js#L8-L17): `Module.onRuntimeInitialized` roda na thread principal.
- [`scripts/client153-entry-browser-pre.js:18-29`](https://github.com/brasalabs6/pw-web/blob/d7be65777b43cefee59fe5ad14465db15d5e36ee/scripts/client153-entry-browser-pre.js#L18-L29): leituras de pacotes via **XMLHttpRequest síncrono** no main thread, inclusive na inicialização e em cada cache miss.
- [`scripts/client153-entry-browser-pre.js:87-95`](https://github.com/brasalabs6/pw-web/blob/d7be65777b43cefee59fe5ad14465db15d5e36ee/scripts/client153-entry-browser-pre.js#L87-L95): chamada direta `Module._pw153_browser_probe()`, sem agendamento/yield.
- [`scripts/client153-browser-entry.cpp:27-39`](https://github.com/brasalabs6/pw-web/blob/d7be65777b43cefee59fe5ad14465db15d5e36ee/scripts/client153-browser-entry.cpp#L27-L39): `pw153_browser_probe` chama `wWinMain` e só retorna quando ele encerrar.
- [`src/client153/Element/CElementClient/ElementClient.cpp:985-1060`](https://github.com/brasalabs6/pw-web/blob/d7be65777b43cefee59fe5ad14465db15d5e36ee/src/client153/Element/CElementClient/ElementClient.cpp#L985-L1060): após `StartLogin`, o cliente entra em `while(1)` com processamento de mensagens, suspensão de threads e `RunInMainThread()`; não há yield explícito ao event loop do navegador nesse trecho.
- [`scripts/link-client153-full.py:48-56`](https://github.com/brasalabs6/pw-web/blob/d7be65777b43cefee59fe5ad14465db15d5e36ee/scripts/link-client153-full.py#L48-L56): flags do probe não incluem `ASYNCIFY`, `PROXY_TO_PTHREAD` nem estratégia equivalente de suspensão cooperativa.

**Conclusão estática:** enquanto `wWinMain`/carregamento permanecerem executando sincronamente no main thread, o navegador não pode processar normalmente tarefas de UI, entrada, composição e repintura. Mesmo que `Present` realize o blit WebGL2, a apresentação visível pode permanecer bloqueada até que a chamada retorne; se o loop do jogo for contínuo, isso inviabiliza uma sessão interativa. Esta é uma incompatibilidade funcional de portabilidade, **não** hipótese de FPS/tuning. A observação de tela branca no handoff é relato do executor, não reprodução independente nesta rodada. A causa específica da parada atual em `StartLogin` permanece por diagnosticar e não se confunde com o bloqueio do event loop.

**Ação/critério de aceite:** manter WebGL2 no contexto correto e executar a simulação com agendamento cooperativo (`emscripten_set_main_loop`/RAF com init/tick/render separados) **ou** Worker/OffscreenCanvas com protocolo de mensagens e ownership claros; não tentar corrigir somente com `preserveDrawingBuffer` ou otimização de draw calls. Primeiro criar fixture de apresentação de cor/primitiva que repinta durante o boot, responder a evento de input e a timer do host enquanto a simulação avança, depois testar `StartLogin`, `Login.ecwld`, frames originais e input real em Chromium desktop. Revalidar em Android/iOS físicos separadamente; confirmar o loop sem deadlock, incluindo loading/HTTP Range. Nenhum teste browser/mobile ou benchmark independente foi executado nesta auditoria.

**Deduplicação:** mantém-se o ID existente **PW23-019**. PW23-014 cobre o pipeline gráfico e a antiga limpeza após blit; PW23-040/041 cobrem assets, PW23-013 cobre o gate de teste. O clear de diagnóstico anteriormente descrito em PW23-014 foi removido no código atual ([`Present`](https://github.com/brasalabs6/pw-web/blob/d7be65777b43cefee59fe5ad14465db15d5e36ee/src/client153/platform/web/d3d8_device.cpp#L103-L123)), mas a integração/renderização original continua sem comprovação.

## Revalidação incremental 29/10/2026 — probe retorna 0 e o cliente encerra

**Log do operador:** `pw153_browser_probe returned 0` após uma extensa carga de dados/texturas; `/Logs/EC.log` inclui `CECReconnect::ResetState` na cauda. Não existe prova neste recorte de loop/tick/render/present contínuos nem de input vivo; as primeiras 998 linhas de EC foram omitidas no trecho enviado.

**Call graph determinístico no HEAD `7c3f67a0f9aa3c7982a0ef1542bdb6cf1f8e6c1f`:** `scripts/client153-browser-entry.cpp:39-41` devolve o resultado de `wWinMain`. `src/client153/Element/CElementClient/ElementClient.cpp:1008-1134` contém um `while(1)` que pode sair via `WM_QUIT` ou `l_theGame.RunInMainThread()==false` e depois executar cleanup + `return 0`. `EC_Game.cpp:1276-1311` delega a `DispatchWindowsMessage`, `OnceRun`, `Tick`, `Render`. No Win32 shim `platform/common/Windows.h`, `GetMessage` retorna `WM_QUIT` diretamente (L3144+), enquanto `PeekMessage` devolve FALSE (L4376+); não atribuir o motivo de saída a um desses sem stack/trace do caminho executado.

**Prioridade:** instrumentar `StartLogin enter/return`, primeiro loop, `RunInMainThread`, `DispatchWindowsMessage`, `OnceRun`, `Tick`, `Render`, saída `WM_QUIT`/break, `_exit`, cleanup e `return 0`. Acrescentar contadores frame/tick/draw/present e stack/result code da primeira função que encerra. Reproduzir com SHA/relink limpo. Depois tornar o loop cooperativo com `requestAnimationFrame/emscripten_set_main_loop` (ou Worker/OffscreenCanvas com canal testado), sem bloquear main thread ou leituras Range síncronas.

**Aceite:** render de frame original e input/timer do host funcionando ao longo de N frames (com duração registrada), não só init/retorno 0; evidência de login não obtida por JS clear. [Comentário técnico no PR](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6502786138).
