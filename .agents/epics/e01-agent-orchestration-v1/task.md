# Task master — e01-agent-orchestration-v1

Resumo operacional (detalhamento completo em [tasks.md](tasks.md)). A unidade de execução automatizável é uma workorder, não o épico inteiro.

- [ ] [WO-01](workorders/wo-01.md): Definir máquina de estados, schemas, escopos, dependências e critérios de evidência
- [ ] [WO-02](workorders/wo-02.md): Implementar execução manual isolada, secret boundary e limites de patch
- [ ] [WO-03](workorders/wo-03.md): Implementar controlador CI/feedback, limites de retries e parada segura
- [ ] [WO-04](workorders/wo-04.md): Adicionar fila determinística programada, anti-duplicação e orçamento/concurrency
- [ ] [WO-05](workorders/wo-05.md): Testar incidentes, rollback, falha de API, injection e auditoria dos agentes

**Regras:** validar dependências, código de terceiros/direitos e critérios de aceite antes de despachar. Configuração/execução dos agentes em `docs/AUTONOMY.md`. Revisão humana obrigatória antes de merge.
