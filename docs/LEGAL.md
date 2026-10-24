# Gate jurídico/IP de dependências e assets

**Isto é uma política de engenharia, não parecer jurídico.** Não presumir direito de distribuição porque um repositório pode ser clonado.

## Classificação
- Código do Perfect World/Angelica publicado por terceiros: **direitos e autorização de derivados não demonstrados**. Avisos de copyright de Archosaur estão presentes nas fontes. Licenças colocadas por terceiros NÃO comprovam cadeia de titularidade.
- Assets, personagens, músicas, interface, mapas, marcas e textos originais: **não estão incluídos**; uso/distribuição requer autorização pertinente.
- Servidor Rust publicado sob licença MIT pelo respectivo autor: verificar quais componentes são originais e eventuais materiais/fixtures dependentes de terceiros.
- Código novo do `pw-web` deverá ter titularidade/documentação de origem clara, com licenciamento decidido pela organização separadamente.

## Gate de entrada
Para cada dependência/asset registrar: titular presumido, origem URL, commit/hash, licença exibida, cadeia de autorização (se houver), necessidade de reprodução/modificação/redistribuição, decisão, aprovador e data.

## Política de distribuição
- `references/` contém **apenas gitlinks** para upstreams, sem cópias dentro da árvore principal.
- Clonagem dos submódulos é opt-in, não ocorre na CI.
- Builds genéricos são permitidos sem material do jogo.
- Nenhum release/container/CDN deve carregar código/arte/áudio/cliente original sem autorização.
- Nunca eliminar headers de copyright/attribution.
- Não abrir PR que copie conteúdos não autorizados do upstream para o projeto principal.

## Rota alternativa
Se permissões forem negadas/incertas para produto público, adotar cliente/engine de implementação própria, conteúdo com licença apropriada e eventual interoperabilidade independente, sujeita à revisão jurídica.
