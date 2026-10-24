# Task master — e19-liveops-evolution-v1

Resumo operacional (detalhamento completo em [tasks.md](tasks.md)). A unidade de execução automatizável é uma workorder, não o épico inteiro.

- [ ] [WO-01](workorders/wo-01.md): Especificar versão semântica cliente/asset/protocolo/realm e migração segura
- [ ] [WO-02](workorders/wo-02.md): Criar catálogo de incidentes, suporte e telemetria minimizada
- [ ] [WO-03](workorders/wo-03.md): Implementar flags de rollout e checagem de compatibilidade em ambiente local
- [ ] [WO-04](workorders/wo-04.md): Projetar manutenção programada, backup, rollback e escalabilidade medida
- [ ] [WO-05](workorders/wo-05.md): Auditar incidentes e autorizar transição para beta/produção com checklist

**Regras:** validar dependências, código de terceiros/direitos e critérios de aceite antes de despachar. Configuração/execução dos agentes em `docs/AUTONOMY.md`. Revisão humana obrigatória antes de merge.
