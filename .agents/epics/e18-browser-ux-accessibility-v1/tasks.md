# Work breakdown structure — Compatibilidade de navegadores, acessibilidade e dispositivos

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Definir navegadores alvo e matriz API/feature flags com critérios de fallback
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Implementar capacidade/diagnóstico e mensagens específicas ao hardware
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Implementar UX responsivo, foco, legendas, remapeamento de entradas e i18n
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Criar testes de perda de contexto, background tab, storage eviction e touch
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Executar campanha manual em dispositivos reais e corrigir divergências
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Definir navegadores alvo e matriz API/feature flags com critérios de fallback | evidência reproduzível, testes e revisão |
| WO-02 | Implementar capacidade/diagnóstico e mensagens específicas ao hardware | evidência reproduzível, testes e revisão |
| WO-03 | Implementar UX responsivo, foco, legendas, remapeamento de entradas e i18n | evidência reproduzível, testes e revisão |
| WO-04 | Criar testes de perda de contexto, background tab, storage eviction e touch | evidência reproduzível, testes e revisão |
| WO-05 | Executar campanha manual em dispositivos reais e corrigir divergências | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
