---
id: PW23-044
title: "CECServerList ignora serverlist.txt presente por usar access/fopen com paths Windows no MEMFS"
priority: P1
status: open
kind: browser-port-blocker
first_observed: 2026-10-29
verified_against: 7d76a6c77cbeb9537a6b2c79fc99d14c6f82c152
epic: e10-network-transport-v1
scope: pr-23
---

# PW23-044 — Server list não abre no MEMFS por barras invertidas

## Evidência

Novo log Chromium do operador em 29/10/2026 às 11:14:08:

```text
CECServerList::Init(), server list file error!, at line 179
```

No HEAD `7d76a6c77cbeb9537a6b2c79fc99d14c6f82c152`, `src/client153/Element/CElementClient/EC_ServerList.cpp:52-73` seleciona os caminhos `"..\\patcher\\server\\serverlist.txt"` e `"userdata\\server\\serverlist.txt"` e faz `access(path, 0)` / `fopen(path, "rb")` **diretos**. Em WASM/MEMFS POSIX a barra invertida é caractere literal, não separador de diretório. `scripts/client153-entry-server.py:46` já prevê o recurso em `userdata\\server\\serverlist.txt`, enquanto `scripts/client153-entry-browser-pre.js:44` normaliza o caminho do manifest para `/userdata/server/serverlist.txt`. Assim há um desencontro determinístico **no call site de acesso POSIX**, mesmo se o arquivo autorizado foi montado corretamente.

Ainda não há prova de que o arquivo está no manifest real do runner ou de que seu conteúdo esteja válido; verificar ambos. Essa falha não abortou `CECGame::Init` neste log, que parou em `world_targets`, mas poderá impedir escolha/conexão de servidor quando o login avançar.

## Correção proposta

1. Inspecionar `/manifest` e `FS.analyzePath('/userdata/server/serverlist.txt')`, tamanho e bytes iniciais no mesmo Chromium.
2. Normalizar `\\`→`/` na **fronteira Win32/FS** usada pelos `access/fopen` diretos (preferir helper centralizado e testar call sites) ou reescrever a seleção para caminhos de plataforma, mantendo semântica e sem mudar assets. `AFileImage` já tem normalização própria; ela **não se aplica** aos `access/fopen` dessa função.
3. Validar formato textual, encoding, grupos/hostnames/endpoints e parser real da server list. O gateway WSS/realm deve ser tratado separadamente de file I/O e não conectar diretamente um browser a TCP.
4. Executar testes nativo/WASM de `access` e `fopen` no arquivo provisionado e prova Chromium de seleção de realm a partir de dados reais.

## Aceite

- `CECServerList::Init` encontra e parseia a serverlist autorizada, com contagem verificada e logs, sem injeção de dummy data.
- Testes cobrem backslashes, barras, variação de casing e ausência de arquivo; resultado correto em WASM e implementação nativa.
- Não confundir este warning **não fatal** com primeiro bloqueio atual de [PW23-012](PW23-012-encoding-gbk-utf16-utf8-mismatch.md).
- Registrar commit, test command, SHA/runner e screenshot da UI de servidores quando funcional.

## Referências

- [Análise publicada no PR](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6499305374)
- [PR #23](https://github.com/brasalabs6/pw-web/pull/23)
- [Épico e10 de rede](../epics/e10-network-transport-v1/README.md)
