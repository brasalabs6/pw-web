id: data-003
title: ~15 subsistemas do gdeliveryd com estado compartilhado entre timer Update() e handlers sem NENHUM lock (+ BattleManager declara locker e não usa; GetAuctionID sem lock)
category: data
type: bug
status: OPEN
severity: HIGH
owner: server153-gdeliveryd
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "Sem Thread::Mutex/RWLock em: kingelection.h:70-80, stockexchange.h:139-220 (Lock()/UnLock() de stockexchange.h:82-103 é flag lógico, não mutex), referencemanager.h:105-160 (comentário liuguichen 20121210 documenta o timing issue), rewardmanager.h:75-130, waitqueue.h:40-100, autoteamman.h, friendextinfomanager.h, playerprofileman.h:73-95, pshopmarket.h, forcemanager.h:33-45, solochallengerank.h:34-70, countrybattleman.h, tankbattlemanager.h, crosssystem.h:44-57 — todos com Update() no IntervalTimer (ex.: kingelection.cpp:125, waitqueue.cpp:330) e handlers no pool de threads"
  - "src/server153/cnet/gdeliveryd/battlemanager.h:60 — declara Thread::RWLock locker; battlemanager.cpp nunca o usa (só o lock do UserContainer)"
  - "src/server153/cnet/gdeliveryd/auctionmarket.cpp:348-364 — GetAuctionID incrementa m_maxAuctionID_ fora do m_lockermap_ (único método do AuctionMarket sem lock) → IDs duplicados"
  - "Padrão correto já existe: AuctionMarket/WebTradeMarket/FactionFortressMan/UniqueDataServer/SysAuctionManager têm mutex"
impact: "Corridas em eleição de rei, bolsa, referência/recompensa, fila de espera, autoteam, pshop, ranking, batalhas: Update() do timer muta estruturas enquanto handlers leem/escrevem (UB em std::vector/map) → crashes e corrupção de dados econômicos sob carga. IDs de leilão duplicados = leilões perdidos/dinheiro preso."
proposed_action: "Dar a cada subsistema um mutex próprio cobrindo Update() + handlers (padrão AuctionMarket), ou serializar o Update com os handlers via fila no mesmo thread. GetAuctionID: uma linha (atomic ou Scoped no m_lockermap_). Referencemanager: mutex interno independente do lock do UserContainer (OnLogin fora / OnLogout dentro hoje)."
validation_notes: "TSan com carga sintética (votos + ordens de bolsa + logins simultâneos) limpo; grep: nenhum subsistema com estado mutável sem mutex; GetAuctionID sob lock."
