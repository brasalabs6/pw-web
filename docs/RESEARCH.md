# Research ledger: upstreams, evidências e hipóteses

Última verificação dos refs: 2026-10-24. Submódulos são referências **não incluídas em builds**; veja `.gitmodules`.

| Nome | Upstream | Commit fixo | Uso |
| --- | --- | --- | --- |
| client-153 | https://github.com/WolfsCore/Source-Cliente-1.5.3 | e3d6d0075d0a745c9f9859f2cb25b01d71dec045 | candidato principal, Angelica2/D3D8, solução VS2005 |
| client-152-modern | https://github.com/coussiraty/PWClient-152 | f321e68953a5502ff5673d6099dab8c193c597a0 | comparação D3D9/modernização VS2022 |
| server-rust | https://github.com/mur1ll0/pw-universal-server | 423c0b7323e3effb3420edef4545b934afd56fda | protocolo, parsers, mapas, servidor |

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

## Hipóteses A TESTAR
H1: 1.5.3 pode ser construída integralmente com toolchain adequado.
H2: subsistemas de Angelica2 são portáveis sem carregar `Windows.h` diretamente quando desacoplados.
H3: um adaptador D3D8→backend WebGL2 para o subconjunto realmente utilizado terá melhor manutenção do que encadear wrappers.
H4: cliente 1.5.3 pode se conectar a um realm escolhido após adaptação de wire format.
H5: streaming de assets e OPFS evitam download inicial do jogo inteiro.

## Experimento POC-001 — 2026-10-24

[Relatório reproduzível](POC-ANGELICA.md): ARandomGen do commit 1.5.3 fixado foi
compilado diretamente da referência, sem alteração, com GCC 16.2.1 e Emscripten
6.0.13. No Chrome 152, 4.096 uniformes coincidiram exatamente e 256 gaussianos
ficaram dentro da tolerância relativa/absoluta definida. A saída WASM alimentou
uma malha procedural de 1.922 triângulos num renderer WebGL2 próprio do teste.

H2 tem evidência positiva **somente para esse componente**. H1, H3, H4 e H5
continuam abertas. A compilação direta de A3DEngine para WASM falhou em Windows.h.
DWORD/unsigned long tem 8 bytes no Linux nativo e 4 no WASM; layouts binários
precisam de validação independente. Nenhuma referência foi atualizada.

Direção de adaptação e contratos iniciais: [ADR-0004](DECISIONS/ADR-0004-incremental-port-method.md)
e [método para executores](ANGELICA-PORTING-METHOD.md).

## Procedimento de atualização
Atualizações de upstream são PRs específicos com SHA antigo/novo, diff relevante, impacto de licença e novos testes. NÃO executar `git submodule update --remote` em automações.
