# Trabalhar com referências de terceiros

Os submódulos são **referências de pesquisa**, não dependências de produção do primeiro bootstrap.

## Consultar
```bash
git submodule update --init references/client-153
git submodule update --init references/client-152-modern
git submodule update --init references/server-rust
git submodule status
```
Use clone padrão sem `--recursive`. Os fontes antigos são grandes; o uso é permitido no escopo da autorização registrada em `docs/LEGAL.md` (2026-10-25).

## Para editar na fase autorizada
Submódulos fixam um commit externo; não é correto editar em detached HEAD e esperar que alterações apareçam automaticamente. Com a autorização registrada e um fork controlado:
1. Documentar titularidade/direitos em `docs/LEGAL.md`.
2. Criar fork próprio autorizado com patches e testes.
3. Revisar remotes do submódulo e `.gitmodules`.
4. Fixar commit do fork via PR com ADR e evidências.
5. Nunca importar pastas de assets nem binários proprietários em git history.
6. Separar adapters de plataforma próprios das alterações inevitáveis no cliente.

## Política de atualizações
Comparar upstream SHA antigo vs novo; registrar alterações de API, dependências e licenciamento; rodar regressões de build nativo/WASM e integração de protocolo. Atualizar `docs/RESEARCH.md` no mesmo PR. Nunca usar `git submodule update --remote` automaticamente.

## Casos de falha
`git submodule update` não garante direitos nem builds. Se o upstream sumir, preservar metadados e histórico de commit; não criar espelho público de código fora do escopo da autorização.
