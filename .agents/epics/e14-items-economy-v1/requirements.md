# Requisitos — Inventário, equipamentos, crafting, loja e economia

## Requisitos funcionais (RF)
- **RF-01**: Especificar esquemas de item, stacks, binds, inventário e mensagens binárias. Deve expor contrato testável e permitir falha controlada.
- **RF-02**: Implementar modelos de estado/HUD com fixtures e invariantes de slots. Deve expor contrato testável e permitir falha controlada.
- **RF-03**: Implementar drag/drop, equip, confirmação de compra e fluxo de craft mock. Deve expor contrato testável e permitir falha controlada.
- **RF-04**: Integrar autoridade servidor de comércio, banco e transações reais. Deve expor contrato testável e permitir falha controlada.
- **RF-05**: Executar testes de concorrência, cancelamentos, replay e duplicação. Deve expor contrato testável e permitir falha controlada.

## Interfaces e estados esperados
- **API-1 ItemInstance:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-2 InventorySlot:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-3 EquipTransaction:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-4 MerchantOffer:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.
- **API-5 TradeProposal:** contrato versionado com entrada inválida rejeitada, tipos explícitos, timeout/cancelamento quando aplicável, erros estruturados, logs correlacionados por trace e testes de backward compatibility.

## Requisitos não funcionais (RNF)
- **Compatibilidade:** browser sem acesso a Win32/Direct3D/TCP raw; identificar plataforma alvo e protocolo exatos.
- **Performance:** medir sem criar números falsos; budgets definidos e aprovados antes de otimizações. Instrumentar invariantes contra duplicação; idempotência de transação; auditoria de trade no servidor.
- **Confiabilidade:** operação repetível, sem vazamento de recursos, retry idempotente, tratamento de dados inconsistentes, queda de contexto e timeouts.
- **Segurança:** entrada externa não confiável, bounds checking, saneamento de caminhos, rate limiting quando houver rede, segredos só no lado apropriado.
- **Qualidade:** testes com fixtures sintéticas/permitidas, ambiente e hash de commit anotados, evidências anexadas ao PR.
- **Licenciamento:** conteúdo de terceiros só após gate de direitos; submódulos são referências não integradas automaticamente.
- **Observabilidade:** telemetria com eventos de erro e performance, sem capturar senhas, tokens, chats privados ou conteúdo protegido desnecessariamente.

## Restrições e não objetivos
- Mitigar risco de dup de item
- Mitigar risco de replay/ordem de trade
- Mitigar risco de manipulação de preço por cliente
- Não propor funcionalidades não verificadas como implementadas.
- Não acoplar endpoints de produção a testes de laboratório.
- Não extrapolar qualidade de WebGL procedural para cliente real.

## Evidências mínimas
- Comando de build/teste e plataforma.
- Fixture esperada e obtida, junto ao commit exato.
- Resultado pass/fail/blocked, logs e comparação, orçamento/limites quando relevante.
- Lista de divergências persistentes e próximo experimento testável.
