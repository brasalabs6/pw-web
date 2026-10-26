# PW Web — plano de build, CDN, streaming e deploy

Estado em 2026-10-25: **arquitetura/tarefas especificadas; pipeline de deploy não
implementada nem implantada**. Cloudflare+Wrangler e conteúdo público no
MVP/desenvolvimento são decisões do operador. Fontes/inputs/debug/contas não são
públicos. O game completo ainda depende dos gates de [VALIDATION](VALIDATION.md).

## Como o jogador recebe o jogo

Shell pequeno abre primeiro. O cliente baixa o runtime WASM e a UI mínima, depois
o personagem e apenas os dados da área de entrada. Instâncias têm entry separado
do restante do mapa; próximos setores entram por prefetch. Cache por hash evita
rebaixar os mesmos bytes em cada sessão e update só traz objetos alterados.
Uma transição sem memória suficiente tem tela de loading: não prometer ausência
de loading antes de medir memória, decode e tamanho real.

```text
fonte 1.5.3 fixada → src/client153 adaptada + módulos engine
                                     ↓
                    CMake/Emscripten no GitHub Actions
                                     ↓
                 runtime WASM + shell (sem assets embutidos)
inputs autorizados → cooker → bundles/mapas/instâncias → chunks SHA-256
                                     ↓
                 validar + assinar release + upload R2
                                     ↓
          Wrangler Preview → smoke → staging → aprovação → produção
                                     ↓
          browser: shell → runtime → entry → streaming/cache
```

## O que já estava previsto e o que foi detalhado

E03/e04 preveem plataforma/runtime, e05 formatos, e15 streaming/cache/performance,
e17 infraestrutura/release. Agora os contratos dizem exatamente como separar,
versionar e promover esses outputs; não são implementação concluída.

| Pergunta | Decisão e fonte canônica |
| --- | --- |
| Onde mexemos na source? | árvore rastreada [src/client153](../src/client153/README.md), preservando referência e proveniência |
| Quais pacotes de código? | targets C++ e tooling em [build-pipeline](../.agents/epics/e17-security-deployment-v1/build-pipeline.md) |
| Quais downloads? | shell/runtime/UI/locale/common/avatar/célula/entry de instância/áudio/optional em [streaming](../.agents/epics/e15-assets-streaming-perf-v1/streaming.md) |
| Como empacotar/validar? | [content-manifest](../.agents/epics/e15-assets-streaming-perf-v1/contracts/content-manifest.md): objetos pequenos, hashes, DAG e spans |
| Como atualizar sem misturar versões? | [release-manifest](../.agents/epics/e17-security-deployment-v1/contracts/release-manifest.md): pins, assinatura, publicação/rollback |
| Onde hospedar? | [Cloudflare](../.agents/epics/e17-security-deployment-v1/cloudflare.md): app Worker/Static Assets, R2/CDN, ambientes e Wrangler |
| O que implementar primeiro? | [24 tasks CF-01–24](../.agents/epics/e17-security-deployment-v1/delivery-tasks.md), em cortes independentes |

## Regras que evitam desperdício

- O browser executa o WASM. Cloudflare distribui; compilação pesada fica no CI.
- Primeiro um WASM principal com targets C++ bem separados; dividir assets por
  instância não exige dividir C++ em vários WASMs. Dynamic linking é decisão medida.
- Nada de baixar todo PCK, `.data` monolítico ou todas as texturas antes de jogar.
- Blocos até 4 MiB e microbundles até 1 MiB são **parâmetros iniciais propostos**,
  não medições do jogo. O total inicial real só virá com runtime+entry homologados.
- Não recookar assets em PR só de código. Não recompilar na promoção de ambiente.
- Normalizar paths e deduplicar por hash, sem mudar precisão/qualidade por surpresa.
- Publicar somente diretórios allowlisted. Fonte, credenciais e símbolos privados
  não chegam ao bucket de distribuição.
- Sem service worker inicialmente: o cache versionado de assets evita criar duas
  autoridades de atualização. Adicionar service worker exige protocolo de upgrade.

## Primeiro resultado verificável

Fixture sintética pequena → build diagnóstico → pacote validado → R2 sandbox →
Preview público → Chrome cold/warm → negativo de objeto adulterado → rollback.
Essa fatia comprova infraestrutura real sem depender do jogo inteiro. Em seguida,
substituir inputs sintéticos por assets/cliente portados com os mesmos contratos.

Não há conta/zone/domínios/credenciais confirmados nesta sessão; deploy remoto
permanece dependente desses inputs. Hospedagem/compatibilidade do servidor
MMORPG é uma trilha de e10/e11; a escolha de CDN não comprova backend jogável.
As tarefas não alteram a implementação R0 que o ChatGPT Web está entregando.

## Evidência, fontes e limites atuais

Base lida: `f18301d` do PR #15, com código R0 novo ainda não revisado por esta
entrega. Nenhum workflow/config de produção foi ativado. Configurações e comandos
Wrangler nos documentos são exemplos de implementação futura.

Documentação primária consultada em 2026-10-25:

- [Workers Static Assets](https://developers.cloudflare.com/workers/static-assets/)
  e [limites](https://developers.cloudflare.com/workers/platform/limits/): limite
  individual atual de 25 MiB ajuda a justificar R2 para binários/conteúdo grandes;
  o WASM do navegador não está sujeito à RAM do isolate Worker.
- [R2 public buckets](https://developers.cloudflare.com/r2/buckets/public-buckets/):
  domínio próprio para CDN; r2.dev é endpoint de desenvolvimento, não a entrega alvo.
- [CORS de R2](https://developers.cloudflare.com/r2/buckets/cors/) e
  [API R2](https://developers.cloudflare.com/r2/api/workers/workers-api-reference/):
  ETag/conditional/range têm semântica própria; não equivalem a SHA-256 do cliente.
- [Previews](https://developers.cloudflare.com/workers/previews/),
  [configuração](https://developers.cloudflare.com/workers/previews/configuration/),
  [recursos](https://developers.cloudflare.com/workers/previews/resources/) e
  [Version URLs](https://developers.cloudflare.com/workers/versions-and-deployments/version-urls/):
  isolamento exige conferir os recursos/bindings, não apenas gerar outra URL.
- [Wrangler commands](https://developers.cloudflare.com/workers/wrangler/commands/workers/)
  e [GitHub Actions](https://developers.cloudflare.com/workers/ci-cd/external-cicd/github-actions/).
- [GitHub secure use](https://docs.github.com/en/actions/reference/security/secure-use):
  credencial e execução de código não confiável ficam em fronteiras separadas.
- [Emscripten otimização](https://emscripten.org/docs/optimizing/Optimizing-Code.html)
  e [dynamic linking](https://emscripten.org/docs/compiling/Dynamic-Linking.html).

Preços/limites e disponibilidade de recursos devem ser reconferidos no
provisionamento; não há benchmark, estimativa de redução em MB ou custo prometido.

## Revisão de prontidão desta especificação

Primeiro passe: corrigidos índice inicial potencialmente monolítico (catálogos
por prefix de hash carregados lazy), integridade do entry JS/WASM (SRI não cobre
fetch do glue) e exposição pública atual conforme resposta do operador. Segundo
passe: corrigida interpretação de preview_urls, mantido gate de prova real de
Previews/bindings, bloqueado GC sem leases e proibida execução/config não confiável
no publisher. Isso é revisão documental direta, não review independente ou prova
de runtime Cloudflare.

Reprodução desta entrega: `git diff --check`, validação dos links locais dos
Markdown alterados/novos e `python3 .agents/automation/epic_check.py` (20 épicos,
135 workorders, 97 automatable; sem mudança do dispatcher). Não foram executados
Wrangler, deploy, benchmark ou pipeline proposta.
