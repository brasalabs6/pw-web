# Onboarding — do zero até a primeira contribuição

## Pré-requisitos mínimos
- Git 2.30+, Python 3.10+ e navegador moderno com WebGL 2.
- Windows: WSL2 recomendado para scripts POSIX; macOS/Linux executam diretamente.
- Editor com C++/Python/JavaScript e Git.
- **Opcional**: Emscripten SDK (`emcc`) para compilar C++ → WASM; toolchain Windows para validar clientes originais; Docker e Rust para testes isolados de servidor.

## Primeira sessão (15 passos operacionais, sem dependências proprietárias)
1. Fazer clone do repositório SEM `--recursive`.
2. Conferir branch, `git status` e `git submodule status`.
3. Ler README, AGENTS e LEGAL (inclusive política de procedência).
4. Executar `./scripts/bootstrap.sh` (verifica ferramentas e escreve estado apenas local).
5. Executar `./scripts/verify.sh` (shell, testes Python e C++ nativo se disponível).
6. Executar `./scripts/dev.sh` e abrir http://localhost:8080.
7. Conferir diagnóstico de WebGL 2 e triângulo animado; isso NÃO é Perfect World.
8. Conferir painel de status: core WASM é opcional até compilar.
9. Instalar emsdk seguindo documentação oficial, se for desenvolver C++.
10. Executar `./scripts/build-wasm.sh` e recarregar a página.
11. Conferir `WASM: carregado` e resposta do núcleo C++.
12. Escolher um issue `P0`, definir critério de aceite mensurável.
13. Abrir branch por issue e fazer mudança mínima.
14. Executar `./scripts/verify.sh`, documentar evidência e abrir PR.
15. Não marcar milestone de Perfect World como concluído com base na demo procedimental.

## Obter referências (opcional)
```bash
git submodule update --init references/client-153
git submodule update --init references/client-152-modern
git submodule update --init references/server-rust
```
Estas árvores NÃO são parte de um build padrão. São referências externas com versões fixas. Requerem espaço considerável. Não reenvie essas árvores para outro repositório nem crie binários redistribuíveis fora do escopo da autorização registrada em `docs/LEGAL.md` (2026-10-25). A fonte 1.5.3 é candidata, não base aprovada.

## Teste Windows do cliente (trilha de pesquisa isolada)
- Usar VM/lab separado, com ferramenta correspondente ao projeto (candidato 1.5.3 requer VS2005).
- Registrar hash dos binários/fontes, SDK/arquitetura, erros de build, versão real da engine e API gráfica.
- Compilar primeiro bibliotecas, depois engine, depois cliente; registrar ordem observada, não presumida.
- Validar login e entrada no mundo antes de declarar versão compatível com servidor.
- Não encaminhar dados extraídos/cliente para CI. Guardar evidências textuais/screenshot autorizadas.

## Solução de problemas
- `WebGL 2 indisponível`: verificar browser, aceleração de hardware, driver e contexto `webgl2`.
- `WASM: não compilado`: resultado esperado antes de rodar `build-wasm.sh`.
- `emcc: command not found`: ativar emsdk (`source emsdk_env.sh`).
- `python3 não encontrado`: instalar Python; `dev.sh` requer a CLI `python3`.
- `submodule not initialized`: referências são opcionais; não rode checkout recursivo por padrão.
- `port 8080 in use`: `PORT=9000 ./scripts/dev.sh`.
- 404 no arquivo .wasm: reexecutar build, conferir `web/wasm` e reiniciar servidor se necessário.

## Ambiente e segurança
Não são necessárias contas nem credenciais para o scaffold. Não há assets Perfect World no repositório. Experimentos de servidor devem usar credenciais locais próprias, não senhas de exemplo.
