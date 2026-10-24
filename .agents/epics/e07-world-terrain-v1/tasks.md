# Work breakdown structure — Terreno, cidades, vegetação, água e visibilidade do mundo

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Definir matrizes de coordenadas, units/axis e modelo de chunks/scenegraph
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Implementar heightfield e malha procedural com LOD e stitching
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Implementar quadtree/frustum culling e carregamento de objetos estáticos
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Adicionar estados para água, vegetação, skybox, iluminação e clima
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Comparar cenário autorizado e testar teleporte/streaming sem corruptelas
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Definir matrizes de coordenadas, units/axis e modelo de chunks/scenegraph | evidência reproduzível, testes e revisão |
| WO-02 | Implementar heightfield e malha procedural com LOD e stitching | evidência reproduzível, testes e revisão |
| WO-03 | Implementar quadtree/frustum culling e carregamento de objetos estáticos | evidência reproduzível, testes e revisão |
| WO-04 | Adicionar estados para água, vegetação, skybox, iluminação e clima | evidência reproduzível, testes e revisão |
| WO-05 | Comparar cenário autorizado e testar teleporte/streaming sem corruptelas | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
