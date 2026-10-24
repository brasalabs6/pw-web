# Work breakdown structure — Estratégia de testes, tracing, replay e QA automatizado

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Criar matriz de testes por camada, severidade e definição de done
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Automatizar fixtures sintéticas, fuzz, ASAN/UBSAN e testes de ABI
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Criar harness browser E2E WebGL/WASM/viewport e screenshot diffs
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Criar evidência JSON/artifacts, traces e perf budgets versionados
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Executar campanhas de compatibilidade e confrontar builds Windows autorizados
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Criar matriz de testes por camada, severidade e definição de done | evidência reproduzível, testes e revisão |
| WO-02 | Automatizar fixtures sintéticas, fuzz, ASAN/UBSAN e testes de ABI | evidência reproduzível, testes e revisão |
| WO-03 | Criar harness browser E2E WebGL/WASM/viewport e screenshot diffs | evidência reproduzível, testes e revisão |
| WO-04 | Criar evidência JSON/artifacts, traces e perf budgets versionados | evidência reproduzível, testes e revisão |
| WO-05 | Executar campanhas de compatibilidade e confrontar builds Windows autorizados | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
