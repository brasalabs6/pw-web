# Task master — e15-assets-streaming-perf-v1

Resumo operacional (detalhamento completo em [tasks.md](tasks.md)). A unidade de execução automatizável é uma workorder, não o épico inteiro.

- [ ] [WO-01](workorders/wo-01.md): Definir budgets alvo por browser/dispositivo e manifesto de assets permitidos
- [ ] [WO-02](workorders/wo-02.md): Implementar fetch Range/retry/sha/abort com mock HTTP server
- [ ] [WO-03](workorders/wo-03.md): Implementar OPFS/IndexedDB cache versionado e LRU com cotas
- [ ] [WO-04](workorders/wo-04.md): Priorizar visibilidade, pré-busca, decode em worker e limites VRAM
- [ ] [WO-05](workorders/wo-05.md): Rodar perfil real com pacotes autorizados e rede lenta/perda de conexão

**Regras:** validar dependências, código de terceiros/direitos e critérios de aceite antes de despachar. Configuração/execução dos agentes em `docs/AUTONOMY.md`. Revisão humana obrigatória antes de merge.
