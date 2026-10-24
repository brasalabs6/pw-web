# Work breakdown structure — Renderer WebGL2 da engine Angelica

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Inventariar render states realmente usados na Angelica2 e dependências D3D8
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Definir tabelas de equivalência de vertex formats e estados blend/cull/depth
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Implementar backend WebGL2 incremental com shaders do subconjunto utilizado
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Integrar materiais, texturas, render targets, transparência e context restoration
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Validar paridade visual com screenshot Windows e GPU capture autorizados
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Inventariar render states realmente usados na Angelica2 e dependências D3D8 | evidência reproduzível, testes e revisão |
| WO-02 | Definir tabelas de equivalência de vertex formats e estados blend/cull/depth | evidência reproduzível, testes e revisão |
| WO-03 | Implementar backend WebGL2 incremental com shaders do subconjunto utilizado | evidência reproduzível, testes e revisão |
| WO-04 | Integrar materiais, texturas, render targets, transparência e context restoration | evidência reproduzível, testes e revisão |
| WO-05 | Validar paridade visual com screenshot Windows e GPU capture autorizados | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
