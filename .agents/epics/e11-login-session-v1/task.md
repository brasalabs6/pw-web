# Task master — e11-login-session-v1

Resumo operacional (detalhamento completo em [tasks.md](tasks.md)). A unidade de execução automatizável é uma workorder, não o épico inteiro.

- [ ] [WO-01](workorders/wo-01.md): Especificar sequência login/realm/lista/personagem/mundo e estados de erro
- [ ] [WO-02](workorders/wo-02.md): Implementar máquina de estados com mocks e sanitização de segredos
- [ ] [WO-03](workorders/wo-03.md): Criar frontend login/seleção com estados loading/error/logout e cancelamento
- [ ] [WO-04](workorders/wo-04.md): Integrar endpoint auth real autorizado com TLS e validade de sessão
- [ ] [WO-05](workorders/wo-05.md): Testar múltiplas sessões, expirados, rate limiting e entrada no mundo

**Regras:** validar dependências, código de terceiros/direitos e critérios de aceite antes de despachar. Configuração/execução dos agentes em `docs/AUTONOMY.md`. Revisão humana obrigatória antes de merge.
