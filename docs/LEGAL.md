# Gate jurídico/IP de dependências e assets

**Isto é uma política de engenharia, não parecer jurídico.** O registro abaixo
documenta a decisão declarada pelo operador; o contrato em si é mantido pelo
operador fora deste repositório.

## Decisão documentada — 2026-10-25

Declaração do operador: o projeto foi contratado pela companhia mãe (detentora
dos direitos de Perfect World) para realizar o port do cliente para o navegador.
Autorização plena para compilar, baixar, usar, modificar e integrar o código do
cliente (Angelica), assets do jogo, servidor e materiais relacionados no escopo
do port. A distribuição segue o escopo do contrato mantido pelo operador.

## Classificação
- Código do Perfect World/Angelica publicado por terceiros: **autorizado para o port** conforme a decisão acima. Avisos de copyright de Archosaur permanecem nas fontes e não devem ser removidos.
- Assets, personagens, músicas, interface, mapas, marcas e textos originais: **autorizados para uso no port** conforme a decisão acima.
- Servidor Rust publicado sob licença MIT pelo respectivo autor: verificar quais componentes são originais e eventuais materiais/fixtures dependentes de terceiros.
- Código novo do `pw-web`: titularidade da organização, com licenciamento decidido separadamente.

## Gate de entrada
Para cada dependência/asset integrar: registrar titular presumido, origem URL, commit/hash, licença exibida, cadeia de autorização (decisão de 2026-10-25), necessidade de reprodução/modificação/redistribuição, decisão, aprovador e data.

## Política de distribuição
- `references/` mantém gitlinks para upstreams; cópias para `engine/`, `web/` ou a branch principal passam a ser permitidas no escopo da decisão acima, com registro de proveniência.
- Clonagem dos submódulos é opt-in, não ocorre na CI.
- Builds genéricos são permitidos sem material do jogo; builds com material autorizado são permitidos no escopo do port.
- Releases/container/CDN podem carregar código/arte/áudio/cliente no escopo do contrato; fora dele, exigem nova autorização.
- Nunca eliminar headers de copyright/attribution.
- Não expor credenciais, IPs internos, tokens nem dados pessoais do cliente original.

## Rota alternativa
Se permissões forem negadas/incertas para produto público, adotar cliente/engine de implementação própria, conteúdo com licença apropriada e eventual interoperabilidade independente, sujeita à revisão jurídica.
