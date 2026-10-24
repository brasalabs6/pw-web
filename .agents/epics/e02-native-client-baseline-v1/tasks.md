# Work breakdown structure — Baseline do cliente Windows e seleção validada da Angelica

> Tasks não concluídas até serem executadas com evidência. Cada checkbox está subordinada a um workorder JSON.

- [ ] **WO-01** — Construir inventário estático de solutions, arquivos e bibliotecas da fonte 1.5.3/1.5.2
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-02** — Criar matriz de compatibilidade D3D8/9, headers, ABI, asset e protocolo por versão
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-03** — Compilar cliente e engine em ambiente Windows autorizado com logs reproduzíveis
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-04** — Verificar inicialização, carregamento da cena, handshake e comportamento visual
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite
- [ ] **WO-05** — Registrar ADR de seleção final, fallback e bloqueios por componente
  - [ ] Confirmar precondições/direitos e versão-alvo
  - [ ] Implementar contratos, testes e observabilidade relacionados
  - [ ] Registrar comandos, fixtures válidas, logs e limites da prova
  - [ ] Enviar PR isolado; revisão e gate de aceite

## Matriz de liberação
| Workorder | Escopo verificável | Aceite |
| --- | --- | --- |
| WO-01 | Construir inventário estático de solutions, arquivos e bibliotecas da fonte 1.5.3/1.5.2 | evidência reproduzível, testes e revisão |
| WO-02 | Criar matriz de compatibilidade D3D8/9, headers, ABI, asset e protocolo por versão | evidência reproduzível, testes e revisão |
| WO-03 | Compilar cliente e engine em ambiente Windows autorizado com logs reproduzíveis | evidência reproduzível, testes e revisão |
| WO-04 | Verificar inicialização, carregamento da cena, handshake e comportamento visual | evidência reproduzível, testes e revisão |
| WO-05 | Registrar ADR de seleção final, fallback e bloqueios por componente | evidência reproduzível, testes e revisão |

## Checklist final do épico
- [ ] Todos os 5 workorders aprovados no GitHub com SHA/artefatos e notas de revisão.
- [ ] Interface pública versionada e documentada.
- [ ] Testes relevantes reproduzidos na CI e, quando exigido, E2E/manual autorizado.
- [ ] Dívida técnica, versões e limitações explicitamente rastreadas.
- [ ] Direitos/proveniência adequados à forma de uso.
- [ ] Integração com próximos épicos liberada por decisão registrada, não por suposição.
