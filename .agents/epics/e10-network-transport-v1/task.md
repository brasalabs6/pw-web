# Task master — e10-network-transport-v1

Resumo operacional (detalhamento completo em [tasks.md](tasks.md)). A unidade de execução automatizável é uma workorder, não o épico inteiro.

- [ ] [WO-01](workorders/wo-01.md): Registrar opcode/length/endian por versão e declarar fonte de evidência
- [ ] [WO-02](workorders/wo-02.md): Implementar leitor/escritor bounds-checked e protocolo mock em memória
- [ ] [WO-03](workorders/wo-03.md): Implementar WSS gateway de testes com limites, origem, sessão e backpressure
- [ ] [WO-04](workorders/wo-04.md): Integrar autenticação e transporte do servidor alvo em ambiente local isolado
- [ ] [WO-05](workorders/wo-05.md): Validar captures autorizadas, reconexão, reorder, timeout e semântica GNET

**Regras:** validar dependências, código de terceiros/direitos e critérios de aceite antes de despachar. Configuração/execução dos agentes em `docs/AUTONOMY.md`. Revisão humana obrigatória antes de merge.
