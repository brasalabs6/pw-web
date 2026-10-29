---
id: OPT-046
title: "Confinar rota /loose/ do servidor local de probe ao diretório de assets"
priority: P1
status: hypothesis
kind: preventive-hardening
first_observed: 2026-10-29
verified_against: 413717155c1ac72573f447e3c29ae801d84949c9
phase: post-browser-port
scope: pr-23
---

# OPT-046 — Contenção de paths na rota /loose/ do probe

## Observação estática

No [servidor do probe, linhas 185–192](https://github.com/brasalabs6/pw-web/blob/413717155c1ac72573f447e3c29ae801d84949c9/scripts/client153-entry-server.py#L185-L192), o nome da rota é percent-decoded e passado a `os.path.normpath(os.path.join(ELEMENT_DIR, disk_name))` sem comprovar que o caminho final permanece dentro de `ELEMENT_DIR`. Um caminho absoluto ou segmentos de diretório pai podem escapar da raiz. O ramo [`/pkg/`, linhas 193–199](https://github.com/brasalabs6/pw-web/blob/413717155c1ac72573f447e3c29ae801d84949c9/scripts/client153-entry-server.py#L193-L199), usa allowlist e é distinto. O servidor atualmente faz [bind apenas em 127.0.0.1, linhas 219–230](https://github.com/brasalabs6/pw-web/blob/413717155c1ac72573f447e3c29ae801d84949c9/scripts/client153-entry-server.py#L219-L230); não há evidência de exposição pública.

## Evidência e classificação

A auditoria reproduziu **localmente apenas a resolução de caminho**, com fixture sintético fora da pasta de assets: a combinação de `unquote`, `join` e `normpath` produziu um caminho existente fora de `ELEMENT_DIR` e satisfaria `os.path.isfile`. **Não foi feito teste HTTP contra o servidor real**, não foi demonstrada exploração remota nem acesso a arquivos sensíveis. É hardening preventivo de ferramenta de desenvolvimento, não vulnerabilidade crítica reproduzida nem blocker comprovado do jogo. Não registrar em `.agents/issues` sem evidência adicional.

## Impacto hipotético

Um consumidor que alcance o servidor de diagnóstico pode solicitar arquivos fora da raiz pretendida, conforme permissões do processo. A superfície é local e não faz parte do renderer original; não inferir impacto em FPS, RAM, VRAM, energia ou gameplay.

## Plano de validação / benchmark (fase post-browser-port)

Após o cliente original estar funcional em desktop e mobile, adicionar testes HTTP com fixture sintético para caminhos normais, `..` codificado, caminhos absolutos, separadores e symlinks; exigir 403/404 para escapes e 200/206 apenas para arquivos autorizados. Aplicar `realpath` + `commonpath`/allowlist de nomes do manifesto, mantendo bind local. Verificar que requests Range válidos não regridem; medir apenas latência e memória de requests de fixtures antes/depois, sem afirmar ganhos. Se o probe for exposto fora do loopback antes dessa fase, antecipar a contenção como requisito de segurança de publicação.

## Deduplicação

Diferente de [PW23-032](PW23-032-manifest-url-resource-trust-and-budgets.md) (política geral de URL/manifesto), [OPT-032](OPT-032-game-pck-shared-open-redecode-lifetime.md) (releitura PCK) e [OPT-044](OPT-044-browser-pck-range-index-32bit-overflow.md) (aritmética de Range). Sem mudança de código-fonte nesta auditoria.
