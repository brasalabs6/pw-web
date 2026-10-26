# Árvore do cliente 1.5.3 portado

**Reservada; ainda não contém o cliente portado.** A implementação ocorre em
incrementos a partir da source fixada em `references/client-153`, SHA
`e3d6d0075d0a745c9f9859f2cb25b01d71dec045`. Não editar a referência congelada.

Estrutura a materializar conforme cada closure for aprovada:

```text
src/client153/
  Angelica2/AngelicaCommon/…   # arquivos originais necessários, adaptados
  Angelica2/Angelica3D/…
  Angelica2/AngelicaFile/…
  Element/CElementClient/…
  source-manifest.json       # origem/hash por arquivo importado; a implementar
  CMakeLists.txt             # targets/fontes explícitos; a implementar
```

Preservar copyrights, caminhos relativos e semântica; cada import tem origem,
hash upstream e decisão de autorização. Não importar assets, binários, SDKs,
credenciais, configurações de servidores ou toda a source por conveniência.
Arquivos novos de plataforma pertencem a `engine/platform/`, contratos novos a
`engine/`; não fabricar Windows.h/D3D no-op. Não manter duas cópias compiladas do
mesmo símbolo. Native/wasm targets escolhem as mesmas unidades portáteis.

A lista de fontes permitidas é explícita no CMake; novas dependências bloqueiam o
build até serem classificadas. Um arquivo não rastreado ou uma biblioteca local
pré-compilada não pode tornar uma build verde. Regras completas:
[build-pipeline](../../.agents/epics/e17-security-deployment-v1/build-pipeline.md)
e [ADR-0005](../../docs/DECISIONS/ADR-0005-cloudflare-delivery.md).
