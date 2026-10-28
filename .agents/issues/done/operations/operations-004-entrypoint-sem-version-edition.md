id: operations-004
title: entrypoint.sh do container Cloudflare não aplica version=10503/edition do glinkd — deploy rejeita o cliente real ("servidor suspenso") e o health não detecta
category: operations
type: bug
status: DONE
severity: HIGH
owner: server153-deploy
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "scripts/boot-server153.sh:90-91 — aplica version=10503 e anexa edition=300000917d... ao [GLinkServer1] do conf de runtime (fix conhecido do handshake PWBR v145)"
  - "cloudflare/entrypoint.sh:49-56 — os sed do glinkd.conf cobrem SÓ portas (9001→29000, 29701→29702); SEM version/edition"
  - "src/server153/cnet/glinkd/gamesys.conf:66 — conf commitado tem version=10204 (o cliente PWBR 2555, GAME_VERSION 0x010503, rejeita versão menor)"
impact: "O container Cloudflare sobe o glinkd com version=10204 e sem edition: TODO cliente real que conectar via mediação TCP recebe 'servidor suspenso'. O deploy atual é não-funcional para clientes reais — e o health (operations-003) não detecta, porque os daemons estão vivos. A prova do gateway ADR-0006 na Cloudflare depende deste fix."
proposed_action: "RESOLVIDO: entrypoint.sh aplica version=10503 (in-place) e edition pinada ao [GLinkServer1] do conf de runtime — mesmas regras do boot-server153.sh."
validation_notes: "VALIDADO 2026-10-28: o conf de runtime do container (imagem e60f1177) carrega version=10503 + edition; o handshake GNET end-to-end com cliente real via gateway (ADR-0006) permanece o próximo milestone — o /tcpprobe prova a perna TCP."

closure_reason: "Fix aplicado no cloudflare/entrypoint.sh (sed version/edition no glinkd.conf de runtime) e deployado."
resolved_in: "commit do branch feat/pw-source-1.5.3-ci-build (deploy Cloudflare 2026-10-28, imagem digest d00dda0f)"
