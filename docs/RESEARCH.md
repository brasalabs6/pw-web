# Research ledger: upstreams, evidências e hipóteses

Última verificação dos refs: 2026-10-24. Submódulos são referências **não incluídas em builds**; veja `.gitmodules`.

| Nome | Upstream | Commit fixo | Uso |
| --- | --- | --- | --- |
| client-153 | https://github.com/WolfsCore/Source-Cliente-1.5.3 | e3d6d0075d0a745c9f9859f2cb25b01d71dec045 | candidato principal, Angelica2/D3D8, solução VS2005 |
| client-152-modern | https://github.com/coussiraty/PWClient-152 | f321e68953a5502ff5673d6099dab8c193c597a0 | comparação D3D9/modernização VS2022 |
| server-rust | https://github.com/mur1ll0/pw-universal-server | 423c0b7323e3effb3420edef4545b934afd56fda | protocolo, parsers, mapas, servidor |
| server-153-source | https://github.com/hrace009/PW-Source-1.5.3 | 7dde52fa03d394f55321931e7f0d7c1b7375fdad | servidor C++ 32-bit (fonte de build e runtime); derivado comunitário, NÃO-canônico — ver `src/server153/PROVENANCE.md` e `docs/SERVER153-REPORT.md` |

## Adicionais não incorporados
- https://github.com/Zyrcon-learn/PW152_CLIENT_ANGELICA2_2 — alternativa Angelica2.2; README afirma compilar, mas relata bugs de inicialização.
- https://github.com/LostMyCode/whiplash-gunz — referência C++/WASM para jogo Direct3D com rede.
- https://github.com/LostMyCode/d3d9-webgl — referência de camada D3D9/fixed function, não drop-in completo da Angelica.
- https://github.com/vthibault/roBrowser — referência cliente MMORPG no navegador.
- https://github.com/crosire/d3d8to9 — tradução D3D8 para D3D9 em ambiente Windows; NÃO é backend pronto para browser.
- https://emscripten.org/docs/ — compilação WebAssembly e plataforma.
- https://developer.mozilla.org/en-US/docs/Web/API/WebGL2RenderingContext — render WebGL2.

## Fatos constatados pelo código
- Fonte 1.5.3 `Angelica2/Angelica3D/Header/A3DPlatform.h` inclui `Windows.h`, `d3d8.h`, `d3dx8.h`.
- Fonte moderna 1.5.2 `PWClient152/Engine/Angelica3D/Header/A3DPlatform.h` inclui `d3d9.h`, `d3dx9.h`.
- Rust server descreve versões 1.5.5 e 1.2.6 com realm; 1.5.3 não deve ser considerado compatível sem teste.
- Empacotamento PCK/PKX e protocolo GNET/gamedata documentados nas specs Rust, mas arquivos exatos dependem da versão.

## Fatos constatados pelo protocolo (2026-10-27, CreateRole Decode Error)
- O cliente PWBR 1.5.3 v145 (build 2555, 2018) envia `CreateRole` (type=84) com corpo de **1704 bytes** no formato de **23 campos** (`RoleInfo` sem `rank`), capturado pelo dump `[decode]` instrumentado em `share/io/protocol.h` (`dist/server153/runtime/logs/glinkd.log`).
- O commit upstream 8aa72d3 ("Upgrade para 1.5.3v145 com custons", out/2024) **adicionou** `Octets rank` a `RoleInfo` (24º campo) para um cliente chinês mais novo; o binário PWBR 2018 **não o envia**. O servidor lia `rank` na posição do byte de comprimento de `referid` e lançava exceção ao ler `referid` além do fim do stream → `Protocol(type=84,size=1704) Decode Error`.
- Prova de tamanho: layout de 24 campos prevê 1705 bytes; o cliente enviou 1704 (exatamente um Octets vazio a menos).
- Fonte de era correspondente (`clients/source_server_153.7z`, árvore `20141231`, `rpcalls.xml`) define `RoleInfo` com 23 campos, sem `rank`. `GRoleStatus` teve `reserved2` (char) renomeado para `rank` (Octets) pelo mesmo commit — compatível no wire enquanto vazio; mantido.
- Correção: remoção de `rank` de `RoleInfo` + 3 sítios de referência (commit 0c445e35); teste de conformância de wire `tests/server153_createrole_wire.cpp` (via `scripts/test-server153-wire.sh`, integrado ao `verify.sh`) reproduz o pacote do cliente byte a byte (1704 bytes, prefixo de 64 bytes idêntico à captura) e prova o unmarshal limpo.
- Os commits pós-7dde52f do hrace009 (d35f7de, cc74d74, 1b7c9fd, c283446) contêm **apenas** mudanças de Makefiles (modernização GCC17/x64) — nenhuma mudança de protocolo; o "upgrade v145" relevante (8aa72d3) já é ancestral da nossa revisão pinada.
- `clients/Source Cliente.rar` (593 MB) contém a source **completa** do cliente (`Element/CElementClient/`, incluindo `Network/` GNET + código de criação de personagem `EC_LoginUIMan.cpp`) — mais completa que `references/client-153` (só engine). Arquivos majoritariamente de 2015 com modificações de 2024 (alinhadas ao servidor v145-upgraded); **não** representa o estado exato do binário PWBR 2018 — usar como referência de protocolo com cautela, validando contra capturas de wire.
- Login chain completo (challenge → response → matrixpasswd2 retcode=0 → keyexchange → userlogin2 retcode=0 → onlineannounce → ChangeState GRoleList) confirmado por traces; a criação de personagem era o único ponto de falha.

## Hipóteses A TESTAR
H1: 1.5.3 pode ser construída integralmente com toolchain adequado.
H2: subsistemas de Angelica2 são portáveis sem carregar `Windows.h` diretamente quando desacoplados.
H3: um adaptador D3D8→backend WebGL2 para o subconjunto realmente utilizado terá melhor manutenção do que encadear wrappers.
H4: cliente 1.5.3 pode se conectar a um realm escolhido após adaptação de wire format.
H5: streaming de assets e OPFS evitam download inicial do jogo inteiro.

## Procedimento de atualização
Atualizações de upstream são PRs específicos com SHA antigo/novo, diff relevante, impacto de licença e novos testes. NÃO executar `git submodule update --remote` em automações.
