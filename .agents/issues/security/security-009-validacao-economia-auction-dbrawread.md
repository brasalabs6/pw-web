id: security-009
title: Validação de input de economia ausente — AuctionOpen repassa item_num/baseprice/elapse_time sem checar faixas; DBRawRead expõe dados raw
category: security
type: bug
status: OPEN
severity: MEDIUM
owner: server153-gdeliveryd
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/gdeliveryd/auctionopen.hpp:38-60,84-118 — item_pos/item_num/baseprice/binprice/elapse_time/deposit repassados sem validação (o padrão correto existe em pshopbuy.hpp:66-69); auctionget.hpp:31-49 aceita auctionid arbitrário"
  - "src/server153/cnet/gamedbd/dbrawread.hrp:86-130 — devolve pares chave-valor descomprimidos de qualquer tabela do conf a qualquer peer conectado (sem controle de acesso; browse 32/128KB, maxsize 256KB)"
impact: "Cliente (ou glinkd comprometido) cria leilão com preço negativo/duração absurda/quantidade inválida → economia corrompida e carga indevida no gamedbd. DBRawRead permite exfiltrar dados de jogadores de qualquer tabela se a rede interna for comprometida."
proposed_action: "1) Validar faixas no gdeliveryd antes de QueryDB (item_num>0, baseprice/binprice em [1, teto], elapse_time em [min,max], deposit>=0) — copiar o padrão de pshopbuy. 2) DBRawRead: exigir flag admin na sessão ou restringir a tabelas não sensíveis; documentar o risco. Valores de teto dependem das regras de economia → confirmar com o operador."
validation_notes: "Fuzz de AuctionOpen com valores negativos/overflow deve ser rejeitado no gdeliveryd antes de chegar ao gamedbd; DBRawRead de tabela sensível sem flag admin deve recusar."
