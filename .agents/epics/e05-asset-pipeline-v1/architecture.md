# Arquitetura — Pipeline de recursos PCK/PKX, formatos e proveniência

Bytes não confiáveis → reader limitado → índice de variante confirmada → descompressão com budget → nomes lógicos → manifesto/hash → preload → bytes prontos para cena. Contrato do formato vem da evidência do produtor; consumidor não inventa layout.

## Donos e consumidores

Parser de uma variante, manifesto e preload limitados; OPFS/CDN completo continua em e15. O pw-web é dono dos novos contratos de plataforma; o servidor
é dono do wire format; referências são evidência, não dependências automáticas do
build público. Recursos possuem owner/lifetime explícitos; erro não é sucesso vazio.

## Caminhos de falha

- WO-01: Nome de extensão ou versão aproximada não determina layout binário.
- WO-02: Schema incompleto impede aprovação; não se cria contrato no consumidor para preencher ausência de evidência.
- WO-03: Bombas de descompressão e offsets além do arquivo são rejeitados antes de exceder os limites.
- WO-04: Dependência ausente/cíclica ou mesmo ID com conteúdos diferentes falha sem manifesto parcialmente válido.
- WO-05: Asset de outra versão ou sem autorização não integra o corpus válido.
- WO-06: offset+length que transborda ou tamanho descomprimido além do budget não permite alocação.
- WO-07: Arquivo de 3 bytes não permite read_u32; erro não avança cursor.
- WO-08: Magic desconhecido, registro truncado e intervalo cruzando arquivo sem regra definida são rejeitados.
- WO-09: Stream curto e saída anunciada acima do budget retornam erro controlado.
- WO-10: Duas entradas que convergem para o mesmo nome e encoding inválido falham explicitamente.
- WO-11: Hash diferente ou dependência ausente impede READY mesmo se HTTP retornou 200.
- WO-12: Cancelamento não deixa recursos half-ready nem conserva referências a buffers descartados.

## Dependências reais

Ver dependencies dos JSONs e o grafo em [port-program.md](../../port-program.md).
Não há requisito global de compilar todo Windows antes de R0 sintético. Integração
original e paridade final exigem a baseline, closure e autorização correspondentes.

## Contratos canônicos

- [docs/LEGAL.md](../../../docs/LEGAL.md)
- [docs/ANGELICA-PORTING-METHOD.md](../../../docs/ANGELICA-PORTING-METHOD.md)
