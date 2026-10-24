# Test plan e prova de funcionamento — Renderer WebGL2 da engine Angelica

## Pirâmide por workorder
- **Unit:** contratos explícitos, dados válidos/inválidos, overflow/empty/state machine/cancelamento.
- **Component:** mocks locais e comparação nativa x browser quando aplicável.
- **Integration:** sem assets comerciais e serviços externos na CI padrão; ambiente de staging isolado e recursos permitidos quando necessário.
- **E2E:** capacidades percebidas pelo usuário; teste browser e screenshot/replay quando resultado visual.
- **Security:** fuzz, ACL, canonicalização de arquivos, bounds, malformed/timeout/rate limit pertinentes.
- **Performance:** FPS/frametime, heap, VRAM, carga de rede, latência e logs por dispositivo (somente cenários que existem).

## Plano de aceitação por workorder
- **T-01:** executar teste reproduzível demonstrando `Inventariar render states realmente usados na Angelica2 e dependências D3D8`, incluindo um caso negativo e uma observação independente. Evidência por log, trace ou screenshot autorizado; declarar blocked caso dependa de recurso indisponível.
- **T-02:** executar teste reproduzível demonstrando `Definir tabelas de equivalência de vertex formats e estados blend/cull/depth`, incluindo um caso negativo e uma observação independente. Evidência por log, trace ou screenshot autorizado; declarar blocked caso dependa de recurso indisponível.
- **T-03:** executar teste reproduzível demonstrando `Implementar backend WebGL2 incremental com shaders do subconjunto utilizado`, incluindo um caso negativo e uma observação independente. Evidência por log, trace ou screenshot autorizado; declarar blocked caso dependa de recurso indisponível.
- **T-04:** executar teste reproduzível demonstrando `Integrar materiais, texturas, render targets, transparência e context restoration`, incluindo um caso negativo e uma observação independente. Evidência por log, trace ou screenshot autorizado; declarar blocked caso dependa de recurso indisponível.
- **T-05:** executar teste reproduzível demonstrando `Validar paridade visual com screenshot Windows e GPU capture autorizados`, incluindo um caso negativo e uma observação independente. Evidência por log, trace ou screenshot autorizado; declarar blocked caso dependa de recurso indisponível.

## Benchmarks e verificação
- Registrar métrica: golden images com tolerância registrada. Publicar valor obtido e condições, não estimativa inventada.
- Registrar métrica: frametime e draw calls. Publicar valor obtido e condições, não estimativa inventada.
- Registrar métrica: recursos GPU liberados sem vazamento. Publicar valor obtido e condições, não estimativa inventada.

## Dados de teste
- Fixtures pequenas sintéticas preferidas. Dados reais só com autorização explícita; não armazenar hashes que revelem credenciais.
- Separar golden expected de saída observada, registrar SHA e ambiente.
- Fixtures malformadas para demonstrar rejeição segura.

## Evidência machine-readable
```json
{"schema_version":1,"epic":"e06-render-webgl2-v1","workorder":"wo-01","commit":"<sha>","environment":"<browser/os/gpu>","tests":[{"id":"T-01","status":"blocked","evidence":"<artifact>"}],"limitations":["<limitação>"]}
```

## Gate
Somente marcar concluído quando evidências corresponderem aos critérios de `workorders/*.json`, CI verde e revisão pertinente. `Bootstrap CI` sozinho não prova gameplay.
