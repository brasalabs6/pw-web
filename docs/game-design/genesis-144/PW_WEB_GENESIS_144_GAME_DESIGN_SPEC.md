# PW Web — Genesis 1.4.4: especificação de gameplay, balanceamento e economia

> **Tipo:** Game Design Document (GDD) + contratos lógicos de configuração  
> **Versão do documento:** 1.0 — 28/10/2026  
> **Status:** decisões de produto aprovadas; parâmetros restantes sujeitos a homologação  
> **Mundos:** `genesis-classic` e `genesis-arena`  
> **Runtime pretendido:** cliente WebAssembly com shell multiplataforma; conteúdo e regras referenciados à era 1.4.4

## 0. Resumo e objetivos

Construir dois mundos persistentes com a identidade de Perfect World Gênesis de 2011, **cinco raças e dez classes**, mesmo shell e infraestrutura compartilhada, mas economias, personagens e progressões totalmente isolados.

- **Clássico:** vida longa, mundo social, jornada valorizada, comércio entre jogadores, Nirvana como meta final, progressão difícil e previsível.
- **Arena:** entrar em PvP rapidamente, começar com equipamento útil e refino +10 gratuito, evoluir até +12 e continuar interessado em combates, eventos, comunidade e coleção; conquistar o topo sem grind abusivo.
- **Compromisso central:** nenhum dos mundos deve depender de resets de personagens, inflação permanente de itens ou venda exclusiva de poder.

### 0.1 Decisões aprovadas pelo responsável do projeto

| ID | Decisão | Status |
|---|---|---|
| DEC-001 | Era de conteúdo Gênesis / 1.4.4, com Arcano e Místico | **Aprovado** |
| DEC-002 | Dois servidores: Clássico low-rate e Arena full PvP | **Aprovado** |
| DEC-003 | Clássico: EXP **1,5x** | **Aprovado** |
| DEC-004 | Arena: kit inicial com refino **+10 gratuito** | **Aprovado** |
| DEC-005 | Arena: progressão de refino até **+12** | **Aprovado** |
| DEC-006 | Nirvana como teto de equipamento nos dois mundos | **Aprovado** |
| DEC-007 | Temporadas redefinem **somente rankings** | **Aprovado** |
| DEC-008 | Evitar progressão excessivamente punitiva no Arena e desequilíbrio extremo de poder | **Objetivo aprovado** |

### 0.2 Recomendações iniciais — **ainda não aprovadas**

| ID | Parâmetro proposto | Motivo |
|---|---|---|
| PROP-001 | Clássico: teto de refino +8 | Reduz disparidade e inflação de poder, mantendo Nirvana valorizado |
| PROP-002 | Clássico: EXP de missões e espírito 1,5x; moedas e drops 1x | Evitar personagem evoluindo sem conseguir desenvolver habilidades |
| PROP-003 | Arena: nível inicial 101, com habilidades/cultivo essenciais liberados | Entrar em combate rapidamente |
| PROP-004 | Arena: kit Nirvana de atributos fixos e refino +10, **vinculado** | Evitar mercado de kits gratuitos e permitir comparação justa |
| PROP-005 | Arena: progressão até +11 no kit em 10–18h; até +12 em 30–50h de atividade efetiva total | Recompensar participação sem semanas de farm obrigatório |
| PROP-006 | Arena: diminuir **incremento de bônus de refino acima de +10** com curva explícita (inicialmente fator 0,35) | Reduzir diferença +10 vs. +12 |
| PROP-007 | Temporada competitiva Arena: 8–12 semanas; sem reset de equipamento, moeda, nível ou itens | Renovar objetivos sem perder conquistas |
| PROP-008 | Clássico: sem reset de progresso nem de ranking estrutural do mundo (ex.: territórios) | Preservar identidade de mundo permanente |

**Regra de governança:** qualquer PROP só vira configuração imutável de lançamento depois de testes, aprovação e entrada no changelog de decisões. Nenhuma dessas propostas deve ser apresentada como decisão já tomada pelo responsável.

## 1. Fidelidade histórica e compatibilidade

### 1.1 Referência de conteúdo

A base é a expansão *Genesis* (patch 1.4.4) de 2011: Humanos (Guerreiro, Mago), Selvagens (Bárbaro, Feiticeira), Alados (Arqueiro, Sacerdote), Abissais (Mercenário, Espiritualista) e Guardiões/Arcanos (Arcano, Místico). Cinco raças, dez classes.

O patch Genesis incluiu também nova área inicial dos Guardiões, fabricação de runas e bases de guildas; alguns recursos e habilidades dos Guardiões chegaram em atualizações subsequentes. **A data exata de conteúdo considerada 1.4.4 deve ser congelada em um inventário de ativos e patches**, sem presumir que toda funcionalidade conhecida de versões posteriores já estivesse disponível no dia de lançamento.

### 1.2 Compatibilidade com o port em andamento

O port WebAssembly pode partir tecnicamente de código de cliente 1.5.3, mas **a versão do runtime não define automaticamente a era de conteúdo**.

Construir o `era_manifest` independente do cliente, controlando:

- Classes permitidas; aparência, mapas, NPCs, portais e regiões.
- Habilidades por classe, ranks, requisitos, efeitos e animações.
- Cultivo, Daimons, missões, instâncias, monstros e receitas.
- Ids de equipamentos, regras de forja, encantamento, refino e gemas.
- Sistemas removidos/introduzidos depois da era; regras de compatibilidade de assets.
- Identificadores e checksums de tabelas de itens, skills e drops.

Proibir uma feature futura não é só esconder sua interface: servidor deve impedir criação, equipagem, comércio e execução de seus dados.

**Teste de ouro:** cliente e servidor só entram em produção se comparações automatizadas com a tabela canônica da era 1.4.4 não apresentarem itens, habilidades, monstros ou sistemas futuros habilitados indevidamente.

## 2. Modelo de conta e isolamento

- **Uma identidade e um shell de login:** usuário escolhe Clássico ou Arena, faz login e cliente/asset bundle são resolvidos automaticamente para a compatibilidade desse mundo.
- **Dois universos econômicos:** cada personagem pertence a um único `realm_id` imutável.
- Inventários, armazéns, moedas, bancos, leilões, mensagens com itens, correspondência, guildas, ranking, território e progressão são isolados por mundo.
- **Nunca permitir** envio, migração, duplicação ou transferência de itens/moedas entre Clássico e Arena.
- Recursos de conta que atravessam mundos só podem ser não econômicos e explicitamente autorizados (preferências de interface, idioma, acessibilidade, segurança).
- Regra de integridade para toda transação: `transaction.realm_id == source.realm_id == destination.realm_id`; validação do servidor e do armazenamento.
- Rankings de Arena por temporada, mas personagens e bens nunca são reinicializados.

## 3. PW Gênesis Clássico

### 3.1 Promessa ao jogador

O mundo deve parecer PW de 2011: começar fraco, depender de grupo e profissões, produzir equipamentos que tenham valor, ganhar reputação social, desenvolver personagem em longo prazo e alcançar Nirvana sem expansão que o torne obsoleto.

### 3.2 Rates

| Variável | Regra proposta | Observação |
|---|---|---|
| EXP de monstros | **1,5x** | Decisão aprovada |
| EXP de missões | 1,5x | Proposta coerente com EXP; homologar escopo |
| Espírito / pontos de skill | 1,5x | Proposta para não atrasar habilidades |
| Drop de monstros | 1x | Original, sem drops adicionais permanentes |
| Moedas de monstros | 1x | Não imprimir moeda artificialmente |
| Materiais de coleta | 1x | Sem multiplicar coleta ou nós do mapa |
| Ouro, Dusk, Nirvana | Tabelas 1.4.4 canônicas | Sem simplificação silenciosa |
| EXP de bônus/hiper EXP | Compatibilidade histórica auditada; sem multiplicação acidental cumulativa | Evitar aceleração oculta |

**Nota:** “EXP 1,5x” não autoriza automaticamente modificar taxas de drop, moedas, XP de eventos ou requisitos do conteúdo. Valores propostos precisam de comparação com referência real.

### 3.3 Loop de progressão

1. **Níveis 1–39:** história da raça, controles, equipamentos iniciais e profissões. Não entregar Nirvana, asas raras ou moeda considerável.
2. **Níveis 40–69:** cultivo, habilidades, Dungeons, equipamentos por monstros e primeiras trocas.
3. **Níveis 70–89:** Dusk, equipamentos especiais, missões de grupo, consumíveis e participação em guildas.
4. **Níveis 90–99:** preparação do conjunto 99, economia de materiais e objetivos de longo prazo.
5. **Nível 100+:** Nirvana como meta de equipamentos; níveis até 105 continuam uma conquista, mas não devem invalidar todo conteúdo do nível 100.

**Meta inicial de produto:** nível 100 deve demandar muitas sessões reais (hipótese para teste: mediana na ordem de 120–240 horas de atividade por personagem típico), ajustada depois de simular a fórmula real de XP por atividade, quantidade de jogadores em grupo, itens de bônus e tempo improdutivo. **Não prometer esse número antes da medição.**

### 3.4 Conteúdo e teto de poder

- Dusk, equipamentos 99 e Vale da Lua como etapas relevantes, quando incluídas no snapshot histórico.
- Nirvana como ponto final do equipamento. Permitidas apenas formas/reconstruções **atestadas** no snapshot 1.4.4 selecionado.
- Equipamentos futuros como Morai, R9 pós-era, Nirvana terceira reformulação ou outras expansões não entram sem revisão explícita; padrão é **bloqueado**.
- A recomendação de refino máximo inicial é +8 (**pendente de aprovação**). Não vender proteção de refino ou gemas exclusivas na loja.
- Guardar o custo e a raridade do Nirvana com materiais de forja e progressão, sem introduzir desgaste extraordinário/obrigatório fora das regras históricas.

### 3.5 Economia lenta e íntegra

**Fontes de moeda:** monstros, missões, venda de itens e sistemas canônicos.  
**Saídas de moeda:** NPCs, consumíveis, reparos canônicos, forjas, teleportes, leilões, serviços de clã e taxas moderadas de comércio.  
**Fontes de materiais:** monstros, coleta, Dusk, Nirvana, manufatura e chefes.  
**Saídas de materiais:** forjas, receitas, consumíveis, aprimoramento e rerrolagem limitada (se existir no snapshot).

**Guardrails:**

- Não injetar equipamentos raros por presentes de login, premiações de Discord ou eventos de publicidade.
- Não vender moeda comerciável nem equipamentos competitivos em dinheiro real.
- Recompensas de eventos não podem gerar mais moeda/hora do que o principal conteúdo de farm ao ponto de matá-lo.
- Sem NPC ilimitado recomprando materiais a preço artificialmente alto.
- Variantes de loot e taxas documentadas; não alterar silently dados canônicos para forçar monetização.
- Para recém-chegados, preferir guias, grupos, transporte razoável e itens iniciais vinculados de baixo valor em vez de presentes econômicos.

### 3.6 Estabilidade de longo prazo

Em um servidor permanente, as pessoas terminam seus equipamentos. O projeto não deve afirmar que haverá demanda eterna por um cristal apenas porque ele é parte de uma receita.

Plano de sustentação: entrada contínua de novatos, rerrolagem limitada, profissões úteis, consumíveis que atendam necessidades reais e trocas entre veteranos e novatos. Qualquer novo consumo recorrente de material deve ser opcional e não punitivo, com teste prévio de preço e disponibilidade.

## 4. PW Gênesis Arena

### 4.1 Promessa ao jogador

O jogador pode criar personagem, escolher build e entrar em PvP no mesmo dia. O +10 é **verdadeiramente gratuito**, sem assinatura, pagamento oculto, disponibilidade temporária ou tarefa de dias. O +12 representa objetivo atingível, não um abismo para novos jogadores.

### 4.2 Onboarding e kit inicial — proposta para validação

- Nível inicial sugerido: **101**; até 105 com progressão acelerada, sem transformar níveis em barreira de entrada.
- Cultivo, skills essenciais, Daimon funcional e equipamentos em estado utilizável desde o início; padronizar possibilidades por classe, sem dar um pacote de habilidades fora de sua era.
- Kit base: **Nirvana da era 1.4.4**, configuração canônica padronizada por classe, refino +10 em todos os slots refináveis elegíveis, gemas compatíveis e itens básicos de PvP. Seleção de builds permitida entre opções predefinidas, auditadas.
- Kit gratuito vinculado à conta/personagem e sem valor de revenda ou conversão em materiais.
- Não usar Nirvana 3ª forja, equipamento R9 ou sistemas de eras posteriores como salto de poder. O ápice continua Nirvana.
- Itens consumíveis fundamentais de PvP são gratuitos ou baratos, para não criar cobrança por luta.
- Meta UX: primeira luta em **até 15 minutos**, após tutorial breve e configuração de teclas/toque.

### 4.3 Progressão de equipamentos

**T0 — Entrada (imediato):** kit Nirvana +10 de estatísticas comparáveis, dano/defesa funcionais.  
**T1 — Otimização (+11):** recompensas por PvP, PvE e comércio; manter possibilidade de duelar contra +10.  
**T2 — Prestígio (+12):** objetivo final de refinamento, gradativo, sem perda de equipamento base nem RNG que recomece do zero.

**Metas iniciais por conjunto elegível completo, NÃO por peça:**

| Marco | Tempo de participação efetiva (mediana) | Observação |
|---|---:|---|
| Competitivo com +10 | Imediato | Entrega gratuita |
| +11 no conjunto | 10–18 horas totais | O esforço depende da atividade escolhida |
| +12 no conjunto | 30–50 horas totais | Inclui trajetória até +11; testar com todas as classes |

Tempos são **hipóteses de tuning**. Para não premiar AFK, somente atividades validadas contam. Jogadores focados em PvE ou que perdem batalhas também precisam de uma rota razoável.

### 4.4 Como controlar +10 versus +12

**Problema:** nas tabelas usuais de refino de Perfect World, +10 multiplica o incremento-base por aproximadamente 22,3; +11 por 29; +12 por 37,5. O bônus adicional de refino é aproximadamente 68% maior em +12 comparado a +10. **Isso não equivale a 68% de poder total do personagem.**

Testar uma curva transparente e igual para as dez classes **somente no Arena**:

```text
rawBonus(slot, refine) = bonusBase(slot) * historicalMultiplier(refine)

if refine <= +10:
    effectiveBonus = rawBonus
else:
    effectiveBonus = rawBonusAt10
                   + alpha * (rawBonus - rawBonusAt10)

alpha = 0.35   # apenas valor de partida para simulação, não balanceamento final
```

Por exemplo, o multiplicador +12 efetivo do **componente de refino** seria aproximadamente `22,3 + 0,35 × (37,5 − 22,3) = 27,62`, ~24% acima do componente +10. Isso **não** é uma diferença prevista de 24% no dano ou sobrevivência total.

- A curva Arena aplica-se de forma consistente a ataque/HP/defesa gerados pelo refino, sem benefícios ocultos.
- Exibir no painel do personagem os valores efetivos, sem apresentar um atributo fictício.
- A curva tem de ser aplicada no servidor, e não apenas na interface.
- O Clássico usa a fórmula histórica do snapshot, sem suavização.
- Se simulações encontrarem desigualdade extrema por classe ou slot, alterar `alpha` e/ou a distribuição de kit **antes** de abrir o servidor, não fazer correções de último minuto sem transparência.
- O Arena pode oferecer um modo torneio com kits **normalizados** para medir habilidade pura; o mundo aberto mantém progressão até +12.

### 4.5 Economia de evolução

Separar três recursos com papéis claros:

1. **Moedas:** negociáveis; circulação tradicional, leilões, cosméticos, serviços econômicos e compras básicas.
2. **Marcas de Guerra:** não negociáveis e vinculadas; ganhas por participação validada em PvP, eventos de clã e metas esportivas; demonstram participação e limitam a compra do progresso apenas com moeda.
3. **Cristais de Conquista:** negociáveis; obtidos por PvE, atividades de guilda, objetivos semanais e recompensa de combate; consumidos na melhoria do Nirvana e em forjas cosméticas.

**Receita de progressão do Arena:** refino acima de +10 exige uma combinação de Marcas de Guerra + Cristais de Conquista, com custo em moedas. Não existe receita pagável exclusivamente com dinheiro real. Os números finais de materiais/custos dependerão de observações de ganho/hora e volume transacionado no beta.

#### Anti-exploit da economia PvP

- Não recompensar abates infinitos de alvos repetidos ou contas controladas pelo mesmo jogador.
- Detectar padrões de win-trading, times alternando vitórias, IP/dispositivo recorrente (como indicador, nunca prova isolada) e matches sem participação real.
- Derrotas honestas também recebem progresso, menor que vitórias, para não excluir iniciantes.
- Recompensas de arena consideram participação, variedade de oponentes e consistência; manter explicabilidade ao jogador.
- Limites diários e/ou retornos decrescentes somente quando os dados mostrarem risco real; não transformar progressão em obrigação de login diário.
- Tudo que for retirado do inventário deve ter `reason`, `source_event_id` e identificação de transação auditável.

### 4.6 Atividades recorrentes

- **Diárias:** duelos, arena casual, desafios PvE curtos e missões sociais leves.
- **Semanais:** 3v3, 6v6, objetivos de guilda, guerras territoriais, chefes e ranking individual/de equipe.
- **Mensais:** torneios com premiação **cosmética** e títulos não transferíveis; calendário previsível.
- **Temporadas:** 8–12 semanas inicialmente; reiniciar rating, ladder e recompensas sazonais, não reiniciar personagens, skills, moedas, inventários, refinamentos nem conquistas permanentes.
- **Atração para veteranos com +12 completo:** torneios, metas sociais, builds alternativas, coleção e prestígio; **não** introduzir +13, R9 ou gear creep.

## 5. Diretrizes de combate e balanceamento

### 5.1 Princípios

- Papel de classe importa: tanque, cura, controle, burst, suporte, furtividade e pressão de área não devem se tornar homogêneos.
- Testar 1v1, 3v3, 6v6, PvP aberto e guerras territoriais; não corrigir automaticamente problemas de 1v1 que afetem negativamente PvE ou guerra.
- Priorizar contrajogo claro: animações legíveis, janelas de reação, custo de cooldown, linha de visão, escape e recursos finitos.
- Autoridade do servidor sobre dano, cura, efeitos, posição validada, alvo, cooldown, recursos e resultados de luta.
- Comparação entre toque e teclado/mouse: assistência mobile não pode selecionar alvos invisíveis, automatizar rotação completa ou aumentar DPS; ajustar UI, não regras de combate.

### 5.2 Objetivos de homologação (provisórios)

| Indicador | Objetivo de avaliação | Condição |
|---|---|---|
| Vitória por classe | Investigar desvios persistentes após ajustar habilidade/rating e composições | Amostra mínima definida por confiança estatística, não só n bruto |
| TTK em 1v1 pareado | Meta inicial de 8–20 s como faixa de avaliação, não regra universal | Testar burst, críticos e builds completas |
| Morte instantânea sem contrajogo | Investigar sequências que eliminem personagem em janela inferior à resposta plausível | Diferenciar combo legítimo de bug/exploit |
| Grupo vs grupo | Nenhuma combinação deve dominar sistematicamente sem counterplay | Mapear 3v3 e 6v6 |
| Arena +10 vs +12 | Jogador +10 habilidoso conserva probabilidade real de vitória | Matchups controlados, não promessa percentual arbitrária |
| PvE | Todas as dez classes têm papel viável em grupo | Medir contribuição por função, não só DPS |
| Performance em massa | Habilidade, alvo e dano permanecem corretos sem dessync | Stress test com dezenas/centenas de personagens simulados |

### 5.3 Plano de testes de dez classes

1. Gerar kits T0, T1, T2 canônicos por classe, com combinações de atributos/skills/daimon rastreáveis.
2. Executar cálculo offline de DPS, burst, cura, mitigação, custo de recurso e uptime de controle.
3. Simular luta com latência (20, 80, 150, 250 ms), perda de pacote e entradas mobile.
4. Rodar partidas pareadas em duelos, grupos e mass PvP; registrar replay e hashes de estado.
5. Publicar matriz de diferença +10/+12 e impactos por slot, incluindo PvE.
6. Congelar build candidata, rodar beta fechado, refazer tuning e documentar changelog.

**Protocolo de alterações:** patches de balanceamento com versão (`balance_revision`), justificativa, métricas anteriores/posteriores e rollback. Não mudar tabelas de dano de maneira silenciosa.

## 6. Economia: invariantes e métricas

### 6.1 Economia independente por mundo

- Clássico não pode receber itens ou divisas do Arena, nem vice-versa.
- Cada ação econômica é operação transacional do servidor (moeda, item, material, saldo, leilão).
- Usar identificadores únicos para itens de valor, anti-duplicação, transaction log imutável, idempotência e reconciliação de estoques.
- Auditoria de admin, GMs, eventos, reembolsos e recuperações de conta separadas do tráfego normal.

### 6.2 Painel mínimo de economia por servidor

1. Moeda gerada/dia e consumida/dia, por origem/destino e coorte.
2. Estoque monetário ativo versus contas inativas, percentis de riqueza e concentração no top 1%.
3. Produção, destruição e estoque dos principais cristais/materiais.
4. Mediana, p25 e p75 de preços de uma cesta de materiais, com filtro de wash trading.
5. Valor e giro de leilões/comércio, preço efetivo por unidade.
6. Tempo e número de sessões para primeiro equipamento 99 e primeiro Nirvana (Clássico).
7. Tempo e sessões para +11/+12 completo (Arena).
8. Taxa de atividade paga pelo comércio vs. PvE vs. PvP, retenção e abandonos por gargalo.
9. Bot signals, transações suspeitas, sanções e apelações.
10. Base de jogadores simultâneos, DAU/MAU, retenção D1/D7/D30 e clãs ativos.

### 6.3 Guardrails de intervenção

- Não corrigir inflação/escassez simplesmente distribuindo novos itens ou apagando saldos.
- Antes de alterar taxas, revisar histórico de origem dos itens e segmentar jogador novo/antigo.
- Alterar receitas/loot apenas via revisão auditável de config, com simulação e aviso prévio.
- Se a economia do Clássico ficar travada, preferir revitalização de conteúdo existente e novos jogadores ao poder artificial.
- Se Arena virar farm de alt, reduzir incentivos fraudáveis e ampliar recompensas de colaboração legítima.

## 7. Guerras territoriais, PvP e comunidades

- **Clássico:** preservar guerra territorial e seus objetivos históricos, equilibrando horários, concorrência, distribuição de guildas e capacidade de servidores.
- **Arena:** combate imediato, arenas organizadas, mundo aberto e TW com premiações não capazes de aumentar ainda mais o poder da guilda dominante.
- Recompensas competitivas: cosméticos, títulos, reconhecimento, material negociável limitado e prestígio; nunca itens exclusivos de combate que criem um ciclo dominante.
- Anti-snowball: critérios transparentes de elegibilidade, proteção de jogadores novos, pareamento de equipes quando aplicável e limites de farming por contas alternativas.
- Rankings separados por mundo, classe/modo, período e temporada; histórico legível e auditável.

## 8. Temporadas e permanência

- Temporada representa **ciclo de competição**, não reinício de mundo.
- Ao iniciar nova temporada, podem ser zerados **apenas** pontuação de ranking, ladder e estatísticas competitivas sazonais explicitamente identificadas.
- Não zerar: nível, XP, skills, quests, guildas, territórios persistentes (salvo regra explícita e aprovada de competição de territórios), refinamento, inventários, moedas, cosméticos, bancos e coleções.
- Publicar antes da temporada uma lista exata de campos que serão redefinidos.
- Preservar histórico de rankings de temporadas anteriores sem revogar prêmios já obtidos.

## 9. Configuração declarativa de mundos — exemplo de contrato

**Exemplo de design**, não arquivo de produção nem evidência de que esses campos já existam no repositório:

```yaml
ruleset_schema: 1
content_era: genesis_1_4_4
client_resolution: automatic_by_realm
runtime_compatible_with:
  - wasm_client_1_5_3_port  # compatibilidade real ainda deve ser validada

realms:
  genesis-classic:
    persistent: true
    isolated_economy: true
    allowed_classes: [warrior, mage, barbarian, venomancer, archer,
                      cleric, assassin, psychic, seeker, mystic]
    xp:
      monster: 1.5
      quest: 1.5           # PROPOSTO; rever escopo de EXP
      spirit: 1.5          # PROPOSTO
    coins: {monster: 1.0}
    drop: {base: 1.0}
    endgame_gear: nirvana_1_4_4
    max_refine: 8          # PROPOSTO, NÃO APROVADO
    arena_refine_soft_cap: false
    season_reset: []

  genesis-arena:
    persistent: true
    isolated_economy: true
    allowed_classes: [warrior, mage, barbarian, venomancer, archer,
                      cleric, assassin, psychic, seeker, mystic]
    initial_level: 101    # PROPOSTO
    starter_gear:
      tier: nirvana_1_4_4_template  # PROPOSTO
      refine: 10         # APROVADO
      bind_on_grant: true
      tradable: false
    endgame_gear: nirvana_1_4_4
    max_refine: 12        # APROVADO
    refine_above_10_scale: 0.35  # PROPOSTO, validação de combate necessária
    rank_season:
      weeks: 10           # PROPOSTO
      resets: [season_rating, season_rank, season_points]
      preserves: [level, xp, inventory, coins, skills, refinement, guild]
```

**Contratos obrigatórios:** `RealmManifest`, `EraManifest`, `ClientBundleManifest`, `GameplayRuleset`, `ItemDefinition`, `SkillDefinition`, `DropTable`, `CurrencyLedger`, `EconomicEvent`, `SeasonResetPlan`, `BalanceRevision`, `CombatReplay`, `PlayerProgressionEvent`.

### 9.1 Relação com o shell e atualizações

- O shell mostra os dois mundos; o usuário seleciona um e autentica.
- O servidor fornece o `realm_id`, `ruleset_hash`, `era_manifest_hash`, `client_bundle_id` e requisitos de protocolo.
- O shell seleciona bundle e cache compatíveis automaticamente; usuário não escolhe versão de cliente.
- Prévia/deploy de conteúdo e backend por mundo, antes de publicação no mundo principal.
- Atualizações atômicas: cliente e servidor rejeitam contratos incompatíveis de forma compreensível, sem corromper estado.
- Assets comuns podem ser compartilhados por hash; caches de patches específicos são isolados/versionados.
- Versão do cliente **não** define rates, drops ou credita moeda: servidor sempre é a autoridade.

## 10. Segurança e integridade

- Limites de requisição por conta/sessão; desafio de login; MFA opcional; senha e sessão com controles modernos.
- Não confiar em timestamp, alvo, loot ou cálculo de combate enviado pelo cliente WebAssembly.
- Validar velocidade, teleporte, cooldown, skill range, linha de visão, estado de controle e uso de consumível.
- Gerar GUID de item no servidor, validar transferência única e estoque final.
- Incluir logs de grants de GM e ferramentas de estorno justificado, com dupla aprovação para operações de alto valor.
- Recuperação e backup testados, restauração por realm, replay de transações e prova de ausência de duplicação.
- Anti-AFK/anti-bot baseado em evidências combinadas e sistema de apelação, para evitar punir jogadores legítimos no mobile.

## 11. UX multiplataforma

- **PC:** mouse/teclado, barras de skills personalizáveis, visualização de clã e chat, controles precisos para PvP e TW.
- **Celular:** layout de skills específico, câmera, seleção de alvo, minimapa, chat prático, inventário adaptado e escala de UI.
- **Ambos:** mesmo personagem e persistência do mundo, com regras de combate idênticas; não oferecer vantagens numéricas por dispositivo.
- Download progressivo/caching de assets, pré-carregamento de animações frequentes, distância dinâmica de efeitos e LOD em guerras.
- Métricas de entrada no jogo, crash-free sessions, memória, taxa de renderização e latência de input segregadas por dispositivo.

## 12. Critérios de aceite e testes

| ID | Aceite |
|---|---|
| ACC-001 | Os 10 personagens/classes do snapshot 1.4.4 podem nascer, evoluir e combater sem habilidades futuras indevidas. |
| ACC-002 | Clássico concede XP 1,5x para as fontes aprovadas e mantém moedas/drops 1x. |
| ACC-003 | Clássico: cadeia de Dusk/99/Nirvana validada sem atalhos de economia não autorizados. |
| ACC-004 | Arena: criação + login + kit +10 acontece sem uso de dinheiro real e sem atraso artificial. |
| ACC-005 | Arena: progresso até +12 pode ser concluído por caminhos PvP/PvE viáveis e auditáveis. |
| ACC-006 | Experimentos +10 vs. +12 provam que há contrajogo, sem dominância absurda em todas as classes. |
| ACC-007 | Nenhum item, moeda ou personagem pode atravessar os dois mundos. |
| ACC-008 | Temporada de Arena redefine só ranking; itens, skills, refinamento e moedas preservados. |
| ACC-009 | Rollback/restore não duplica itens nem credita moeda duas vezes. |
| ACC-010 | Cliente no browser e em mobile homologado executa combate e inventário sem divergência de estado. |
| ACC-011 | Economias separadas possuem painel de origens/saídas e alertas de inflação, escassez e fraude. |
| ACC-012 | Toda mudança em rates/combat possui versão, diff, justificativa e possibilidade de rollback. |
| ACC-013 | Equipamentos de eras futuras/R9 e ultrapassagem de Nirvana são barrados por dados e servidor. |
| ACC-014 | Rankings históricos de temporada são preservados e novas ladders começam com estado de competição zerado. |

## 13. Sequência de implementação e dependências

**P0 — Necessário antes do beta funcional**

- [ ] Finalizar migração browser/WebAssembly existente: movimento, autenticação, personagem, combate, skills, renderização e assets.
- [ ] Criar inventário de compatibilidade 1.4.4 e extrair baseline verificável.
- [ ] Implementar manifest/contratos por `realm_id` e seleção automática no shell.
- [ ] Implementar isolamento de inventário, moeda, personagens e itens.
- [ ] Aplicar rulesets de XP, drop, moeda, nível e equipamento no servidor.
- [ ] Testes de consistência client/server, idempotência e replay de combate.

**P1 — Clássico testável**

- [ ] Auditar missões, mapas, cultivo, skills, Daimons e instâncias para snapshot 1.4.4.
- [ ] Auditar XP 1,5x e trajetórias de níveis; calibrar tempo até nível 100.
- [ ] Testar cadeia de drop, Dusk 99 e Nirvana.
- [ ] Implementar métricas de moedas/materiais e mercado.
- [ ] Homologar teto de refino Clássico (ainda aberto).

**P1 — Arena testável**

- [ ] Criar kit +10 por classe, skills e Daimon, sem vantagem indevida.
- [ ] Implementar moedas de progresso, receitas, ledger e limites antifraude.
- [ ] Implementar progressão +11/+12 sem perda punitiva de todo refinamento.
- [ ] Prototipar e medir curva de refino acima de +10.
- [ ] Construir arena, eventos e ranking com resets somente competitivos.

**P2 — Preparação de lançamento**

- [ ] Simulações de progressão/mercado por centenas de horas de personagem.
- [ ] Beta de 100–300 jogadores para confirmar funis, retensão e economia.
- [ ] Testes de guerra territorial e grandes concentrações de skills em PC/mobile.
- [ ] Testes de recuperação de falhas, backups, reembolso e reconciliação.
- [ ] Revisão jurídica de licenças/IP, marcas, redistribuição de código e assets antes de operação pública/comercial.
- [ ] Congelar configurações, regras públicas e roadmap de temporadas.

## 14. Perguntas de aprovação restantes

1. Qual será o teto definitivo de refino **do Clássico**: +8 (recomendado), +10 ou +12 original?
2. A taxa de XP 1,5x aplica-se igualmente às **missões** e aos **pontos de espírito**?
3. No Arena, o kit +10 é de **Nirvana da mesma geração** para todos, ou devemos entregar Dusk 99 com transformação gratuita imediata?
4. O Arena deve usar o **fator de suavização** acima de +10? Aprovar somente após prova quantitativa.
5. Quais peças são elegíveis à progressão +12 e qual o custo real por peça?
6. Quais skills/sistemas posteriores ao lançamento original de Gênesis devem ser aceitos como correções, se houver?
7. O calendário de 10 semanas do Arena e os modos de ranking atendem o perfil competitivo desejado?
8. Qual estratégia de operação/licenciamento será adotada antes de lançar publicamente?

## 15. Referências públicas e limitações

- **PWI Genesis, patch 1.4.4 (2011):** https://pwi.fandom.com/wiki/PWI_Genesis
- **PWpedia — Refining, bônus +10/+11/+12:** https://pwi.fandom.com/wiki/Refining
- **Wiki PW Brasil — Refinamento:** https://wikipw.com.br/index.php/Refinamento
- **PWpedia — Nirvana Gear:** https://pwi.fandom.com/wiki/Nirvana_Gear
- **The Classic PW 1.4.4 (comparador de design, não referência canônica):** https://pw144.theclassic.games/faq
- **Perfect World — termos sobre servidores não autorizados:** https://www.perfectworld.com/safestatic/contract.html

Estas referências ajudam a fundamentar a especificação, mas **não substituem** a extração e a auditoria das tabelas reais do snapshot 1.4.4 que o projeto decidir usar. Em especial, custos de cristais, itens R8/R9 e disponibilidade de mecânicas variaram entre patches e regiões.

**Observação jurídica:** a licença de Perfect World exige autorização relevante para servidores privados e uso de sua propriedade intelectual. A especificação é planejamento de design e não resolve o direito de operação pública/comercial.