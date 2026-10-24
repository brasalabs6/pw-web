# Work breakdown structure — UI do jogo, HUD, entrada, câmera e áudio

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Documentar inventário de telas, fluxos UI e inputs com baseline autorizado
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Implementar mapeamento de ações e câmera sem prender foco do browser
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Criar shell responsivo HUD/inventário/chat/configurações com dados mockados
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Integrar Web Audio lifecycle/volume/spatialization e suspensão por aba
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Validar interação e acessibilidade, toque, fullscreen e fluxos reais do cliente
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Documentar inventário de telas, fluxos UI e inputs com baseline autorizado | evidência reproduzível, testes e revisão |
| WO-02 | Implementar mapeamento de ações e câmera sem prender foco do browser | evidência reproduzível, testes e revisão |
| WO-03 | Criar shell responsivo HUD/inventário/chat/configurações com dados mockados | evidência reproduzível, testes e revisão |
| WO-04 | Integrar Web Audio lifecycle/volume/spatialization e suspensão por aba | evidência reproduzível, testes e revisão |
| WO-05 | Validar interação e acessibilidade, toque, fullscreen e fluxos reais do cliente | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
