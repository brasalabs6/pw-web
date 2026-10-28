# Auditoria de Código — src/server153/share/ (READ-ONLY)

**Repo:** brasalabs6/pw-web, branch feat/pw-source-1.5.3-ci-build
**Área:** src/server153/share/ — framework GNET compartilhado
**Data:** 2026-10-28
**Método:** grep/sed dirigido, leitura seletiva de arquivos-chave
**Status:** concluído

---

## Findings

### [Critical|Confirmed] Octets::frob_size loop infinito com size=0xFFFFFFFF (hang DoS)
- Type: security
- Evidence: `src/server153/share/common/octets.h:93-98` — `frob_size` dobra `tmp` até `size <= tmp`. Se `size = 0xFFFFFFFF`, `tmp` chega a `0x80000000`, próximo shift wraps para `0`, e `0xFFFFFFFF > 0` é sempre true → loop infinito.
- Failure Scenario: Atacante envia pacote com header `CompactUINT(size)` = `0xFFFFFFFF` (5 bytes: `e0 ff ff ff ff`). `Protocol::Decode` em `protocol.h:528` chama `data.reserve(size)` → `Octets::reserve` → `Rep::reserve` → `frob_size(0xFFFFFFFF)` → loop infinito. O thread de poll trava indefinidamente; todos os I/O param.
- Impact: Qualquer cliente conectado a qualquer daemon (gs, gdeliveryd, etc.) pode travar o processo inteiro com um único pacote de 5 bytes. O servidor não responde a SIGTERM (thread em loop apertado sem cancellation point).
- Validation: Confirmar lendo `octets.h:96-99` e `protocol.h:530`. Reproduzir com unit test que chama `Octets o; o.reserve(0xFFFFFFFF)` e verifica timeout.
- Fix Direction: Limitar `frob_size` com guard `if (size > MAX_CAP) throw/return MAX_CAP;` ou usar `__builtin_clz` para calcular o próximo power-of-2 sem loop. Adicionar cap global em `Protocol::Decode` antes de `data.reserve(size)`: rejeitar se `size > is.size() - is.position()` (tamanho real do stream).

### [Critical|Confirmed] Protocol::Decode reserva buffer por size do header sem validação (memory exhaustion / crash)
- Type: security
- Evidence: `src/server153/share/io/protocol.h:528` — `data.reserve(size)` onde `size` é `CompactUINT` lido do pacote, controlado pelo atacante. `CheckPolicy` (linha 488-490) chama `InputPolicy(type, size)` → `SizePolicy(size)` cujo default retorna `true` (`protocol.h:573`). Sem cap, `reserve(0x7FFFFFFF)` → `frob_size` = `0x80000000` → `malloc(~2GB)`.
- Failure Scenario: Atacante envia pacote válido com `type` conhecido (que não sobrescreve `SizePolicy`) e `size = 0x7FFFFFFF`. O servidor tenta alocar 2GB. Em processo 32-bit (`-m32`), `malloc` pode falhar → `Rep::create` faz `new (cap) Rep` com `operator new` que retorna NULL → placement new em NULL = crash (UB). Se `malloc` succeeds, memória exausta.
- Impact: Crash ou OOM em qualquer daemon com um único pacote. Afeta todos os protocolos que usam `SizePolicy` default (a maioria).
- Validation: Confirmar lendo `protocol.h:510-535`, `octets.h:68-89` e `octets.h:108` (`operator new` sem check NULL). Grep por `SizePolicy` overrides para ver quais protocolos se protegem.
- Fix Direction: Em `Protocol::Decode`, antes de `data.reserve(size)`, validar `size <= is.size() - is.position()` (bytes realmente disponíveis no stream). Adicionar check NULL em `Rep::create` e lançar `std::bad_alloc` em vez de prosseguir com NULL.

### [High|Confirmed] Conf::find usa operator[] sob read lock — insere entradas fantasma e causa data race
- Type: bug, data-consistency
- Evidence: `src/server153/share/common/conf.h:82-84` — `value_type find(...)` toma `RDScoped l(locker)` (read lock) e retorna `confhash[section][key]`. `std::map::operator[]` é não-const: insere entrada default se a chave não existe. Isso (a) modifica o mapa sob read lock, permitindo data race com outras leituras concorrentes que também fazem `operator[]`; (b) faz o mapa crescer indefinidamente com entradas vazias a cada `find` para chave inexistente.
- Failure Scenario: Múltiplos threads chamam `Conf::find("section", "key_inexistente")` concorrentemente. Todos pegam RDScoped (read lock), todos fazem `operator[]` que insere. `std::map` não é thread-safe para inserção concorrente → corrupção de árvore RB, crash ou leitura de dados corrompidos. Mesmo sem concorrência, o mapa cresce a cada lookup de chave inexistente.
- Impact: Todos os daemons que chamam `Conf::find` (via `LoadConfig`, `Thread::Pool::Policy::LoadConfig`, `NetSession::LoadConfig`, etc.). `conf.cpp:43` chama `conf->find(section, "threads")` e `thread.cpp:91` chama `conf->find(section, "prior_strict")` — se a chave não existe, o mapa é modificado sob read lock.
- Validation: Confirmar lendo `conf.h:82-84`. Observar que `getkeys` (linha 96) tem o mesmo padrão: `section_hash h = confhash[section]` sob RDScoped.
- Fix Direction: Usar `confhash.find(section)` + `section_hash::find(key)` em vez de `operator[]`. Retornar string vazia se não encontrado. Alternativamente, usar WRScoped para `find` (mas isso serializa todas as leituras).

### [High|Confirmed] Debug fprintf em Protocol::Decode — dump de todos os payloads para stderr
- Type: performance-risk, security
- Evidence: `src/server153/share/io/protocol.h:537-546` — bloco `fprintf(stderr, "[decode] type=%d size=%d bytes:", ...)` seguido de loop que imprime até 64 bytes hex de cada protocolo decodificado. Comentário diz "pw-web debug: dump ALL protocol payloads". Executado para CADA protocolo decodificado, em produção.
- Failure Scenario: Em servidor ativo com milhares de pacotes/segundo, `fprintf` para stderr (unbuffered por default) domina o I/O. stderr pode ser redirecionado para arquivo que cresce sem parar. Conteúdo dos pacotes (potencialmente incluindo tokens de autenticação, senhas em hash, dados de personagem) é vazado para stderr.
- Impact: Degraduação massiva de performance em todos os daemons. Vazamento de dados sensíveis em log. Risco de disco cheio.
- Validation: Confirmar lendo `protocol.h:537-546`. Verificar que não há `#ifdef DEBUG` protegendo o bloco.
- Fix Direction: Remover o bloco inteiro de debug. Se necessário para diagnóstico futuro, proteger com `#ifdef PROTOCOL_DEBUG` (não definido em builds de produção).

### [High|Confirmed] Timer::Update itera observers() sem lock enquanto Attach/Detach modificam
- Type: bug
- Evidence: `src/server153/share/common/timer.h:46-49` — `Update()` faz `std::for_each(observers().begin(), observers().end(), ...)` sem nenhum lock. `Attach` (linha 28) faz `observers().push_back(o)` e `Detach` (linha 29) faz `observers().erase(...)` — também sem lock. `observers()` retorna `static Observers tmp` (um `std::vector<Observer*>`).
- Failure Scenario: Thread A chama `Timer::Update` (do main loop em `Thread::Pool::Run`, `thread.cpp:191`). Thread B chama `Timer::Attach` (de `HouseKeeper` constructor, ou de qualquer código que cria um timer observer). Se `push_back` causa realocação do vector durante a iteração de `Update`, iteradores são invalidados → use-after-free / crash.
- Impact: Crash em qualquer daemon multithread que registre timers dinamicamente. Na prática, `Attach` é chamado majoritariamente em construtores estáticos (antes das threads), então o race é latente mas real se `Attach` for chamado após startup.
- Validation: Confirmar lendo `timer.h:28-29,46-49`. Grep por `Timer::Attach` para verificar se algum chamador o invoca após threads iniciarem.
- Fix Direction: Adicionar um `Thread::Mutex` para `observers()` e travar em `Update`, `Attach`, `Detach`. Ou usar `std::atomic` com copy-and-swap.

### [Medium|Confirmed] Protocol::Manager::GetOsbytes usa WRScoped para operação de leitura
- Type: performance-risk
- Evidence: `src/server153/share/io/protocol.h:412` — `GetOsbytes` toma `Thread::RWLock::WRScoped l(locker_map)` mas apenas lê `session->GetOsbytes()`. Deveria ser `RDScoped`.
- Failure Scenario: Cada chamada a `GetOsbytes` bloqueia todas as outras leituras do mapa de sessões (porque WRScoped é exclusivo). Em servidor com muitas sessões, isso serializa desnecessariamente.
- Impact: Degradção de throughput em consultas de estatísticas de I/O.
- Validation: Confirmar lendo `protocol.h:411-414`. Comparar com `SetVerbose` (linha 408) que corretamente usa `WRScoped` porque modifica `verbose`.
- Fix Direction: Trocar `WRScoped` por `RDScoped` na linha 412.

### [Medium|Confirmed] Security::Create: assert(false) em tipo desconhecido — crash em debug, NULL deref em release
- Type: security, operational-risk
- Evidence: `src/server153/share/io/security.h:52-57` — `Security::Create(Type type)` faz `assert(false); return NULL;` se o tipo não está no mapa. Em release (`NDEBUG`), `assert` é no-op, retorna NULL. O chamador (`NetSession::SetISecurity` em `netio.h:134-139`) faz `isec = Security::Create(type); isec->SetParameter(key)` — dereferência NULL.
- Failure Scenario: Se `type` provém de dados de rede (ex: handshake que negocia security), atacante envia tipo inválido → `Security::Create` retorna NULL → `isec->SetParameter(key)` = segfault. Em debug, `assert(false)` aborta o processo.
- Impact: Crash de daemon por input de rede. Depende de se `Security::Type` é controlável pelo atacante (verificar callers de `SetISecurity`).
- Validation: Confirmar lendo `security.h:52-57` e `netio.h:134-139`. Grep por chamadas a `SetISecurity` que passem `type` de origem duvidosa.
- Fix Direction: Substituir `assert(false)` por `throw` ou retornar `NullSecurity` como fallback. Validar `type` antes de chamar `Create`.

### [Medium|Confirmed] OctetsStream::pop_byte faz leitura não-alinhada (UB em C++)
- Type: security, maintainability-risk
- Evidence: `src/server153/share/common/marshal_i386.h:45-49` — `pop_byte` faz `t = *(T *)( (char*)data.begin() + pos )` onde `pos` é arbitrário. Para `T = unsigned long long` (8 bytes em `pop_byte_64`), isso é uma leitura não-alinhada de 8 bytes. Em x86 funciona, mas é UB em C++.
- Failure Scenario: Com otimização agressiva (`-O2`), o compilador pode assumir alinhamento e gerar código que usa SSE/instruções alinhadas, causando crash em dados não-alinhados. Mais relevante se o código for portado para arquitetura que não tolera acesso não-alinhado (ARM, etc.).
- Impact: Latente em x86-32 atual. Crashes se flags de compilação mudarem ou se portado para outra arquitetura.
- Validation: Confirmar lendo `marshal_i386.h:45-49`. Compilar com `-fsanitize=alignment` e rodar testes.
- Fix Direction: Usar `memcpy(&t, (char*)data.begin() + pos, sizeof(t))` em vez de cast direto. O compilador otimiza para instrução única em x86 quando alinhado.

### [Medium|Confirmed] Conf::reload não verifica retorno de stat() — loop imprevisível se arquivo some
- Type: operational-risk
- Evidence: `src/server153/share/common/conf.h:39-42` — `for ( stat(filename.c_str(), &st); mtime != st.st_mtime; stat(filename.c_str(), &st) )` — se `stat` falha (arquivo deletado, permissão negada), `st.st_mtime` é indefinido (não inicializado entre iterações se `stat` falha). O loop pode não terminar ou terminar com estado corrompido.
- Failure Scenario: Operador deleta ou renomeia o arquivo de configuração durante reload. `stat` retorna -1, `st.st_mtime` contém lixo da iteração anterior. Se `mtime != lixo`, o loop continua chamando `stat` (que continua falhando) — potencialmente loop infinito, ou entra no corpo com `st` corrompido e tenta abrir o arquivo inexistente.
- Impact: Hang ou comportamento indefinido em reload de configuração.
- Validation: Confirmar lendo `conf.h:39-42`. Simular `stat` falhando.
- Fix Direction: Verificar retorno de `stat`: `if (stat(...) != 0) break;` ou logar erro e retornar.

### [Medium|Confirmed] Condition::TimedWait usa timezone deprecated com tz_dsttime=8
- Type: maintainability-risk, operational-risk
- Evidence: `src/server153/share/common/mutex.h:159-170` (branch _REENTRANT_) e `mutex.h:348-358` (branch DEADLOCK_DETECT). `gettimeofday(&now, &tz)` com `tz.tz_dsttime = 8`. `struct timezone` é deprecated desde POSIX 2008; `tz_dsttime` nunca foi confiável. O valor 8 é sem significado documentado.
- Failure Scenario: O cálculo de timeout usa `now.tv_sec + nseconds` que está correto (CLOCK_REALTIME). Mas o uso de `gettimeofday` com timezone é misleading e pode confundir mantenedores. Se alguém "corrige" removendo `gettimeofday` e usa `clock_gettime(CLOCK_MONOTONIC)`, o timeout para de funcionar porque `pthread_cond_timedwait` usa CLOCK_REALTIME por default.
- Impact: Latente. Funciona atualmente mas é frágil e misleading.
- Validation: Confirmar lendo `mutex.h:159-170`.
- Fix Direction: Remover `struct timezone tz` e chamar `gettimeofday(&now, NULL)`. Ou usar `clock_gettime(CLOCK_REALTIME, &ts)` diretamente (evita conversão `timeval`→`timespec`).

### [Medium|Confirmed] Random::Update ignora retorno de read() — dados não-inicializados se /dev/urandom falha
- Type: security
- Evidence: `src/server153/share/io/security.h:71` — `Octets& Update(Octets &o) { read(fd, o.begin(), o.size()); return o; }` (branch não-WIN32). Retorno de `read` não é verificado. Se `read` retorna -1 (erro) ou 0 (EOF improvável em /dev/urandom), o buffer contém dados não-inicializados ou lixo de alocação anterior.
- Failure Scenario: Se `/dev/urandom` não está acessível (fd inválido, permissões), `read` falha silenciosamente. O Octets retornado contém lixo. Se usado como chave de sessão ou nonce, a "entropia" é previsível (zeros ou conteúdo de heap reciclado).
- Impact: Chaves criptográficas fracas se /dev/urandom falha. Silencioso — sem log, sem erro.
- Validation: Confirmar lendo `security.h:71`. Verificar `Random::Init()` (linha 65) — se `open` falha, `fd = -1` e `read(-1, ...)` retorna -1.
- Fix Direction: Verificar retorno de `read`; se < `o.size()`, logar erro e lançar ou preencher com fallback.

### [Medium|Confirmed] Protocol::Represent::Map nunca é liberado — leak estático de Reps para stubs
- Type: operational-risk
- Evidence: `src/server153/share/io/protocol.cpp:7` — `Protocol::Represent::Map* Protocol::Represent::map = new Protocol::Represent::Map();` — alocado com `new`, nunca `delete`-ado. Reps para stubs (protocolos estáticos registrados no construtor) permanecem no mapa para sempre.
- Failure Scenario: Cada tipo de protocolo registrado tem um stub. O stub é vivo para sempre. Seu Rep (com Octets cacheada e Mutex) fica no mapa para sempre. Não é um leak crescente (número fixo de stubs), mas é um leak estático que confunde ferramentas de análise de memória.
- Impact: Menor. Memória constante, não cresce. Mas se stubs são criados dinamicamente (improvável), vaza.
- Validation: Confirmar lendo `protocol.cpp:7` e `protocol.h:75-78` (Represent::Release só é chamada de `~Protocol`).
- Fix Direction: Aceitável para programa longo. Documentar que é intencional. Ou usar `static Map` em vez de `new Map` para destruição no exit.

### [Low|Confirmed] CompressARCFourSecurity::GetRatio — divisão por zero se srcsize=0
- Type: operational-risk
- Evidence: `src/server153/share/io/security.h:437` — `static double GetRatio() { return (double)dstsize/srcsize; }`. `srcsize` e `dstsize` são `static int64_t` inicializados a 0 (`security.cpp:23-24`).
- Failure Scenario: Se `GetRatio` é chamada antes de qualquer `Update`, `srcsize=0` → divisão por zero em double → `inf` ou `NaN`. Se o resultado é usado para decisão (ex: log), comportamento indefinido.
- Impact: Menor. Apenas afeta estatísticas/monitoramento.
- Validation: Confirmar lendo `security.h:437` e `security.cpp:23-24`.
- Fix Direction: `return srcsize ? (double)dstsize/srcsize : 0.0;`

### [Low|Confirmed] Octets::Rep::addref sem clobber de memória (assembly x86)
- Type: bug
- Evidence: `src/server153/share/common/octets.h:38-43` — `addref` usa `lock; add $1, %0` sem `"memory"` clobber. `release` (linha 44-53) corretamente inclui `"memory"`. A ausência permite ao compilador reordenar loads/stores não-atômicos em torno do `addref`, potencialmente quebrando a semântica de reference counting em cenários de COW.
- Failure Scenario: Thread A copia um Octets (addref), thread B modifica via `unique()`. Sem barreira de memória em addref, o compilador pode reordenar o load de `base` para antes do `addref`, lendo um valor stale de `rep()->ref`.
- Impact: Latente em x86 (TSO forte). Pode manifestar em ARM ou com otimizações agressivas.
- Validation: Confirmar lendo `octets.h:38-43`. Comparar com `release` (linha 44-53) que tem `"memory"`.
- Fix Direction: Adicionar `: : "memory"` ao clobber list de `addref`.

### [Low|Confirmed] ProxyRpc::Process declara XID tmp não utilizada
- Type: maintainability-risk
- Evidence: `src/server153/share/io/proxyrpc.h:192-195` — `{ XID tmp; rpc->proxy_data >> xid; }` — `tmp` é declarada, não inicializada, não usada. O `>> xid` lê para `this->xid`, não para `tmp`.
- Failure Scenario: Código morto que confunde leitores. Pode indicar intenção original de ler para `tmp` e copiar para `xid`, mas foi alterado.
- Impact: Nenhum funcional. Apenas clutter.
- Validation: Confirmar lendo `proxyrpc.h:192-195`.
- Fix Direction: Remover `XID tmp;` ou usar `rpc->proxy_data >> tmp; xid = tmp;` se era a intenção.

---

## Otimizações e melhorias

1. **PollIO single-threaded por design** — `pollio.h:490-500`: `PollIO::Task::Run` chama `PollIO::Poll(1000)` (single-thread) ou `Poll(-1)` (multi-thread). O poll loop é single-threaded; se qualquer `PollIn`/`PollOut` handler bloqueia, todo I/O para. Considerar múltiplos poll threads com sharding por fd.

2. **Alocação por pacote em Protocol::Task::Dispatch** — `protocol.h:455-466`: `new Task(manager, sid, protocol)` para cada protocolo decodificado. Combinado com `Protocol::Create` → `Clone()` (outra alocação) e cópias de `Octets`, cada pacote gera 2+ heap allocations. Considerar pool de Tasks ou dispatch inline para protocolos leves.

3. **Protocol::Represent cache global com mutex único** — `protocol.h:49-88`: todo `Encode()` toma `Represent::locker` (mutex global) para lookup no mapa, depois toma o mutex do Rep individual. Em servidor com muitos protocolos, o mutex global é bottleneck. Considerar per-thread cache ou lock-free map.

4. **Octets::frob_size sempre arredonda para potência de 2** — `octets.h:94-98`: `reserve(16385)` aloca 32768 bytes. Para alocações frequentes de tamanho fixo (ex: buffers de I/O de 8192), isso é OK, mas para tamanhos variáveis pode desperdiçar até 50% de memória.

5. **Compress/Decompress usam MMX (instruções x87/MMX)** — `streamcompress.h:20-50` e `mppc.h:110-140`: `__asm__ __volatile__ ("movd %%mm2,%0"...)` usa registradores MMX. `emms` é chamado no final, mas entre o início e `emms`, a FPU está em estado MMX. Se outra thread usa FPU no mesmo core (sem `emms`), resultados corrompidos. MMX é obsoleto; SSE2 seria seguro. Compilado com `-m32`, MMX funciona mas limita portabilidade.

6. **gcc.defs.mk: -w suprime todos os warnings** — `share/mk/gcc.defs.mk:2`: `CPP = g++ -w -m32 -std=gnu++11 -fpermissive`. `-w` silencia todos os warnings, mascarando bugs potenciais (unused variables, signedness, etc.). Considerar remover `-w` e corrigir warnings ou usar `-Wno-XXX` seletivo.

7. **IntervalTimer::Update chama observers sem lock** — `itimer.h:97-113`: `it->second->Update()` é chamado fora do lock. Se o observer chama `Attach` (que toma o lock), não há deadlock, mas o iterador `it` é usado depois. Como `std::multimap::insert` não invalida iteradores existentes, é seguro, mas frágil. Considerar coletar observers para notificar sob o lock e notificar fora.

---

## Fatos verificados vs inferências

### Verificados (lidos diretamente no código)
- `frob_size` loop infinito com `size=0xFFFFFFFF` — `octets.h:96-99` ✓
- `Protocol::Decode` chama `data.reserve(size)` sem cap — `protocol.h:530` ✓
- `Conf::find` usa `operator[]` sob `RDScoped` — `conf.h:82-84` ✓
- Debug `fprintf` em `Protocol::Decode` — `protocol.h:537-546` ✓
- `Timer::Update`/`Attach`/`Detach` sem lock — `timer.h:28-29,46-49` ✓
- `GetOsbytes` usa `WRScoped` para leitura — `protocol.h:412` ✓
- `Security::Create` com `assert(false)` — `security.h:52-57` ✓
- `pop_byte` leitura não-alinhada — `marshal_i386.h:45-49` ✓
- `Conf::reload` sem check de `stat()` — `conf.h:39-42` ✓
- `Condition::TimedWait` timezone deprecated — `mutex.h:159-170` ✓
- `Random::Update` ignora `read()` — `security.h:71` ✓
- `Represent::map` nunca liberado — `protocol.cpp:7` ✓
- `CompressARCFourSecurity::GetRatio` div por zero — `security.h:437` ✓
- `Octets::Rep::addref` sem memory clobber — `octets.h:38-43` ✓
- `ProxyRpc::Process` `XID tmp` não usada — `proxyrpc.h:192-195` ✓
- `-w` em gcc.defs.mk — `share/mk/gcc.defs.mk:2` ✓
- Fixes pw-web aplicados: `-D_REENTRANT_` e `-DUSE_HASH_MAP` em ambos branches — `gcc.defs.mk:33,44` ✓; `WRScoped`/`RDScoped` com `locked` flag — `mutex.h:318-333` ✓

### Inferidos (não verificados por execução)
- Que `Security::Type` é controlável por atacante via rede — inferido pela API pública de `SetISecurity`; depende dos callers específicos em cada daemon (não verificados nesta auditoria do share/).
- Que `Timer::Attach` é chamado após threads iniciarem — inferido pela ausência de lock; na prática provavelmente só em construtores estáticos.
- Que `malloc` falha e `Rep::create` crasha com NULL — inferido pela ausência de check NULL em `operator new` (`octets.h:108`); não reproduzido.
- Impacto de performance do `fprintf` debug — inferido pelo custo conhecido de `fprintf`+stderr unbuffered; não medido.
