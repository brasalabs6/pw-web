# Procedência — src/server153

Cópia de trabalho versionada do código-fonte do servidor Perfect World 1.5.3
(comunitário, derivado). Não é a fonte oficial; ver classificação abaixo.

- **Origem:** https://github.com/hrace009/PW-Source-1.5.3
  (fork de `WolfsCore/PW-Source-1.5.3`; raiz da cadeia: `wladsoncedraz/Source-Server`).
- **Revisão copiada:** `7dde52fa03d394f55321931e7f0d7c1b7375fdad` (2024-11-05)
  — pós "Upgrade para 1.5.3v145 com custons" (`8aa72d3`) e anterior à
  "modernização" de 2025-01 (`d35f7de`, `cc74d74`, `1b7c9fd`), que substituiu
  Makefiles funcionais por stubs não-construíveis (referenciam diretórios
  `source/`, `src/`, `server_src/` inexistentes).
- **Upstream HEAD no momento do pin:** `c2834467cad4d78bc96851e477f6315e97907691`
  (2025-01-18) — **não** tratado como referência canônica do servidor original
  (análise do operador, 2026-10-25: o primeiro commit disponível, `f370f9c` de
  2024-10-25, já descreve um servidor adaptado; a correspondência com a versão
  oficial não é demonstrável a partir deste repositório).
- **Referência congelada idêntica:** `references/server-153-source` (gitlink
  fixado na mesma `7dde52f`). É a base de diff para nossas modificações:
  `git diff --no-index references/server-153-source src/server153`.
- **Autorização:** decisão documentada em `docs/LEGAL.md` (2026-10-25) — port
  contratado; compilar, baixar, usar, modificar e integrar estão autorizados no
  escopo do port. Distribuição segue o escopo do contrato mantido pelo operador.
- **Headers de copyright** (Archosaur/Perfect World) existem nas fontes e
  **devem ser preservados** em qualquer modificação. Nunca remover.
- **Build e CI:** `scripts/build-server153.sh` e
  `.github/workflows/server153-build.yml`; análise de completude em
  `docs/RESEARCH.md`.
