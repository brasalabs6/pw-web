# Task master — e05-asset-pipeline-v1

Resumo operacional (detalhamento completo em [tasks.md](tasks.md)). A unidade de execução automatizável é uma workorder, não o épico inteiro.

- [ ] [WO-01](workorders/wo-01.md): Catalogar magic/versionamento e limites de PCK/PKX e famílias de arquivos
- [ ] [WO-02](workorders/wo-02.md): Especificar parsing streaming e GBK→Unicode sem confiar em nomes de caminho
- [ ] [WO-03](workorders/wo-03.md): Implementar parsers defensivos em fixtures sintéticas e fuzzer de entrada
- [ ] [WO-04](workorders/wo-04.md): Gerar manifestos versionados com hashes, tamanhos, offsets e dependências
- [ ] [WO-05](workorders/wo-05.md): Validar casos reais apenas com assets autorizados e decisão de distribuição

**Regras:** validar dependências, código de terceiros/direitos e critérios de aceite antes de despachar. Configuração/execução dos agentes em `docs/AUTONOMY.md`. Revisão humana obrigatória antes de merge.
