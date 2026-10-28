id: data-005
title: WDB perde dados silenciosamente — checkpoint de 60s é o único flush (RPO 60s) e erros de disco são engolidos (abort comentado)
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
  - "src/server153/share/common/db.h:1507-1527 — PageCache::put só altera páginas em memória; nenhum fsync/log por operação; db.h:2944-2956 — o log só recebe snapshot no prepare/commit do CHECKPOINT; BackupThread (storagewdb.h:695-755) é o único disparo, a cada checkpoint_interval=60s (gamedbd.conf:53)"
  - "src/server153/cnet/storage/storagewdb.h — TODOS os catch(PageFile::Exception) têm //abort() comentado (insert:276-279, find:309-312/332-335, del:362-365, checkpoint:245-248, walk:424-471, StorageEnv::checkpoint:639-642, BackupThread:720-723)"
impact: "1) Crash (SIGKILL/segfault/OOM) perde até 60s de escritas — dinheiro, inventário, XP, criação de role — enquanto o gdeliveryd JÁ recebeu sucesso do RPC. 2) Erro de disco (ENOSPC) no insert é engolido: o RPC retorna ERR_SUCCESS sem gravar — perda silenciosa sem alerta."
proposed_action: "1) Reduzir checkpoint_interval para 5-10s (RPO menor; custo de I/O) e/ou fsync do log a cada N segundos. 2) Reabilitar tratamento de erro de disco: propagar ao RPC (retcode de erro) e marcar o storage read-only — nunca reportar sucesso sem gravar. 3) Documentar o RPO como decisão (ops-005 relacionada: shutdown gracioso reduz a janela)."
validation_notes: "Teste de crash controlado: gravar role, matar -9 o gamedbd antes do checkpoint, reiniciar — a role deve estar OU presente OU o RPC ter falhado (nunca sucesso fantasma). Injetar ENOSPC (quota) e verificar retcode de erro no RPC."
