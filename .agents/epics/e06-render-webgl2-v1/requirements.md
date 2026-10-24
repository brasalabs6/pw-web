# Requisitos — Renderer WebGL2 da engine Angelica

Cada requisito abaixo tem uma workorder proprietária. Resultado mensurável prevalece
sobre presença de arquivos. Schema ou ambiente ausente mantém a tarefa bloqueada.

- **RF-01 / WO-01:** Inventário estabelece subconjunto necessário e identifica toda dependência que impede o primeiro draw.
- **RF-02 / WO-02:** Nenhuma chamada D3D usada pela fatia fica sem semântica, erro ou adaptação explícita.
- **RF-03 / WO-03:** Fixtures R0 6/7 desenham cores/profundidade esperadas; pixel central e canto são conferidos por readPixels.
- **RF-04 / WO-04:** Cena com opaco, alpha e offscreen produz pixels previstos e mantém contagem estável de recursos após reload.
- **RF-05 / WO-05:** Relatório demonstra paridade da primeira fatia Angelica, com referência Windows e limites verificáveis.
- **RF-06 / WO-06:** Caller sabe exatamente o que o backend suporta e como a ausência é reportada.
- **RF-07 / WO-07:** R1 fecha todo dado necessário a um upload e draw texturizado, com resultado de pixels previsto.
- **RF-08 / WO-08:** Quadrantes têm cores esperadas e bytes inválidos não alteram uma textura válida existente.
- **RF-09 / WO-09:** Estados mapeados reproduzem a semântica aprovada sem vazamento entre draws.
- **RF-10 / WO-10:** WO-12 recebe contrato fechado com estado anterior preservado em falha de criação/resize.
- **RF-11 / WO-11:** Cada combinação suportada tem fixture de imagem e chave única; não há programa reutilizado incorretamente.
- **RF-12 / WO-12:** Offscreen e readback retornam dimensões/cores esperadas após resize válido.
- **RF-13 / WO-13:** Closure tem fronteira auditável e não depende de suprimir funcionalidades usadas para compilar.
- **RF-14 / WO-14:** Um draw executado pelo caminho original adaptado coincide com os valores/pixels da baseline.
- **RF-15 / WO-15:** Draw em Lost retorna ContextLost; após recriação válida o pixel de referência volta a ser desenhado.
- **RF-16 / WO-16:** Benchmark informa condições, amostras e métricas reais; cenários de paridade permanecem verdes.

## Requisitos transversais

- Sem fontes/assets proprietários no build público sem decisão documentada.
- Mesmo caso reproduzível no ambiente declarado; todos os testes obrigatórios contam.
- Input inválido, lifetime e limites tratados antes de efeitos irreversíveis.
- Comandos planejados não são evidência; saída real com versão/SHA é obrigatória.
