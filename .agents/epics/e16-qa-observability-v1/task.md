# Task master — e16-qa-observability-v1

Resumo operacional (detalhamento completo em [tasks.md](tasks.md)). A unidade de execução automatizável é uma workorder, não o épico inteiro.

- [ ] [WO-01](workorders/wo-01.md): Criar matriz de testes por camada, severidade e definição de done
- [ ] [WO-02](workorders/wo-02.md): Automatizar fixtures sintéticas, fuzz, ASAN/UBSAN e testes de ABI
- [ ] [WO-03](workorders/wo-03.md): Criar harness browser E2E WebGL/WASM/viewport e screenshot diffs
- [ ] [WO-04](workorders/wo-04.md): Criar evidência JSON/artifacts, traces e perf budgets versionados
- [ ] [WO-05](workorders/wo-05.md): Executar campanhas de compatibilidade e confrontar builds Windows autorizados

**Regras:** validar dependências, código de terceiros/direitos e critérios de aceite antes de despachar. Configuração/execução dos agentes em `docs/AUTONOMY.md`. Revisão humana obrigatória antes de merge.
