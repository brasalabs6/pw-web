# ADR-0005 — fonte portada 1.5.3 e entrega Cloudflare por conteúdo

Data: 2026-10-25. Estado: **proposta técnica pronta para revisão; infraestrutura não provisionada**.
Mandato Cloudflare/Wrangler e árvore modificada 1.5.3: user-input. Regras de IP:
[LEGAL](../LEGAL.md). Complementa ADR-0004 sem mudar WebGL2, WSS ou o realm.

## Contexto observado

Base inspecionada: `f18301d`, posterior ao handoff do PR #15. Existem core WASM,
probe R0, testes e GitHub Actions; não existem package.json, CMake do cliente,
Wrangler, uploader de conteúdo ou workflows de deploy. O backend R0 recebeu código
no PR; esta entrega de planejamento não o revisa nem presume seus resultados.
E15/e17 já planejam streaming/deploy, mas suas interfaces eram genéricas.

## Decisão proposta

1. `references/client-153` permanece upstream congelado. `src/client153/` será a
   árvore rastreada de arquivos efetivamente portados, com caminhos relativos,
   copyrights e proveniência. Nada de compilar automaticamente a árvore Windows.
2. Emscripten compila o cliente no CI/laboratório; o navegador executa o WASM.
   Cloudflare distribui os arquivos. Não compilar o jogo em requisições Worker.
3. Um Worker `app` com Static Assets hospeda o shell e resolve bootstrap da versão.
   R2 armazena binários grandes, manifests e chunks; CDN via domínio próprio.
   Nenhum pacote lógico vira Worker. Gateway WSS será uma fronteira separada de
   e10 quando o contrato do realm estiver confirmado; servidor de jogo mantém
   sua hospedagem/autoridade e ainda não tem implantação Cloudflare comprovada.
4. Build único, artefatos imutáveis por SHA-256, promoção dos mesmos bytes entre
   preview/staging/produção. Binários de debug e fontes nunca vão ao diretório CDN.
5. Conteúdo primeiro; manifest de release depois; Worker só aponta à release
   completa depois de verificar objetos e executar smoke. Isso não é transação
   atômica entre R2, Workers e sessões antigas.
6. Loading por conjuntos de dependências: bootstrap → login → personagem → célula
   de entrada → células vizinhas/instâncias sob demanda. Um WASM principal no
   início; dividir código C++ via dynamic linking exige medição e contrato ABI.
7. Previews nativos de Wrangler com recursos de teste explícitos. Version URLs
   não substituem isolamento. Assets liberados públicos no CDN durante MVP e
   desenvolvimento, conforme decisão explícita do operador; r2.dev desligado.

## Alternativas e consequências

- Pages também serviria um shell, mas adotamos Workers Static Assets para manter
  a mesma ferramenta/configuração de bootstrap e futuras rotas. Sem migração de
  hosting existente, porque não há deploy Cloudflare rastreado neste projeto.
- Um `.data`/PCK gigante anteciparia download e memória de mapas não visitados;
  não será a distribuição web. Arquivos originais continuam como inputs do cooker.
- Uma cópia integral da source com `.lib/.obj/.pch` traria dependências ocultas;
  importamos apenas arquivos de código necessários, em commits auditáveis.
- R2 direto com domínio/CDN é menor e evita Worker por download. Se distribuição
  exigir login individual, será necessário gateway de assets com autorização
  antes de cache; essa ramificação fica fora do MVP e depende de contrato de identidade.
- Custos dependem de bytes armazenados, número de objetos/requests, misses de CDN,
  leituras R2 e invocações Worker. Sem estimativa de jogadores, não há promessa de
  custo ou escala. Chunks pequenos demais aumentam requests e metadados.

## Contratos e prova exigida

[Manifest de conteúdo](../../.agents/epics/e15-assets-streaming-perf-v1/contracts/content-manifest.md),
[streaming](../../.agents/epics/e15-assets-streaming-perf-v1/streaming.md),
[release](../../.agents/epics/e17-security-deployment-v1/contracts/release-manifest.md),
[build](../../.agents/epics/e17-security-deployment-v1/build-pipeline.md),
[Cloudflare](../../.agents/epics/e17-security-deployment-v1/cloudflare.md) e
[tarefas](../../.agents/epics/e17-security-deployment-v1/delivery-tasks.md).

Aceite inicial: checkout limpo → fixture sintética → build → upload em sandbox →
preview → reload em cache → rollback, com ausência de credencial no build e
negativos de hash, conteúdo ausente, acesso ao bucket privado e mistura de versões. Essa prova
não depende de ter um cliente completo e não pode ser anunciada como MMO.

## Riscos e decisões restantes

[HIGH][Confirmed] Faltam conta/zone/domínios/tokens; não houve deploy nesta entrega. A decisão
do operador libera CDN público para assets no MVP/desenvolvimento, sem liberar
fontes, debug, inputs do cooker ou dados privados. Login para downloads pós-MVP
é evolução ainda não solicitada, sem impacto no formato de conteúdo. [HIGH][Confirmed]
Compatibilidade de assets/realm 1.5.3 permanece sem prova. [MEDIUM][Inferred]
Normalização de paths/cooker pode alterar semântica do legado: colisões são erro,
não conversão silenciosa. [MEDIUM][Unverified] Toolchain/cooker final e budgets
precisam de benchmark. O produtor de cada contrato abaixo é `pw-web`, sem mudar
contratos do servidor externo.
