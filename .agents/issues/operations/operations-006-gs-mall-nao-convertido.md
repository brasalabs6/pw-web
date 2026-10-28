id: operations-006
title: gs-server153.sh inicia o gs com mall não convertido quando python3 e .old estão ausentes (warning e continua)
category: operations
type: bug
status: OPEN
severity: MEDIUM
owner: server153-ops (scripts)
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "scripts/gs-server153.sh:85-89 — else do guard python3 só imprime WARNING e segue; sem set -e, sys.exit(1) do conversor não para o script; o gs lê gshopsev 196-byte como 220-byte"
impact: "Primeiro staging em ambiente sem python3 (e sem .old): gs inicia com leitura desalinhada do mall → crash ou itens compráveis com dados corrompidos. Falha silenciosa de um passo obrigatório do boot."
proposed_action: "Se python3 ausente E .old ausente → exit 1 (falha explícita). Ou converter em shell puro (awk/dd). O guard existe para o host; o container tem python3 — tornar a falha barulhenta."
validation_notes: "unset python3 + rm .old em ambiente de teste → script deve falhar com erro claro, não subir o gs."
