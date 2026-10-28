id: security-004
title: Chaves de sessão reais commitadas no git (gdeliveryd/gacd confs, boot script, entrypoint Cloudflare)
category: security
type: bug
status: OPEN
severity: HIGH
owner: server153-ops
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/cnet/gdeliveryd/gamesys.conf:61-64 — iseckey/oseckey/shared_key reais (ativas, não comentadas) trackeadas (git ls-files confirma)"
  - "src/server153/cnet/gacd/gamesys.conf:52-54 e src/server153/cnet/gacd/io.conf:50-52 — mesmas chaves reais ativas"
  - "scripts/boot-server153.sh:133-135 e cloudflare/entrypoint.sh:64-66 — as mesmas chaves hardcoded em sed de runtime"
  - "src/server153/cnet/gauthd/gauthd.conf:11-14 — placeholders 456/123/123456 (o boot/entrypoint sobrescrevem com as reais)"
impact: "Quem tem acesso ao repo lê as chaves de criptografia de sessão entre daemons (delivery↔authd, gacd control) — pode forjar/decifrar tráfego interno se alcançar as portas. As chaves atuais estão no histórico do git para sempre (comprometidas por definição). Viola a regra do projeto de não expor credenciais."
proposed_action: "1) Rotacionar as chaves (as atuais são comprometidas). 2) Substituir por placeholders comentados nos confs commitados. 3) Injetar via variável de ambiente/secret no boot e entrypoint (sed lendo ${GDELIVERY_ISECKEY} etc.), com fallback que gera chaves aleatórias locais. 4) Nunca commitar confs de runtime gerados (já gitignored — manter)."
validation_notes: "git log -p nos confs não deve conter chaves reais após o fix (o histórico antigo permanece — documentar a rotação); grep de chaves no working tree deve achar só placeholders."
