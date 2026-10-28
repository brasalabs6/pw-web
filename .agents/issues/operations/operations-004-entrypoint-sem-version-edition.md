id: operations-004
title: entrypoint.sh do container Cloudflare não aplica version=10503/edition do glinkd — deploy rejeita o cliente real ("servidor suspenso") e o health não detecta
category: operations
type: bug
status: OPEN
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
proposed_action: "Replicar as duas linhas sed de boot-server153.sh:90-91 no entrypoint.sh (version in-place + edition anexada). Idealmente extrair a geração de conf para um script compartilhado entre boot local e entrypoint (uma fonte da verdade)."
validation_notes: "docker run da imagem: grep version $RUN/glinkd.conf → 10503; grep edition → presente. Probe de handshake GNET via getTcpPort deve completar challenge/response."
