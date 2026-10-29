# Cliente original: bloqueio de janela no bootstrap

Em 2026-10-28, um probe local do módulo de entrada do cliente original no
Chromium, com os pacotes reais e os três arquivos `font/FZL2JW.ttf`,
`font/FZLBJW.ttf` e `font/FZXH1JW.ttf` mapeados em leitura, avançou além
de `CheckImportantFiles`. O módulo retornou `-1`; o `EC.log` registrou
`wWinMain: Failed to call function. (line: 879)`. A linha corresponde à
falha de `_RegisterWndClass` no estado da source testada. O probe utilizou
artefatos locais e mapeamento experimental de fontes: não é uma prova de
build limpo, tela de login ou jogo.

O contrato atual de `platform/common/Windows.h` retorna falha em
`RegisterClassEx` e `CreateWindowEx`; `PeekMessage` não entrega mensagens.
O loop original em `ElementClient.cpp` chama `PeekMessage` e
`RunInMainThread` em um `while` contínuo. Portanto, desbloquear o registro
isoladamente não fornece janela funcional nem ciclo cooperativo do browser.
O próximo trabalho deve ligar a janela lógica ao canvas real, preservar o
tratamento dos eventos necessários e adaptar o loop ao scheduler do browser,
com testes de lifecycle e entrada e nova execução do cliente original.

O log `Load GT failure` também foi observado antes da falha de registro;
este teste não determinou se ele impede a inicialização posterior.
