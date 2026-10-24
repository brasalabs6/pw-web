# Task master — e17-security-deployment-v1

Resumo operacional (detalhamento completo em [tasks.md](tasks.md)). A unidade de execução automatizável é uma workorder, não o épico inteiro.

- [ ] [WO-01](workorders/wo-01.md): Desenhar topologia por ambiente e segurança das portas/rede
- [ ] [WO-02](workorders/wo-02.md): Criar manifests IaC para sandbox sem produção e secrets externos
- [ ] [WO-03](workorders/wo-03.md): Implementar TLS/CSP/CORS/cabeçalhos e limites de ataque do gateway mock
- [ ] [WO-04](workorders/wo-04.md): Definir release pipeline assinado com stage e aprovação jurídica
- [ ] [WO-05](workorders/wo-05.md): Testar incident response, backup/restore, rollback e deploy controlado

**Regras:** validar dependências, código de terceiros/direitos e critérios de aceite antes de despachar. Configuração/execução dos agentes em `docs/AUTONOMY.md`. Revisão humana obrigatória antes de merge.
