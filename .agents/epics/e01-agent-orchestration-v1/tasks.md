# Work breakdown structure — Orquestração autônoma, isolamento e governança de agentes

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Definir máquina de estados, schemas, escopos, dependências e critérios de evidência
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Implementar execução manual isolada, secret boundary e limites de patch
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Implementar controlador CI/feedback, limites de retries e parada segura
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Adicionar fila determinística programada, anti-duplicação e orçamento/concurrency
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Testar incidentes, rollback, falha de API, injection e auditoria dos agentes
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Definir máquina de estados, schemas, escopos, dependências e critérios de evidência | evidência reproduzível, testes e revisão |
| WO-02 | Implementar execução manual isolada, secret boundary e limites de patch | evidência reproduzível, testes e revisão |
| WO-03 | Implementar controlador CI/feedback, limites de retries e parada segura | evidência reproduzível, testes e revisão |
| WO-04 | Adicionar fila determinística programada, anti-duplicação e orçamento/concurrency | evidência reproduzível, testes e revisão |
| WO-05 | Testar incidentes, rollback, falha de API, injection e auditoria dos agentes | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
