id: operations-010
title: Performance de gameplay/orquestração — broadcasts O(N) por login/logout, WorldChat O(N), sends de rede sob lock, page cache ~1GB em 32-bit, DoHeartbeat O(N), Skill::Create por operação
category: operations
type: enhancement
status: OPEN
severity: MEDIUM
owner: server153-gdeliveryd/cgame
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "gdeliveryd: map.cpp:282-287 (BroadcastProtocol por logout), enterworld.hpp:40-41 (por login), switchserversuccess.hpp:40-42 — O(N_gs) mensagens por evento de player; playerlogin_re.hpp:175/tradestart.hpp:90/auctionopen.hpp:106-112 — dsm->Send/QueryDB SOB o lock do UserContainer"
  - "glinkd: worldchat.hpp:30-33 — for_each em todos os roles online por mensagem de chat; glinkserver.hpp:316-318 — AccumProto4Switch clona cada pacote na janela de switch sem limite de fila"
  - "gamedbd: conf cache_high 16384 páginas×4KB = 64MB por tabela quente ×4 + 4096×16MB → ~960MB de page cache num processo 32-bit (limite ~3GB); ThreadContext::lock() (storagewdb.h:759-767) serializa TODAS as storages por transação"
  - "cgame: actobject.cpp:1300 — DoHeartbeat por tick com RefreshSubscibeList incondicional; start.cpp:57-99 — contabilidade de alocação (interlocked_add por new/delete); cskill: skill.h:520-525 — Skill::Create = map::find + heap alloc por operação de skill (StartSkill/Run/Condition/Interrupt/Cancel)"
impact: "Limites de escala: pico de login/logout gera tempestade de broadcasts; chat mundial O(N) por mensagem; lock global do UserContainer retido durante I/O de rede (mesma família do incidente do rwlock); gamedbd com risco de exaustão de address space 32-bit sob carga."
proposed_action: "1) Sends fora do lock (padrão já aplicado no enterworld — estender a todos). 2) Broadcast seletivo por interesse/batch. 3) Dimensionar cache_high pelas tabelas quentes reais (resto 1024). 4) Rate limit na fila de switch. 5) Skill: reusar stub para leitura de propriedades imutáveis + pool de Skill. 6) Medir com perf counters antes de cada mudança."
validation_notes: "Load test com N players sintéticos: latência de login/logout e throughput de chat medidos antes/depois; RSS do gamedbd < limite com conf de cache ajustada."
