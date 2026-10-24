# Test plan e prova de funcionamento — Compatibilidade de navegadores, acessibilidade e dispositivos

## Pirâmide por workorder
- **Unit:** contratos explícitos, dados válidos/inválidos, overflow/empty/state machine/cancelamento.
- **Component:** mocks locais e comparação nativa x browser quando aplicável.
- **Integration:** sem assets comerciais e serviços externos na CI padrão; ambiente de staging isolado e recursos permitidos quando necessário.
- **E2E:** capacidades percebidas pelo usuário; teste browser e screenshot/replay quando resultado visual.
- **Security:** fuzz, ACL, canonicalização de arquivos, bounds, malformed/timeout/rate limit pertinentes.
- **Performance:** FPS/frametime, heap, VRAM, carga de rede, latência e logs por dispositivo (somente cenários que existem).

## Plano de aceitação por workorder
- **T-01:** executar teste reproduzível demonstrando `Definir navegadores alvo e matriz API/feature flags com critérios de fallback`, incluindo um caso negativo e uma observação independente. Evidência por log, trace ou screenshot autorizado; declarar blocked caso dependa de recurso indisponível.
- **T-02:** executar teste reproduzível demonstrando `Implementar capacidade/diagnóstico e mensagens específicas ao hardware`, incluindo um caso negativo e uma observação independente. Evidência por log, trace ou screenshot autorizado; declarar blocked caso dependa de recurso indisponível.
- **T-03:** executar teste reproduzível demonstrando `Implementar UX responsivo, foco, legendas, remapeamento de entradas e i18n`, incluindo um caso negativo e uma observação independente. Evidência por log, trace ou screenshot autorizado; declarar blocked caso dependa de recurso indisponível.
- **T-04:** executar teste reproduzível demonstrando `Criar testes de perda de contexto, background tab, storage eviction e touch`, incluindo um caso negativo e uma observação independente. Evidência por log, trace ou screenshot autorizado; declarar blocked caso dependa de recurso indisponível.
- **T-05:** executar teste reproduzível demonstrando `Executar campanha manual em dispositivos reais e corrigir divergências`, incluindo um caso negativo e uma observação independente. Evidência por log, trace ou screenshot autorizado; declarar blocked caso dependa de recurso indisponível.

## Benchmarks e verificação
- Registrar métrica: matriz browser/dispositivo preenchida. Publicar valor obtido e condições, não estimativa inventada.
- Registrar métrica: fluxos HUD navegáveis por teclado. Publicar valor obtido e condições, não estimativa inventada.
- Registrar métrica: sem crashes após pause/resume. Publicar valor obtido e condições, não estimativa inventada.

## Dados de teste
- Fixtures pequenas sintéticas preferidas. Dados reais só com autorização explícita; não armazenar hashes que revelem credenciais.
- Separar golden expected de saída observada, registrar SHA e ambiente.
- Fixtures malformadas para demonstrar rejeição segura.

## Evidência machine-readable
```json
{"schema_version":1,"epic":"e18-browser-ux-accessibility-v1","workorder":"wo-01","commit":"<sha>","environment":"<browser/os/gpu>","tests":[{"id":"T-01","status":"blocked","evidence":"<artifact>"}],"limitations":["<limitação>"]}
```

## Gate
Somente marcar concluído quando evidências corresponderem aos critérios de `workorders/*.json`, CI verde e revisão pertinente. `Bootstrap CI` sozinho não prova gameplay.
