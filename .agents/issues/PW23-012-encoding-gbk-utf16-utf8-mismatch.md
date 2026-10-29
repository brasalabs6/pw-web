---
id: PW23-012
title: "Codificações GBK/UTF-16/UTF-8 incompatíveis entre caminhos"
priority: P1
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: 7b04fde30553ed4d0207b2d96fd949e82469fd38
epic: e05-asset-pipeline-v1
scope: pr-23
---

# PW23-012 — Codificações GBK/UTF-16/UTF-8 incompatíveis entre caminhos

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`app.mjs` usa TextDecoder('gbk') em recursos; `atlbase.h` faz conversões narrow usando UTF-8 strict; compilação usa `UNICODE` e `-fshort-wchar` com CRT UTF-16 próprio.

## Impacto / por que importa
Texto/nome de assets/strings GNET/diálogos podem estar em CP936, e macros de conversão ATL podem receber tipos errados.

## Solução proposta
Formalizar encoding por origem e boundary (WinAPI A/W, ACHAR, PCK, Lua, RPC), implementar conversões explícitas CP936/UTF8/UTF16 e falhas claras para sequências inválidas.

## Testes e critérios de aceite
Fixtures nomes reais não ASCII, português/chinês, surrogate pairs, path PCK e string wire; round-trip sem data loss.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e05-asset-pipeline-v1`](../epics/e05-asset-pipeline-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).


## Revalidação 2026-10-29 — conversão GB18030 bloqueada por consulta de tamanho

**Classificação:** defeito determinístico do caminho de abertura de scripts não UTF-16, comprovado por inspeção e reprodutor **isolado**; não é benchmark nem reprodução do cliente completo no Chromium. Mantido neste PW23-012 para não duplicar o mesmo domínio de codificações. **SHA inspecionado:** [`7b04fde30553ed4d0207b2d96fd949e82469fd38`](https://github.com/brasalabs6/pw-web/commit/7b04fde30553ed4d0207b2d96fd949e82469fd38).

- [`AWScriptFile.cpp:112–139`](https://github.com/brasalabs6/pw-web/blob/7b04fde30553ed4d0207b2d96fd949e82469fd38/src/client153/Angelica2/AngelicaFile/Source/AWScriptFile.cpp#L112-L139): para texto sem BOM UTF-16, `AWScriptFile::Open` chama `a_Gb18030ToWide(pBuf, dwFileLen, nullptr, 0)` para calcular o tamanho. Se o resultado não for positivo, libera o buffer e retorna `false` com `Couldn't handle non-unicode file !`.
- [`AAssist.cpp:192–208`](https://github.com/brasalabs6/pw-web/blob/7b04fde30553ed4d0207b2d96fd949e82469fd38/src/client153/Angelica2/AngelicaCommon/Source/AAssist.cpp#L192-L208): a implementação retorna **0 imediatamente** quando `szWide == nullptr` ou `iWideSize <= 0`. Portanto, a consulta de tamanho acima **nunca pode ser bem-sucedida**, mesmo para bytes GB18030 válidos. A conversão subsequente jamais é alcançada nesse fluxo.
- **Reprodutor isolado desta auditoria:** transcrição literal da função de `AAssist.cpp`, compilada no host com `g++ -std=c++17 -fshort-wchar` e `iconv`; entrada ASCII válida `world_targets` (13 bytes). Resultado: `size_probe=0, actual_conversion=13, wchar_bytes=2`. A conversão com buffer fornecido funciona no host, mas a consulta `nullptr,0` falha incondicionalmente. **Não** foi compilado o repositório inteiro, nem executado WASM, navegador ou celular.

**Impacto funcional:** todo script não UTF-16 que depende deste fallback falha ao abrir; o código menciona `world_targets.txt` como exemplo. Não está demonstrado que este seja a causa do erro de `help/script.pck`, `elements.data` ou `Login.ecwld`; investigar essas falhas separadamente. O port não deve considerar o caminho GB18030 funcional.

**Correção e aceite (antes de tuning):** implementar consulta de comprimento válida em `a_Gb18030ToWide` (ou conversão em duas passagens com capacidade verificada), verificar a segunda chamada e tratar sequências inválidas/overflow. Acrescentar teste nativo e WASM para entrada GB18030 sem BOM (ASCII e caracteres chineses), UTF-16LE com BOM e dados inválidos; validar `AWScriptFile::Open` retornando sucesso para os fixtures válidos e reportando erro claro para os inválidos. Registrar SHA e logs de execução no Chromium antes de marcar resolvido. Preservar o comportamento dos arquivos UTF-16 originais.

## Complemento de encoding (29/10) — atenção a dupla transcodificação

O relatório local do executor afirma simultaneamente (a) reversão de 2.489 literais chineses para bytes GB18030 e (b) conversão `UTF-8 → GB18030` no boundary `AFilePackGame::GetFileEntry`. **Risco a testar:** se o caller passar um literal **já codificado em GB18030** ao conversor que espera UTF-8, haverá erro ou bytes diferentes no CRC32. Essa possibilidade não confirma que a dupla conversão ocorreu no HEAD observado; auditar o caminho com fixture real.

**Critério:** documentar encoding por origem e ter conversão **uma única vez** em cada borda (PCK path hash, scripts, configs, GNET). Dois testes separados com os mesmos caracteres chineses: caminho UTF8→GB18030→CRC32; caminho GB18030→CRC32 diretamente; ambos têm hash/lookup idêntico. Não aplicar conversão automática a `char*` sem contrato, nem reverter todas as fontes no escuro. [Análise no PR](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6499021552).

## Revalidação com log Chromium de 29/10/2026 — esta falha agora é o primeiro bloqueador fatal

**Evidência do operador (11:14:10):** `/Logs/AF.log` contém `AWScriptFile::Open, Couldn't handle non-unicode file !`, seguido imediatamente por `/Logs/EC.log` com `GlobalData_Load(), Failed to open file world_targets`, `CECGame::Init` abortando na linha 553 e `wWinMain` na 935. `A3DEngine/A3DDevice` passou (formato X8R8G8B8, device 800x600x32, software skin). Esta é a primeira falha fatal observada no novo probe; não é problema gráfico.

**Call graph verificado em HEAD `7d76a6c77cbeb9537a6b2c79fc99d14c6f82c152`:**
- `Element/CElementClient/EC_Game.cpp:550-556` exige `globaldata_load()`.
- `Element/CCommon/globaldataman.cpp:446-454` abre `maps\\world\\world_targets.txt` via `AWScriptFile`.
- `Angelica2/AngelicaFile/Source/AWScriptFile.cpp:110-139` chama `a_Gb18030ToWide(pBuf,len,nullptr,0)` para a consulta de capacidade se não houver BOM UTF-16, e depois deixa de continuar quando retorna zero.
- `Angelica2/AngelicaCommon/Source/AAssist.cpp:192-208` **sempre devolve zero** para `szWide==nullptr` ou `iWideSize==0`. O contrato de consulta é impossível no estado atual, independentemente da validade do GB18030. Este defeito foi isoladamente reproduzido em inspeção anterior e agora é corroborado pelo log do fluxo do jogo, embora não se tenham executado os assets nesta revisão.

**Ação:** implementar consulta de tamanho UTF-16LE real (contar unidades de 16 bits através de iconv com buffer de scratch, tratar E2BIG como continuação, rejeitar sequências inválidas/overflow) ou converter uma única vez a buffer com capacidade verificada; checar resultado da segunda conversão e garantir `sizeof(wchar_t)==2` no target; tratar arquivos de 1 byte antes do cast BOM. Validar `world_targets.txt` do manifest (já listado em `scripts/client153-entry-server.py:45`), seus bytes/codificação e testes nativo/wasm/browser para ASCII, chinês GB18030, UTF-16 com BOM, truncado/malformado. **Não contornar `globaldata_load` nem declarar PASS sem tokens e contagens válidos.** Depois desse gate, esperar novos erros de `Data/domain.data` ou `elements.data` e depurar separadamente.

[Comentário técnico publicado no PR](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6499305374).

## Evidência incremental 29/10/2026 — probe avançou além do script, mas paths asiáticos ainda exigem prova por bytes

O operador encaminhou novo recorte com `pw153_browser_probe returned 0` e logs avançados de sprites, personagens, texturas/modelos após `globaldata_load`. Isso indica avanço além do bloqueio fatal de `world_targets` do log anterior, **sem demonstrar qual patch corrigiu a consulta `a_Gb18030ToWide(..., nullptr, 0)` nem provar regressão resolvida**. Manter issue aberta até commit de correção + testes de contract/fixture.

Os paths com `����` no console podem ser **artefato de `FS.readFile('/Logs/A3D.log', {encoding:'utf8'})`** decodificando bytes GB18030 como UTF-8, e não necessariamente bytes de busca incorretos no `.pck`. Antes de alterar novamente literais, capturar filename em hex no caller e no lookup `AFilePackGame`, comparar bytes e CRC32 com índice do pacote, e ler log raw bytes decodificados como GB18030 vs UTF-8. Separar falha de lookup de falha por `D3DXGetImageInfoFromFileInMemory`/texture upload (PW23-014). Testar uma amostra chinesa e uma ASCII com resultados positivos/negativos explícitos. [Comentário técnico no PR](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6502786138).
