id: security-007
title: Command injection via system() com paths/nomes de arquivo concatenados no gamedbd (~20 sítios)
category: security
type: bug
status: OPEN
severity: MEDIUM
owner: server153-gamedbd
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/gamedbd/mergedb.cpp:890-895 — sprintf(indir, \"%s/%s\", src_dir, d_name de scandir) e system(\"/bin/cp -f \" + src_dir + \"/\" + dbname): nome de ARQUIVO do storage vai ao shell sem escapar"
  - "src/server153/cnet/gamedbd/mergedb2.cpp:3286-3298, accessdb.cpp:2391, clsconfig.h:37/157 — mesmo padrão system(\"/bin/rm -f \" + data_dir + ...) com paths de conf/variáveis"
  - "src/server153/cnet/gamedbd/domaincmd.cpp:202 — popen(line.c_str(), \"r\") com linha construída"
  - "src/server153/cgame/gs/global_manager.cpp:420 — system(_restart_shell) executa comando do gs.conf (RestartShell) — superfície de execução via conf"
impact: "Um nome de arquivo malicioso no diretório de storage (via backup restaurado, outro serviço ou manipulação de datadir) executa comando arbitrário como o usuário do gamedbd no merge/import. Paths de conf comprometidos idem. Ferramenta admin local, mas padrão perigoso + superfície real."
proposed_action: "Substituir system(python string concat) por exec* com argv (sem shell) ou, no mínimo, escapar/validar cada componente (rejeitar tudo que não seja [A-Za-z0-9._-]). Para o RestartShell do gs: validar contra whitelist ou documentar como conf de confiança."
validation_notes: "Criar arquivo de teste com ; e $() no nome num datadir de teste e rodar merge — nada deve executar além do cp/rm. Grep de regressão: nenhum system() com concatenação de input variável."
