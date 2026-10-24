# Work breakdown structure — Direitos, proveniência, licenciamento e decisões de produto

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Catalogar arquivos, bibliotecas, símbolos e conteúdos protegidos por origem e versão
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Validar direitos de reprodução/modificação/distribuição e termos comerciais por titular
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Desenhar estratégia de implementação independente para componentes sem autorização
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Construir SBOM, política de attribution e gates de PR/build/release
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Aprovar matriz de distribuição, marcas, privacy e revisão periódica
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Catalogar arquivos, bibliotecas, símbolos e conteúdos protegidos por origem e versão | evidência reproduzível, testes e revisão |
| WO-02 | Validar direitos de reprodução/modificação/distribuição e termos comerciais por titular | evidência reproduzível, testes e revisão |
| WO-03 | Desenhar estratégia de implementação independente para componentes sem autorização | evidência reproduzível, testes e revisão |
| WO-04 | Construir SBOM, política de attribution e gates de PR/build/release | evidência reproduzível, testes e revisão |
| WO-05 | Aprovar matriz de distribuição, marcas, privacy e revisão periódica | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
