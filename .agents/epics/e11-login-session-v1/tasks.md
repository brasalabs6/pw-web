# Work breakdown structure — Identidade, login, personagem e sessão

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Especificar sequência login/realm/lista/personagem/mundo e estados de erro
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Implementar máquina de estados com mocks e sanitização de segredos
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Criar frontend login/seleção com estados loading/error/logout e cancelamento
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Integrar endpoint auth real autorizado com TLS e validade de sessão
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Testar múltiplas sessões, expirados, rate limiting e entrada no mundo
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Especificar sequência login/realm/lista/personagem/mundo e estados de erro | evidência reproduzível, testes e revisão |
| WO-02 | Implementar máquina de estados com mocks e sanitização de segredos | evidência reproduzível, testes e revisão |
| WO-03 | Criar frontend login/seleção com estados loading/error/logout e cancelamento | evidência reproduzível, testes e revisão |
| WO-04 | Integrar endpoint auth real autorizado com TLS e validade de sessão | evidência reproduzível, testes e revisão |
| WO-05 | Testar múltiplas sessões, expirados, rate limiting e entrada no mundo | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
