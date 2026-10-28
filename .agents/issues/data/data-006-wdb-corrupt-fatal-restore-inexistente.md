id: data-006
title: WDB CORRUPT é fatal no boot sem recovery; restore inexistente (recover.cpp stub); backup cp -r sem checagem; kill(0,SIGKILL) e times_incbackup %0 no BackupThread
category: data
type: bug
status: OPEN
severity: HIGH
owner: server153-gamedbd (storage)
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/share/common/db.h:2057 — GlobalLogger::truncate() retorna CORRUPT (stub); db.h:2941-2956 — DBCollection::init propaga; gamedbd.cpp:87-90 — exit(-1) 'Initialize storage environment failed' (o PageLogger dos DBStandalone recupera via abort(); o GlobalLogger das 48 tabelas não)"
  - "src/server153/cnet/storage/recover.cpp:28-30 — StorageEnv::recover comentado e INEXISTENTE; DBCollection::restore(timestamp) (db.h:2969) não é exposto por nenhuma ferramenta CLI"
  - "src/server153/cnet/storage/storagewdb.h:665-688 — backup() usa system(\"/bin/cp -r\") sem checar retorno; mkdir sem check"
  - "src/server153/cnet/storage/storagewdb.h:728 — kill(0, SIGKILL) no quit_lockfile mata o GRUPO de processos inteiro (colateral em containers/scripts); :738 — (times-1)%times_incbackup com times_incbackup=0 (atol de conf ausente) → SIGFPE"
impact: "Um crash durante o commit do checkpoint (escrita rasgada) ou setor ruim deixa o storage CORRUPT → o gamedbd NUNCA MAIS SOBE sem intervenção manual (wipe --fresh = perda total de personagens). Os backups existem mas não há ferramenta de restore funcional — a garantia de backup é nominal. kill(0) pode derrubar o container inteiro."
proposed_action: "1) Implementar GlobalLogger::truncate() real (descartar checkpoint PREPARED e truncar o log — análogo ao PageLogger::abort() db.h:1789) OU auto-restore do último backup válido no boot. 2) Comando CLI 'gamedbd restore <backupdir>' que copia datadir+logdir e chama DBCollection::restore()/redo(). 3) Checar retorno de system()/mkdir no backup. 4) kill(getpid()) em vez de kill(0); default times_incbackup=1."
validation_notes: "Corromper um log de teste deliberadamente: o boot deve truncar/recuperar (ou restaurar backup) e subir; 'gamedbd restore' deve reconstruir o estado do backup; BackupThread não pode matar processos irmãos."
