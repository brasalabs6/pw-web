# Work breakdown structure — Pipeline de recursos PCK/PKX, formatos e proveniência

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Catalogar magic/versionamento e limites de PCK/PKX e famílias de arquivos
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Especificar parsing streaming e GBK→Unicode sem confiar em nomes de caminho
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Implementar parsers defensivos em fixtures sintéticas e fuzzer de entrada
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Gerar manifestos versionados com hashes, tamanhos, offsets e dependências
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Validar casos reais apenas com assets autorizados e decisão de distribuição
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Catalogar magic/versionamento e limites de PCK/PKX e famílias de arquivos | evidência reproduzível, testes e revisão |
| WO-02 | Especificar parsing streaming e GBK→Unicode sem confiar em nomes de caminho | evidência reproduzível, testes e revisão |
| WO-03 | Implementar parsers defensivos em fixtures sintéticas e fuzzer de entrada | evidência reproduzível, testes e revisão |
| WO-04 | Gerar manifestos versionados com hashes, tamanhos, offsets e dependências | evidência reproduzível, testes e revisão |
| WO-05 | Validar casos reais apenas com assets autorizados e decisão de distribuição | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
