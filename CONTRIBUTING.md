# Contribuindo

Antes de contribuir, leia `AGENTS.md`, `docs/ONBOARDING.md` e `docs/LEGAL.md`. Faça PRs pequenos e verificáveis.

## Branches e commits
- `feat/<escopo>`, `fix/<escopo>`, `research/<escopo>`, `docs/<escopo>`.
- Conventional Commits: `feat:`, `fix:`, `docs:`, `test:`, `chore:`.
- Não escrever na `main` sem revisão; preferir PR.

## Formato recomendado de PR
- Objetivo/hipótese.
- Alterações e arquivos afetados.
- Evidências antes/depois.
- Comandos executados e resultados.
- Riscos e limites da prova.
- Checklist jurídico/terceiros quando aplicável.

## Política de dependências e conteúdo
- Terceiros em submódulos com commit fixo, jamais copiar sem autorização.
- Assets do jogo só em diretórios locais ignorados pelo Git, e somente quando seu uso estiver autorizado.
- Nenhum binário distribuível do jogo incluído em CI/publicações até aprovação.
- Não modificar código de referência in-place como se fosse nosso código; desenvolva adapters próprios ou forks autorizados.

## Testar
```bash
./scripts/verify.sh
python3 -m unittest discover -s tests -v
```

Para reproduzir testes visuais, inclua browser/OS/GPU, resolução, console e screenshot.
