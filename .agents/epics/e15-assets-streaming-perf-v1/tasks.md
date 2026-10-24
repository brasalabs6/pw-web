# Work breakdown structure — Streaming, CDN, cache OPFS e desempenho do navegador

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Definir budgets alvo por browser/dispositivo e manifesto de assets permitidos
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Implementar fetch Range/retry/sha/abort com mock HTTP server
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Implementar OPFS/IndexedDB cache versionado e LRU com cotas
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Priorizar visibilidade, pré-busca, decode em worker e limites VRAM
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Rodar perfil real com pacotes autorizados e rede lenta/perda de conexão
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Definir budgets alvo por browser/dispositivo e manifesto de assets permitidos | evidência reproduzível, testes e revisão |
| WO-02 | Implementar fetch Range/retry/sha/abort com mock HTTP server | evidência reproduzível, testes e revisão |
| WO-03 | Implementar OPFS/IndexedDB cache versionado e LRU com cotas | evidência reproduzível, testes e revisão |
| WO-04 | Priorizar visibilidade, pré-busca, decode em worker e limites VRAM | evidência reproduzível, testes e revisão |
| WO-05 | Rodar perfil real com pacotes autorizados e rede lenta/perda de conexão | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
