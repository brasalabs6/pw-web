# Auditoria de Infraestrutura e Deploy — server153 (READ-ONLY)

**Repo:** brasalabs6/pw-web, branch `feat/pw-source-1.5.3-ci-build`
**Data:** 2026-10-28
**Escopo:** `scripts/`, `cloudflare/`, `.github/workflows/`, `src/server153/cnet/{begingame,killgame,startgame,stopgame}`
**Método:** leitura direta de arquivos + grep dirigido. Nenhuma escrita no repositório.

---

## Findings

### [High|Confirmed] Chaves de sessão reais commitadas em confs trackeados e hardcoded em scripts
- Type: security
- Evidence:
  - `src/server153/cnet/gdeliveryd/gamesys.conf:61-64` — chaves reais de sessão (iseckey/oseckey/shared_key) ativas (não comentadas) commitadas e trackeadas (`git ls-files` confirma).
  - `src/server153/cnet/gacd/gamesys.conf:52-54` — mesmas chaves reais ativas no [GACControlServer].
  - `src/server153/cnet/gacd/io.conf:50-52` — duplicata das chaves reais.
  - `scripts/boot-server153.sh:133-135` — as mesmas chaves hardcoded em `sed` de runtime.
  - `cloudflare/entrypoint.sh:64-66` — as mesmas chaves hardcoded no entrypoint do container.
  - `src/server153/cnet/gauthd/gauthd.conf:11-14` — placeholders 456/123/123456 (não são as reais, mas o boot/entrypoint sobrescrevem com as reais).
  - Contraste: `gdeliveryd/gdeliveryd.conf:64-67` usa placeholders 123/456/123456 (conf vintage diferente, não usado em runtime).
- Failure Scenario: Qualquer pessoa com acesso ao repo lê as chaves de criptografia de sessão entre daemons (delivery↔authd, gacd control). Com as chaves, é possível forjar pacotes entre daemons ou descriptografar tráfego interno se os portos forem alcançáveis. No deploy Cloudflare, `enableInternet: false` limita a exposição externa, mas as chaves permanecem no histórico do git permanentemente.
- Impact: Integridade do protocolo inter-daemon comprometida se o tráfego interno for interceptável. Violação da regra do projeto ("não expor credenciais") e da política global de segurança.
- Validation: `git log --oneline -- src/server153/cnet/gdeliveryd/gamesys.conf` para confirmar que as chaves estão no histórico; `git ls-files` confirma tracking.
- Fix Direction: Substituir as chaves por placeholders comentados nos confs commitados. Injetar as chaves reais via variáveis de ambiente ou secret manager no boot/entrypoint (ex.: `sed` lendo de `${GDELIVERY_ISECKEY}`). Rotacionar as chaves (as atuais estão no histórico e devem ser consideradas comprometidas). Adicionar `*.conf` com chaves reais ao `.gitignore` se forem geradas em runtime, ou sanitizar no commit.

### [High|Confirmed] /health do container retorna JSON estático — falhas de daemon são mascaradas
- Type: operational-risk
- Evidence:
  - `cloudflare/entrypoint.sh:21` — `echo '{"ok":true,...}' > /www/health` grava um arquivo estático que sempre responde `ok:true`.
  - `cloudflare/worker.js:77-83` — o Worker faz `port.fetch("http://container" + url.pathname)` para `/health`, recebendo o arquivo estático.
  - O health dinâmico (conta de daemons vivos) está em `/www/cgi-bin/health.sh` (entrypoint:23-39), mas o Worker nunca referencia `/cgi-bin/health.sh`.
- Failure Scenario: Um ou mais daemons morrem no container. O Worker/proxy/monitoramento consulta `GET /health` → recebe `{"ok":true}` → considera tudo saudável. Nenhum alerta é gerado. O container permanece "saudável" até o inactivity timeout (30 min) ou até alguém consultar `/tcpprobe` manualmente.
- Impact: Detecção de falhas de daemon é impossível via /health. Orquestração e monitoramento externo que dependem de /health recebem falsos positivos.
- Validation: `docker run` a imagem, matar um daemon (`kill $(pidof gauthd)`) e consultar `GET /health` → ainda 200 `{"ok":true}`.
- Fix Direction: O Worker deve fazer proxy para `/cgi-bin/health.sh` (ou o entrypoint deve servir o JSON dinâmico em `/health` em vez do arquivo estático). Alternativa: o busybox httpd pode usar um CGI em `/health` via configuração de redirect, ou substituir o arquivo estático por um symlink/script que executa a contagem de daemons.

### [High|Confirmed] entrypoint.sh não aplica fixes de version e edition do glinkd — container rejeita cliente real
- Type: operational-risk
- Evidence:
  - `scripts/boot-server153.sh:90-91` — aplica `version=10503` e anexa `edition=300000917d...` ao `[GLinkServer1]` do glinkd.conf de runtime.
  - `cloudflare/entrypoint.sh:49-56` — NÃO aplica essas duas correções. O `sed` do entrypoint para glinkd.conf cobre apenas port (9001→29000) e GAntiCheatClient port (29701→29702).
  - `src/server153/cnet/glinkd/gamesys.conf:66` — conf commitado tem `version = 10204` (o cliente PWBR v145, GAME_VERSION 0x010503, rejeita versão menor com diálogo "servidor suspenso").
- Failure Scenario: Container Cloudflare sobe glinkd com `version=10204` e sem chave `edition`. Cliente PWBR v145 conecta via mediação TCP (getTcpPort(29000)), glinkd rejeita por versão incompatível. Nenhum cliente consegue entrar no mundo.
- Impact: O deploy Cloudflare é não-funcional para conexões de cliente reais. A prova de viabilidade do gateway/protocolo (P2) não pode ser demonstrada no ambiente de deploy.
- Validation: `docker run` a imagem, verificar `grep version $RUN/glinkd.conf` → ainda 10204; `grep edition $RUN/glinkd.conf` → ausente.
- Fix Direction: Replicar as linhas `sed` de boot-server153.sh:90-91 no entrypoint.sh, após o sed de port do glinkd.

### [Medium|Confirmed] /probe via Worker sempre retorna 503 "starting" — endpoint quebrado
- Type: operational-risk
- Evidence:
  - `cloudflare/worker.js:77-83` — para qualquer path que não seja `/tcpprobe`, faz `port.fetch("http://container" + url.pathname)`. `/probe` é enviado ao busybox httpd do container.
  - `cloudflare/entrypoint.sh` — busybox httpd serve `/www/health` e `/www/cgi-bin/health.sh`. Não existe arquivo ou CGI em `/probe`. busybox httpd retorna 404.
  - `cloudflare/worker.js:78` — `if (!response.ok) throw` → 404 não é ok → retry 3x com 2s → 503 `{"starting":true}`.
  - `cloudflare/health-server.py:51-52` — o `health-server.py` (não usado em produção) tem `/probe`, mas o entrypoint usa busybox httpd, não python.
- Failure Scenario: Qualquer chamada a `GET /probe` no Worker retorna 503 com `starting:true` mesmo com o container totalmente saudável. Monitoramento que usa /probe reporta o container como "iniciando" permanentemente.
- Impact: Sinal falso de cold-start permanente. Degrada confiança em monitoramento automatizado.
- Validation: `curl https://<worker-url>/probe` → 503 `{"starting":true,"detail":"container returned 404"}`.
- Fix Direction: Ou (a) adicionar um CGI `/www/cgi-bin/probe.sh` que faz `nc -z 127.0.0.1 29000` e retorna JSON, e o Worker faz proxy para `/cgi-bin/probe.sh`; ou (b) remover /probe do Worker e documentar que /tcpprobe é o probe de TCP (já funcional via getTcpPort(29000).connect()).

### [Medium|Confirmed] stop_all envia SIGTERM (não SIGUSR1) para daemons WDB — risco de corrupção de storage
- Type: data-consistency
- Evidence:
  - `scripts/boot-server153.sh:49-56` — `stop_all()` envia `kill` (SIGTERM) para todos os daemons, incluindo gamedbd e uniquenamed.
  - `src/server153/cnet/killgame` — upstream usa `killall -SIGUSR1 gamedbd.wdb` e `killall -SIGUSR1 uniquenamed.wdb` (graceful: checkpoint + flush).
  - `src/server153/cnet/stopgame` — upstream usa `killall -SIGUSR1 gamedbd` (parcial).
  - `src/server153/share/common/thread.cpp:110-117` — `sigusr1_handler` chama `OnQuit()` → `stateQuitAtOnce` → `WakeUp()` (shutdown limpo). SIGTERM não está no `sigwait` set (thread.cpp:168-177) → default disposition = terminate imediato, sem checkpoint.
  - `scripts/boot-server153.sh:139` (comentário) — "the WDB engine has no CORRUPT-state recovery, so state left by a hard kill can permanently break gamedbd startup".
- Failure Scenario: `boot-server153.sh --stop` envia SIGTERM para gamedbd. O processo termina sem checkpoint do WDB. No próximo boot, gamedbd encontra storage em estado inconsistente e segfaulta ou recusa iniciar (o comentário do próprio script reconhece isso). Requer `--fresh` (wipe de storage) para recuperar.
- Impact: Perda de dados de persistência (contas, itens, factions). Necessidade de wipe completo para recuperar. Em deploy Cloudflare, se o storage persiste entre restarts, o container fica permanentemente quebrado.
- Validation: Inspecionar `kill -l` para confirmar que SIGTERM (15) ≠ SIGUSR1 (10); comparar `stop_all` com `killgame` upstream.
- Fix Direction: `stop_all` deve enviar SIGUSR1 para gamedbd e uniquenamed (com timeout + SIGKILL de fallback), e SIGTERM/SIGKILL para os demais. Ex.: `kill -USR1 $pid; sleep 5; kill -0 $pid && kill -9 $pid`.

### [Medium|Confirmed] Watchdog sem backoff/jitter — crash loop vira thrashing permanente
- Type: operational-risk
- Evidence:
  - `scripts/watchdog-server153.sh:60-67` — loop infinito com `sleep $INTERVAL` (10s fixo). Se um daemon morre, `restart_daemon` é chamada imediatamente, sem nenhum backoff exponencial ou jitter.
  - Se gamedbd crasha na inicialização (ex.: WDB corrupto do finding anterior), o watchdog reinicia a cada 10s para sempre, gerando logs, CPU e I/O sem progresso.
- Failure Scenario: WDB corrupto → gamedbd crasha em 2s → watchdog reinicia → crasha em 2s → ciclo infinito a cada 10s. Logs crescem, CPU é desperdiçada, ninguém é alertado.
- Impact: Degradação de recursos; mascaramento de falha persistente como "blip" temporário.
- Validation: Corromper um arquivo WDB deliberadamente (em ambiente de teste) e observar o watchdog por 60s.
- Fix Direction: Adicionar contador de restarts por daemon com backoff exponencial (10s → 20s → 40s → ... → max 300s) e alerta após N falhas consecutivas (ex.: log CRITICAL, exit ou notificação).

### [Medium|Confirmed] Race entre watchdog e boot manual — daemon reiniciado durante boot
- Type: operational-risk
- Evidence:
  - `scripts/watchdog-server153.sh:60-67` — roda continuamente, checa `/proc/*/comm` a cada 10s. Não há lock file, PID file ou coordenação com `boot-server153.sh`.
  - `scripts/boot-server153.sh:200-210` — boot inicia daemons sequencialmente com `nohup`. Durante a janela entre `stop_all` e o start de cada daemon, o watchdog pode detectar o daemon como "morto" e reiniciá-lo.
  - Se `boot-server153.sh --stop` é executado enquanto o watchdog roda, o watchdog reinicia todos os daemons imediatamente após o sleep.
- Failure Scenario: Operador executa `boot --stop` para manutenção. Watchdog reinicia tudo 10s depois. Manutenção é interrompida. Ou: boot está iniciando gamedbd (ainda não listening), watchdog detecta gamedbd "morto" e inicia uma segunda instância.
- Impact: Comportamento não-determinístico durante operações de manutenção. Duas instâncias do mesmo daemon podem competir por portos/storage.
- Validation: Iniciar watchdog, executar `boot --stop`, observar se daemons voltam em <10s.
- Fix Direction: Watchdog deve adquirir um lock (flock ou PID file) que o boot também respeita. Ou: o watchdog verifica a existência de um arquivo sentinel (`$RUN/.booting`) e pula o ciclo se presente.

### [Medium|Likely] Worker /tcpprobe sem autenticação nem rate limit — DoS por exaustão de conexões
- Type: security
- Evidence:
  - `cloudflare/worker.js:59-67` — `GET /tcpprobe` abre `container.getTcpPort(29000).connect("127.0.0.1:29000")` sem nenhuma verificação de identidade, token ou rate limit.
  - `cloudflare/worker.js:118-122` — `fetch` default export não tem auth; qualquer cliente HTTP pode chamar /tcpprobe.
  - `src/server153/cnet/glinkd/gamesys.conf:58` — `listen_backlog = 10` (backlog pequeno).
- Failure Scenario: Atacante faz N requisições concorrentes a `/tcpprobe`. Cada uma abre uma conexão TCP a glinkd:29000 dentro do container. Com backlog=10, glinkd satura rapidamente. Clientes legítimos não conseguem conectar.
- Impact: DoS no servidor de jogo via endpoint público do Worker.
- Validation: `ab -n 100 -c 20 https://<worker-url>/tcpprobe` e observar se glinkd rejeita conexões.
- Fix Direction: Adicionar autenticação (header/token compartilhado) ou rate limiting no Worker (ex.: Cloudflare Rate Limiting Rules, ou contador no DO com `ctx.storage`). Considerar mover /tcpprobe para um path não público ou protegido por `Authorization`.

### [Medium|Confirmed] Logs crescem sem limit no container — risco de exaustão de disco
- Type: operational-risk
- Evidence:
  - `cloudflare/entrypoint.sh:79-80` — `start()` usa `>> /opt/state/logs/$(basename "$dir").log` (append, sem rotação).
  - `scripts/watchdog-server153.sh:49,53` — `restart_daemon` usa `>> "$LOGS/$d.log"` (append). `watchdog.log` também append.
  - Nenhum logrotate, truncamento ou limite de tamanho em nenhum dos três scripts.
  - PW daemons produzem log contínuo (trace, chat, err).
- Failure Scenario: Container Cloudflare roda por dias/semanas. Logs crescem até preencher o storage efêmero do container. Daemons começam a falhar ao escrever logs (ENOSPC) ou o container é reiniciado pela plataforma.
- Impact: Degradção gradual; falha catastrófica de daemon por ENOSPC; reinício não-gracioso.
- Validation: `docker run` a imagem, deixar rodar por 1h com tráfego, `du -sh /opt/state/logs/`.
- Fix Direction: Adicionar rotação de logs no entrypoint (logrotate com cron, ou um wrapper que trunca logs > N MB). Ou usar stdout do daemon e deixar a plataforma coletar logs. Ou `tail -c 10M` periódico.

### [Low|Confirmed] Container executa tudo como root — `export USER=builder` é apenas env var
- Type: security
- Evidence:
  - `cloudflare/Dockerfile` — sem diretiva `USER`. Base `debian:12` default = root. `ENTRYPOINT ["/opt/entrypoint.sh"]` roda como root.
  - `cloudflare/entrypoint.sh:13` — `export USER=builder` é uma variável de ambiente, não `su`/`gosu`/`runuser`. Os daemons e o busybox httpd executam como root.
  - CGI `health.sh` (entrypoint:23-39) executa como root via busybox httpd.
- Failure Scenario: Se um daemon ou o httpd/CGI tiver uma vulnerabilidade de execução de comando, o atacante obtém root dentro do container. Escalada para o host depende do runtime de container da Cloudflare.
- Impact: Superfície de ataque ampliada (root vs usuário não-privilegiado).
- Validation: `docker exec pw153-cf-test id` → `uid=0(root)`.
- Fix Direction: Criar usuário `builder` no Dockerfile (`RUN useradd -r builder`), `RUN chown -R builder /opt /www`, e `USER builder` antes do ENTRYPOINT. Ou usar `gosu`/`runuser` no entrypoint. Os daemons não precisam de root (portos >1024).

### [Low|Confirmed] Worker faz proxy de paths arbitrários para o container — sem whitelist
- Type: security
- Evidence:
  - `cloudflare/worker.js:77` — `port.fetch("http://container" + url.pathname)` — qualquer pathname é encaminhado ao busybox httpd do container.
  - O busybox httpd serve `/www/` e executa `/www/cgi-bin/*`. Se futuramente houver arquivos sensíveis em `/www`, seriam expostos.
- Failure Scenario: Um arquivo sensível é acidentalmente colocado em `/www/` (ex.: conf com chaves). O Worker o serve a qualquer chamador.
- Impact: Exposição acidental de conteúdo interno do container.
- Validation: `curl https://<worker-url>/health` → proxy funciona; `curl https://<worker-url>/cgi-bin/health.sh` → também funciona.
- Fix Direction: Whitelist de paths no Worker: só permitir `/health`, `/probe`, `/tcpprobe`. Rejeitar todo o resto com 404.

### [Low|Confirmed] gs-server153.sh: python3 ausente + .old ausente → gs inicia com mall não convertido
- Type: operational-risk
- Evidence:
  - `scripts/gs-server153.sh:55-88` — se `python3` está ausente, o `else` (linha 88-89) apenas imprime WARNING e continua.
  - O gs inicia (linha 92-94) e lê os arquivos gshopsev.data/gshopsev1.data no layout 196-byte (o fonte espera 220-byte com `buy_times_limit`).
  - O script tem `set -uo pipefail` (sem `-e`), então `sys.exit(1)` do conversor python não para o script.
- Failure Scenario: Container/runtime sem python3 e sem .old (primeiro staging). gs lê 196 bytes como 220 → leitura desalinhada → crash ou dados de mall corrompidos em runtime.
- Impact: gs crasha ou mall funciona incorretamente (itens compráveis com dados errados).
- Validation: `unset PYTHON3; rm -f $PKGGAMED/config/gshopsev.data.old; scripts/gs-server153.sh` e observar se gs inicia.
- Fix Direction: Se python3 ausente e .old ausente, `exit 1` em vez de continuar. Ou: fazer o conversor em shell puro (awk/dd) sem depender de python3. Ou: garantir python3 no container (já tem, mas o guard existe para o runtime host).

### [Low|Confirmed] libxml2.so.2 stale permanece na imagem (não é carregado, mas é peso morto)
- Type: maintainability-risk
- Evidence:
  - `scripts/cloudflare-server153.sh:28-29` — `find` deleta `*.so` mas `libxml2.so.2` termina em `.so.2`, não `.so`. O pattern não o captura.
  - `src/server153/cnet/gacd/libxml2.so.2` — 1MB, git-tracked, copiado para o staging.
  - `readelf -d gacd` mostra `NEEDED libxml2.so.2` mas sem RUNPATH/RPATH → o loader usa a libxml2 do sistema (i386), não a do diretório. A stale não é carregada em runtime.
- Failure Scenario: A stale libxml2.so.2 (upstream, versão antiga com CVEs conhecidos) permanece na imagem. Não é carregada, mas análise de vulnerabilidade de imagem a sinaliza. Ocupa ~1MB.
- Impact: Falsos positivos em scan de imagem; peso desnecessário.
- Validation: `docker exec pw153-cf-test ls -la /opt/server153/cnet/gacd/libxml2.so.2`.
- Fix Direction: Adicionar `-o -name '*.so.*'` ao find de stage(), ou `rm -f $CTX/server153/cnet/gacd/libxml2.so.2` explícito.

---

## Otimizações e melhorias

### Imagem: strip antes do COPY para reduzir contexto de build
- **Atual:** `cloudflare-server153.sh stage()` copia binários unstripped (gdeliveryd 67MB, gs 58MB) para o contexto. O strip acontece no Dockerfile (`RUN strip ... 2>/dev/null || true`). O contexto de build é 147MB.
- **Otimização:** Fazer `strip` no stage() antes do `cp` para o contexto. Reduz o contexto de 147MB → ~60MB (gdeliveryd 67→7MB, gs 58→9MB, outros similares). Build de imagem mais rápido, push de contexto menor.
- **Risco:** strip no host requer `strip` i386 disponível. No container de build (Debian 12), `binutils` tem strip multilib.

### Imagem: multistage build
- **Atual:** Single `FROM debian:12`. O contexto inclui .o, .bak, CVS, logs, etc. (filtrados parcialmente).
- **Otimização:** Multistage: stage 1 = build (debian:12 + toolchain), stage 2 = runtime (debian:12-slim + apenas libs i386 + binários stripped). Reduz imagem final de ~480MB para ~200-250MB.
- **Benefício:** Menor cold start (pull de imagem menor), menos superfície de ataque.

### CI: cache de apt e ccache
- **Atual:** `server153-build.yml` instala o toolchain i386 do zero a cada run (~2-3 min de apt). Sem ccache. Build total a cada push.
- **Otimização:** (a) Pré-construir uma imagem base com o toolchain e usá-la como `container:`. (b) Usar `actions/cache` para `~/.ccache` com `CC="ccache g++"`. (c) Cachear o diretório `dist/server153/` entre runs (incremental make).
- **Benefício:** Build de ~15min → ~5min.

### CI: artifact de 120MB a cada push
- **Atual:** `server153-build.yml:55-60` — `if: always()` faz upload do tar.gz (~120MB, binários unstripped) a cada push/PR.
- **Otimização:** (a) Strip binários antes do `tar` (reduz para ~40MB). (b) `if: success()` em vez de `always()` (não subir artifact de build falho). (c) `retention-days: 7` em vez do default 90. (d) Subir apenas em push para `main`, não em PR.
- **Benefício:** Menos storage de artifacts; builds de PR mais rápidos.

### CI: jobs=2 conservador
- **Atual:** `build-server153.sh` default `JOBS=2`; workflow passa `--jobs 2`.
- **Otimização:** Usar `--jobs $(nproc)` ou pelo menos 4. O runner ubuntu-latest tem 4 vCPUs. O build é CPU-bound (C++ -m32). O risco de OOM é baixo (binários individuais <100MB).
- **Benefício:** Build ~2x mais rápido.

### Cold start: entrypoint boot time
- **Atual:** 8 daemons iniciados sequencialmente sem wait entre eles. O Worker tenta 3 fetches com 2s de espera (6s total), depois 503+Retry-After.
- **Otimização:** (a) Iniciar daemons em paralelo onde não há dependência direta (gauthd, logservice, gacd podem iniciar juntos). (b) O Worker poderia aumentar tentativas ou usar polling com backoff. (c) Pré-aquecer o container no deploy (chamar /health após deploy).

### Logs: rotação no container
- **Atual:** Append sem limit (ver finding Medium acima).
- **Otimização:** `busybox` tem `logrotate`? Não. Usar um cron simples no entrypoint: `while true; do sleep 3600; for f in /opt/state/logs/*.log; do tail -c 5M "$f" > "$f.tmp" && mv "$f.tmp" "$f"; done; done &`. Ou redirecionar daemon stdout para `multilog` (se disponível) ou um wrapper de tamanho.

---

## Fatos verificados vs inferências

### Verificados (confirmados por leitura direta ou comando)
- Chaves reais commitadas em `gdeliveryd/gamesys.conf:61-64`, `gacd/gamesys.conf:52-54`, `gacd/io.conf:50-52` — `git ls-files` confirma tracking.
- `gauthd.conf:11-14` tem placeholders 456/123/123456; boot e entrypoint sobrescrevem com as chaves reais.
- `/health` do container é arquivo estático `{"ok":true}` (entrypoint:21); o dinâmico está em `/cgi-bin/health.sh` não referenciado pelo Worker.
- entrypoint.sh não aplica `version=10503` nem `edition` no glinkd.conf (comparação linha-a-linha com boot-server153.sh:90-91).
- `/probe` não existe no busybox httpd do container → 404 → Worker 503.
- `stop_all` usa SIGTERM (kill sem sinal); upstream `killgame` usa SIGUSR1 para gamedbd/uniquenamed.
- `thread.cpp:110-117` — SIGUSR1 → OnQuit/stateQuitAtOnce; SIGTERM não está no sigwait set.
- Watchdog: sleep fixo 10s, sem backoff, sem lock/PID file.
- Dockerfile sem `USER`; entrypoint `export USER=builder` é env var.
- `libxml2.so.2` (1MB) no staging; `find -name '*.so'` não captura `.so.2`; gacd sem RUNPATH.
- Binários unstripped: gdeliveryd 67MB→7MB stripped, gs 58MB→9MB stripped.
- CI: sem cache de apt/ccache; artifact `if: always()`; `--jobs 2`.
- Logs: entrypoint `>>` (append), sem rotação.
- `gs-server153.sh` sem `set -e`; python3 ausente + .old ausente → warning e continua.

### Inferências (não confirmadas por execução, mas suportadas por evidência)
- Impacto do SIGTERM em gamedbd (corrupção WDB) — inferido do comentário do próprio script (boot:139) + análise do handler de sinais; não reproduzido.
- DoS via /tcpprobe (exaustão de backlog=10) — inferido do código; não testado com carga.
- Tamanho final da imagem 480MB — citado no enunciado; não medido diretamente (estimado: base ~120MB + libs i386 ~80MB + staging stripped ~90MB + overhead).
- Persistência de storage entre restarts do container Cloudflare — não confirmado; depende da plataforma.
- `health-server.py` é leftover não usado — inferido (entrypoint usa busybox httpd, não python); não há referência a health-server.py no entrypoint ou wrangler.
