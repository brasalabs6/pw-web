id: security-010
title: Dump [decode] de todos os pacotes em stderr sem #ifdef — débito pw-web (info leak + custo por pacote na thread de poll)
category: security
type: bug
status: OPEN
severity: MEDIUM
owner: server153-framework (share/)
evidence_quality: confirmed
discovered_by: @code-audit
discovered_during: code-audit-server153-2026-10-28
discovered_at: 2026-10-28
evidence:
  - "src/server153/share/io/protocol.h:536-545 — bloco 'pw-web debug: dump ALL protocol payloads' com fprintf(stderr, \"[decode] type=%d size=%d bytes:\") + loop de 64 bytes hex, executado para CADA protocolo decodificado, sem #ifdef nem log level"
impact: "Rodando em produção: 3+ fprintf por pacote na thread de poll (stderr unbuffered) — gargalo de I/O e vazamento do conteúdo de pacotes (dados de login, chat, tokens) para o log. Foi instrumentação de diagnóstico do wire v145 que ficou no código."
proposed_action: "Remover o bloco ou proteger com #ifdef PW_WEB_DEBUG (não definido em builds normais); se precisar de diagnóstico futuro, usar Log::log(LOG_DEBUG, ...) com dump limitado a tipos específicos."
validation_notes: "grep '[decode]' nos binários/fontes do build de produção deve dar zero; um login de teste não deve gerar dump de payload no stderr."
