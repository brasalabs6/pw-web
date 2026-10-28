# AUDITORIA DE CÓDIGO — server153/cskill + PWSL (READ-ONLY)

Repo: `brasalabs6/pw-web` branch `feat/pw-source-1.5.3-ci-build`
Área: `src/server153/cskill/` (sistema de skills) e `src/server153/PWSL/` (lib)
Data: 2026-10-28 — Auditoria estática, sem execução do servidor, sem edição de arquivos.

## Escopo verificado e método

Lidos por inteiro: `skill/skill.cpp` (903), `skill/skillwrapper.cpp` (2029),
`skill/skill.h` (848), `skill/skillwrapper.h` (437), `expr/exprparser.cpp` (371),
`expr/executor.cpp` (459), `expr/exprparser.hpp`, `expr/ogparser.hpp`,
`expr/scanner.hpp`, `expr/syntaxtree.hpp`, `expr/treenode.h`, `expr/main.cpp`,
`Makefilelib`, `Makefile2`, `skill/makefile`, `mk/gcc.defs.mk`.
Amostrados: `skills/stubs1.cpp`, `skills/skill1.h`, `skills/skill1522.h`,
`skills/skill2696.h`, `skills/skill234.h`, `skills/skill2688.h`,
`skills/skill2921.h`, `skills/skill2733.h`, `skills/skill509.h`, e varreduras
`grep` sobre os ~2879 headers gerados e os 12 `stubs*.cpp`.
Verificado o build ativo via `server153/Makefile` (execdirs, libgs, gs) e
compilação-teste de `stubs1.cpp`.

---

## Findings

### [High|Confirmed] SkillWrapper::Swap troca `cd_adjust_count` com `sw.cd_adjust` (typo)
- Type: bug / data-consistency
- Evidence: `skill/skillwrapper.cpp:1162-1163`
  ```cpp
  std::swap(cd_adjust, sw.cd_adjust);          // correto
  std::swap(cd_adjust_count, sw.cd_adjust);   // BUG: deveria ser sw.cd_adjust_count
  ```
  A linha 1163 troca `cd_adjust_count` (this) com `sw.cd_adjust` (que acabou de receber o `cd_adjust` antigo de this na linha 1162). Após o swap, ambos os wrappers ficam com `cd_adjust`/`cd_adjust_count` cruzados e incorretos.
- Failure Scenario: `SkillWrapper::Swap` é usado em troca de forma/estado que recria o wrapper. Após a troca, `GetCDAdjust()` (`skill/skillwrapper.h:354`: `if(cd_adjust_count) { --cd_adjust_count; return cd_adjust; }`) lê `cd_adjust_count` que agora contém o valor antigo de `cd_adjust` do outro wrapper, e retorna `cd_adjust` que agora contém o `cd_adjust_count` antigo. O filtro `skillfilter.h:16957` consulta `GetCDAdjustCount()` — o ajuste de cooldown é aplicado o número errado de vezes, ou nunca, ou com magnitude errada.
- Impact: Jogadores em transição de forma (GodEvilConvert / EventChange) recebem ajuste de cooldown (buff/debuff de CD) incorreto — cooldowns duplicados ou perdidos. Afeta balanceamento de combate.
- Validation: `grep -n 'std::swap(cd_adjust' skill/skillwrapper.cpp` — confirmar a segunda linha usa `sw.cd_adjust` em vez de `sw.cd_adjust_count`. Comparar com a ordem dos campos em `skill/skillwrapper.h:118-119`.
- Fix Direction: Trocar `sw.cd_adjust` por `sw.cd_adjust_count` na linha 1163. Adicionar um teste que chame `Swap` e verifique que `cd_adjust`/`cd_adjust_count` de ambos os wrappers preservam a identidade (cada campo vai para o campo homônimo do outro).

### [High|Likely] `GetState(1)` sem bounds-check em NpcStart/NpcRun — crash com skill de estado único
- Type: bug / operational-risk
- Evidence:
  - `skill/skillwrapper.cpp:990-992`:
    ```cpp
    int time = skill->GetStub()->GetState(0)->GetTime(NULL);
    next = skill->GetStub()->GetState(1)->GetTime(NULL);
    ```
  - `skill/skill.h:778`: `void NpcRun() { const SkillStub::State * state = stub->GetState(1); Run( state ); }`
  - `skill/skill.h:345-348`: `GetState(int index)` só protege com `ASSERT((unsigned int)index<statestub.size())` — sem `NDEBUG` o ASSERT aborta; com `NDEBUG` é leitura OOB silenciosa.
  - `GetTime(NULL)`: o parâmetro `skill` é `NULL`; nos stubs gerados atuais `GetTime` retorna constante (ignora skill), então o NULL não derefere hoje — mas o `GetState(1)` OOB é o problema real.
  - Contagem: **389** dos ~2879 skills gerados possuem apenas `State1` (sem `State2`), ex.: `skills/skill1000.h` … `skills/skill1006.h`. Para esses, `statestub.size()==1` e `GetState(1)` é OOB.
- Failure Scenario: Um NPC configurado (AI/gamedata) para conjurar um skill de estado único dispara `NpcStart` → `GetState(1)` → `ASSERT_FAIL` aborta o gs (build de debug, que é o caso aqui: `mk/gcc.defs.mk:51` define `-D_DEBUG` e não define `NDEBUG`, então ASSERT é ativo). Em build release (`NDEBUG`), `statestub[1]` lê ponteiro lixo → `GetTime`/`Calculate` em endereço inválido → segfault.
- Impact: Crash (DoS) do gamed/gs. Qualquer skill de 1 estado atribuído a NPC é um vetor de crash determinístico.
- Validation: `grep -L 'class State2' skills/skill*.h | wc -l` → 389. Confirmar que `GetState` não tem fallback além do ASSERT. Confirmar que o build ativo não define `NDEBUG` (`grep -R NDEBUG mk/ skill/makefile`).
- Fix Direction: Em `NpcStart`/`NpcRun`, checar `stub->GetStateSize() >= 2` antes de `GetState(1)` e tratar o caso de skill de estado único (ou garantir que NPCs só usem skills de >=2 estados via validação de gamedata). Reforçar `GetState` para retornar `NULL` em OOB em vez de ASSERT+UB.

### [Medium|Likely] Divisão por `skill_level` em fórmulas geradas (Calculate) — sem guarda local
- Type: bug / data-consistency
- Evidence (gerado, `skills/`):
  - `skills/skill1522.h:79`: `skill->SetRatio (60.0 / skill->GetLevel ());`
  - `skills/skill2696.h:79`: `skill->SetRatio (30.0 / skill->GetLevel () - 1);`
  - `skills/skill234.h:81`: `skill->SetRatio ((1.5 + 0.15 * skill->GetLevel ()) * skill->GetCharging () / (5200 - 200 * skill->GetLevel ()));` — divisor `5200-200*level` zera em `level==26` (fora do `max_level=10`, só alcançável com level corrompido).
  - `skill/skill.h:633-634`: `int GetPlus() { return (int)plus; } int GetRatio() { return (int)ratio; }` — `ratio` é `float`; `60.0/0` → `+inf`; `(int)+inf` é comportamento indefinido (UB), tipicamente `INT_MIN`.
  - `skill/skill.cpp:893`: `int Skill::GetAttack() { return player->GetAttack((int)ratio, (int)plus); }` — propaga o ratio corrompido para o cálculo de dano.
- Failure Scenario: O path principal (`SkillWrapper::StartSkill`, `skillwrapper.cpp:248-251`) guarda `if(level<=0) return -1;` ANTES de `FirstRun`, então o fluxo de conjuração normal protege `level>=1`. Porém: (1) `SkillWrapper::Run` (`skillwrapper.cpp:209` e `:280`) re-busca `GetLevel(skilldata.id,player.GetClass())` **sem** o flag `use=true` e **sem** revalidar `level<=0` antes de `skill->Run()` → se o nível mudar para 0 entre StartSkill e Run (ex.: skill removido por `GodEvilConvert`/`Remove` no meio do cast), `Calculate` roda com `level==0` → `60.0/0`. (2) `SkillWrapper::NpcStart` (`skillwrapper.cpp:970`) faz `skill->SetLevel(level)` com `level` vindo do script/AI de NPC sem validar `>0` antes de `NpcFirstRun` → se o script passar 0, `Calculate` (estado 0) divide por zero. O resultado `inf→(int)UB` produz dano negativo/gigante ou, em modo interpretado (`INTEPRETED_EXPR=1`), SIGFPE em divisão inteira.
- Impact: Dano corrompido (negativo ou INT_MIN) em edge cases de mudança de estado do skill durante o cast, ou por configuração errada de NPC. Não é client-triggerable diretamente (o StartSkill guarda), mas é frágil: a fórmula gerada confia numa pré-condição (`level>=1`) que nem todo caller revalida.
- Validation: `grep -rn '/ skill->GetLevel ()' skills/*.h` → `skill1522.h`, `skill2696.h`. Confirmar que `SkillWrapper::Run` não tem `if(level<=0)` antes de `skill->Run()`. Confirmar `NpcStart` não valida `level`.
- Fix Direction: O gerador (`gen/`) deve emitir guarda `(skill->GetLevel()>0 ? ... : 0)` em qualquer divisão por `GetLevel()`. Em `SkillWrapper::Run`/`NpcStart`, revalidar `level>0` antes de executar `Calculate`. Para `skill234`, garantir `level` nunca excede `max_level` ou clampar o divisor.

### [Medium|Confirmed] Três cópias divergentes do SkillWrapper/skill — duplicação da fonte da verdade
- Type: maintainability-risk / data-consistency
- Evidence:
  - `skill/` (ativo) vs `skill2/` (morto): `diff -rq skill skill2` — todos os `.cpp`/`.h` diferem; `skill2/` contém `*.bak` (skill.cpp.bak, skillwrapper.cpp.bak, etc.). Diferenças reais de código: `skill2/skill.cpp` usa `assert`/`<assert.h>` onde `skill/skill.cpp` usa `ASSERT`/`<ASSERT.h>`; `skill2` adiciona parênteses explícitos em expressões de precedência.
  - `skills/` (ativo) vs `skills2/` (morto): `diff -rq skills skills2` — 2872/2879 headers diferem; diferença é **encoding** dos strings chineses (`skills/` = UTF-8 `"虎击"`, `skills2/` = GBK `"????"`). `skills2/` tem extras `skill167.h.bak`, `skillgbk.txt`, `skillstr.txt`. Os `stubs*.cpp` são idênticos.
  - `cgame/gs/skillwrapper.h` (usado pelo gs ao compilar) é uma **cópia separada** de `cskill/skill/skillwrapper.h` (não symlink): `diff` mostra só whitespace hoje, mas é um arquivo distinto incluído por `cgame/gs/actsession.cpp`, `aipolicy.cpp`, `serviceprovider.cpp`, etc., enquanto o gs **linka** `libskill.so` (compilado de `cskill/skill/`).
- Failure Scenario: Qualquer alteração estrutural em `SkillWrapper` (novo campo, reordenação, novo método virtual) feita em `cskill/skill/skillwrapper.h` que não for espelhada em `cgame/gs/skillwrapper.h` → o gs compila contra um layout de classe diferente do que está em `libskill.so` → offsets de campo/vtable errados → corrupção de memória silenciosa (campo lido no offset errado, método virtual despachado para função errada). Hoje coincidem (diff só whitespace), mas não há mecanismo que force a sincronia. `skill2/`/`skills2/` são armadilha para manutenção: um desenvolvedor pode editar a cópia errada e o build ativo não refletir.
- Impact: Risco de ABI drift silencioso entre gs e libskill.so; risco de manutenção em código morto divergente.
- Validation: `diff cskill/skill/skillwrapper.h cgame/gs/skillwrapper.h` (só whitespace). `ls -la cgame/gs/skillwrapper.h` (arquivo regular, não symlink). `grep -rn '#include "skillwrapper.h"' cgame/gs/*.cpp`. Confirmar que o build ativo usa `skill/`+`skills/` (`server153/Makefile:3` execdirs inclui `cskill/skill/`; `Makefile:117` symlinka `../cskill/skill/libskill.a`).
- Fix Direction: Eliminar `skill2/` e `skills2/` (código morto) ou documentar explicitamente como referência. Fazer `cgame/gs/skillwrapper.h` ser um symlink (ou include) para `cskill/skill/skillwrapper.h` para garantir fonte única. Idealmente, o gs deveria incluir os headers de `cskill/skill/` via `-I` em vez de manter cópia.

### [Medium|Confirmed] Build: `subs`→`skill/makefile` produz `libskill.a` 64-bit; o gs linka `libskill.so` 32-bit (de `Makefilelib`)
- Type: operational-risk / build
- Evidence:
  - `skill/makefile`: `CPP = g++` (sem `-m32`), `DEFINES = -D_SKILL_SERVER -g -ggdb -O0`, target `all: $(LIB)` = `libskill.a`. Compilação-teste neste host x86-64 produz objeto **64-bit** (`file /tmp/archtest.o` → `ELF 64-bit x86-64`).
  - `Makefilelib`: `include ./mk/gcc.defs.mk` → `mk/gcc.defs.mk:6` `CPP = g++ -w -m32 ...`; target `dlib` produz `libskill.so`. O `libskill.so` em disco é **32-bit** (`file cskill/skill/libskill.so` → `ELF 32-bit LSB i386`); os objetos dentro de `libskill.a` em disco também são 32-bit (logo, o `.a` atual foi construído por outro comando, não por `skill/makefile` como escrito).
  - `server153/Makefile:3` `execdirs` inclui `cskill/skill/`; `subs` faz `$(MAKE) -C cskill/skill` → roda `skill/makefile` → produz `.a` 64-bit.
  - `cgame/gs/Makefile:23` linka `libskill.so` (não `.a`); `server153/Makefile:117` symlinka `../cskill/skill/libskill.a` (usado por `libgs`, não pelo binário `gs` final).
- Failure Scenario: Um `make all` limpo pela documentação (`subs` + `libgs` + `gs`) constrói `libskill.a` 64-bit via `skill/makefile`, mas o `gs` 32-bit precisa de `libskill.so` 32-bit (de `Makefilelib dlib`) que **não é produzido** pelo path documentado → link do `gs` falha (ou pega um `.so` obsoleto/stale). O `.a` 64-bit do `subs` é inútil para o gs 32-bit. O build só funciona se alguém rodar manualmente `make -f Makefilelib dlib` (com `-m32`).
- Impact: Reprodutibilidade do build comprometida; risco de linkar `.so` stale (lógica de skill desatualizada) sem perceber. Dois makefiles com toolchains divergentes (`-m32` vs não) e saídas diferentes (`.a` vs `.so`).
- Validation: `file cskill/skill/libskill.so` (32-bit) vs compilação de `skill/makefile` (64-bit). `grep -m32 skill/makefile` (ausente) vs `grep -m32 mk/gcc.defs.mk` (presente). Confirmar que `subs` não invoca `Makefilelib`.
- Fix Direction: Unificar o build do cskill num único makefile que produza `libskill.so` 32-bit (com `-m32`) e que o `gs` linka. Remover `skill/makefile`/`Makefilelib`/`Makefile2` duplicados ou tornar um wrapper do outro. Garantir que `subs` produza o artefato que o `gs` consome.

### [Medium|Confirmed] Vulnerabilidade de format string no path de erro dos parsers interpretados
- Type: security
- Evidence:
  - `expr/exprparser.hpp:97-99`:
    ```cpp
    sprintf(errinfo,"ExprParser:: ERROR:LINE%4d: %s",context.lineno,context.linebuf);
    printf(errinfo);   // errinfo usado como format string
    ```
  - `expr/ogparser.hpp:99-101`: padrão idêntico (`printf(errinfo)` com `context.linebuf` embutido).
  - `expr/scanner.hpp:160-167`: `getContext` faz `strncpy(con.linebuf,str_source.c_str(),std::min(BUFLEN,(int)str_source.size()))` — se `str_source.size() >= BUFLEN(256)`, **não** escreve terminador nulo → `linebuf` não terminada; o `%s` do sprintf lê além do buffer.
  - `printf(errinfo)` trata o conteúdo controlável (`linebuf`, que vem do `skill.og`/expressão em string) como format string: `%n` → escrita de memória, `%s` → leitura de pilha, `%lx` longo → leak.
- Failure Scenario: No modo interpretado (`INTEPRETED_EXPR=1`), um `skill.og` malicioso/corrompido com uma linha contendo `%n`/`%s` dispara um erro de parse → `printf(errinfo)` → escrita/leitura arbitrária de memória no processo do servidor. Mesmo sem `%`, o `linebuf` não terminado pode causar over-read.
- Impact: Execução de código/crash no servidor (apenas no modo interpretado, que NÃO é o build ativo — ver "Fatos verificados").
- Validation: `grep -n 'printf(errinfo)' expr/exprparser.hpp expr/ogparser.hpp`. Confirmar `INTEPRETED_EXPR` não definido no build ativo (`grep INTEPRETED_EXPR skill/makefile Makefilelib`).
- Fix Direction: Trocar `printf(errinfo)` por `printf("%s", errinfo)` (ou `fputs(errinfo, stderr)`). Garantir terminação nula em `getContext` (`linebuf[BUFLEN-1]='\0'`). Idealmente, validar/sanitizar `skill.og` no carregamento.

### [Low|Confirmed] Recursão sem limite de profundidade no parser/executor interpretado
- Type: operational-risk
- Evidence:
  - `expr/exprparser.cpp:6-9` `stmt_sequence`→`statement`→`if_stmt`(`exprparser.cpp:70-86`)→`stmt_sequence` (corpos then/else); `exp`→`assign_exp`→`select_exp`→`logic_relation_exp`→...→`multi_div_exp`→`factor`(`exprparser.cpp:240`)→`exp` (para parênteses, `exprparser.cpp:309`). Sem contador de profundidade.
  - `expr/executor.cpp:222-224` `stmtexec` recursa em `child[i]` sem limite.
- Failure Scenario: Um `skill.og` com aninhamento profundo (`((((...))))` ou `if then if then ... end end`) exaure a pilha → stack overflow → crash do servidor no parse/exec. O input é o arquivo de dados `skill.og` (shipped), mas se for regenerado/corrompido, derruba o processo.
- Impact: Crash no modo interpretado (não no build ativo).
- Validation: Confirmar ausência de contador de profundidade em `exprparser.cpp`/`executor.cpp`.
- Fix Direction: Adicionar limite de profundidade (contador ou profundidade máxima no parser/executor) e abortar com erro ao exceder.

### [Low|Confirmed] Divisão/módulo por zero sem guarda no executor interpretado
- Type: bug
- Evidence: `expr/executor.cpp:447` `case Token::OVER: CALC_OPERATION(/);` e `:449` `case Token::MOD: t->value.nval = t->child[0]->value.nval % t->child[1]->value.nval;`. O macro `CALC_OPERATION` para `INT_T` faz divisão inteira direta sem checar divisor zero → SIGFPE; o `MOD` idem.
- Failure Scenario: Expressão de skill interpretada com `x / 0` ou `x % 0` (inteiro) → SIGFPE → crash do servidor.
- Impact: Crash no modo interpretado (não no build ativo).
- Validation: `grep -n 'CALC_OPERATION(/)' expr/executor.cpp`.
- Fix Direction: Checar divisor zero antes da divisão/módulo inteiro e lançar erro ou retornar 0.

### [Low|Info] `PWSL/compatible`: binários `.so` NÃO commitados (premissa da tarefa não confirmada); PWSL não linkado
- Type: data-consistency / maintainability-risk
- Evidence:
  - `git ls-files src/server153/PWSL/compatible/` → apenas `readme.txt`. `ls -la PWSL/compatible/` → só `readme.txt` (256 bytes, GBK). Os `libio32.so`/`liblog32.so`/`libio32v.so` mencionados no readme **não estão no repo** (nem no disco, nem no git).
  - `grep -rn 'PWSL\|libio32\|liblog32' server153/Makefile cnet/*/Makefile* cgame/*/Makefile*` → **nenhuma referência**. O build do servidor não inclui nem linka PWSL.
  - `cskill/skill/makefile` usa `-I../header -I../header/include -I../common -I../share`, **não** `-I../PWSL/include`. Logo, o cskill usa suas próprias cópias de headers, não as do PWSL.
  - Divergência de headers confirmada: `diff PWSL/include/hashmap.h cskill/header/hashmap.h` (diferem em allocator/constructor), `diff PWSL/include/hashtab.h cskill/header/hashtab.h` (diferem: `stdint.h` vs `allocator.h`). `ASSERT.h` é idêntico.
- Failure Scenario: Sem falha funcional hoje (PWSL é código morto neste repo). Risco futuro: dois conjuntos de headers (PWSL/include vs cskill/header) divergem silenciosamente; um desenvolvedor que adote PWSL como "lib oficial" encontra versões diferentes das mesmas classes.
- Impact: Nenhum impacto runtime (PWSL não é construído/linkado). Maintainability: duas fontes de verdade para `hash_map`/`hashtab`/etc.
- Validation: `git ls-files src/server153/PWSL/compatible/`; `find PWSL -name '*.so'`; `grep -rn PWSL server153/Makefile`.
- Fix Direction: Decidir se PWSL é referência arquivística (então documentar e não tratar como build input) ou lib ativa (então integrar ao build e eliminar as cópias em `cskill/header`). Remover `readme.txt` que referencia binários ausentes ou restaurá-los com proveniência documentada.

### [Low|Confirmed] `SkillWrapper::Interrupt`/`Cancel` retornam `-1` (bool true) quando `Skill::Create` falha
- Type: bug (menor)
- Evidence: `skill/skillwrapper.cpp:395` (`bool SkillWrapper::Interrupt(...) { ... if( !skill ) return -1; ...}`) e `:425` (`bool SkillWrapper::Cancel(...) { if( !skill ) return -1; ...}`). `-1` convertido a `bool` = `true`.
- Failure Scenario: Um `skilldata.id` não registrado faz `Skill::Create` retornar NULL → `Interrupt`/`Cancel` retornam `true` → o gs envia `SendClientMsgSkillInterrupted(1)/(2)` ao cliente, reportando interrupção de um skill inexistente.
- Impact: Mensagem espúria ao cliente; sem efeito em estado de jogo real (skill inválido não estava em execução). Baixo.
- Validation: `grep -n 'return -1' skill/skillwrapper.cpp` dentro das funções bool.
- Fix Direction: Retornar `false` (não `-1`) quando `Skill::Create` falha em funções `bool`.

### [Low|Info] Overflow de dano em `Set*Damage`: `(int)(attack * (1.0+inc))` sem checar INT_MAX
- Type: performance-risk / data-consistency (menor)
- Evidence: `skill/skill.cpp:895-899`:
  ```cpp
  void Skill::SetGolddamage(int attack)  { magicdamage[0] = (int)(attack * (1.0+player->GetIncgold())); }
  // ... análogo para Wood/Water/Fire/Earth
  ```
  `attack` é `int`; `attack * (1.0+inc)` é `double`; se `attack` próximo de `INT_MAX` e `inc>0`, o produto excede `INT_MAX` → `(int)` cast é UB (tipicamente wrap para negativo).
- Failure Scenario: Buff stacking extremo (`GetIncgold()` etc. altos) combinado com `attack` muito alto (skill de dano percentual) → dano mágico negativo (cura o alvo) ou valor absurdo. Em PW 1.5.3 valores típicos são < 10^6, então improvável, mas não há clamp.
- Impact: Dano mágico corrompido em casos extremos de buff.
- Validation: `grep -n 'SetGolddamage\|SetWooddamage' skill/skill.cpp`.
- Fix Direction: Clampar o produto a `[0, INT_MAX]` antes do cast, ou usar `int64`/`long long` intermediário e saturar.

### [Low|Info] `Skill::Create` faz `map.find` O(log N) + heap alloc por chamada de skill
- Type: performance-risk
- Evidence: `skill/skill.h:520-525` `static Skill * Create(ID i) { const Skill * sk = GetStub(i); return sk?sk->Clone():NULL; }` — `GetStub` é `std::map::find` (O(log N), N≈2879). `Clone()` é `new Skill(*this)` (heap alloc). Cada método de `SkillWrapper` (`StartSkill`, `Run`, `Condition`, `Interrupt`, `Cancel`, `Attack`, `Infect`, …) chama `Skill::Create` novamente, criando/destruindo um `Skill` por operação (`SkillKeeper` faz `delete` no fim do escopo).
- Failure Scenario: Servidor carregado: cada cast/ataque/condição cria e destrói um `Skill` no heap + ~12 comparações de mapa. Não é incorreto, mas é churn de alocação e lookup repetitivo.
- Impact: Overhead de CPU/heap em servidor populado; não é correção.
- Validation: `grep -n 'Skill::Create' skill/skillwrapper.cpp` (muitas ocorrências).
- Fix Direction: Reusar o `Skill` protótipo (`GetStub` retorna `const Skill*`) para leitura de propriedades imutáveis; alocar `Skill` de execução de um pool. Otimização opcional, não bloqueante.

### [Low|Info] Stubs `.o` gigantes (~8.8 MB cada) compilados com `-O0 -w`; sem tracking de dependência de headers
- Type: operational-risk / maintainability-risk
- Evidence: `skill/makefile`: `DEFINES = -D_SKILL_SERVER -g -ggdb -O0`, `CPPFLAGS = -Wall -Werror` para os objetos principais, mas `$(STUBOBJS)` é compilado com `-w` (suprime warnings). Compilação-teste de `stubs1.cpp` produziu `stubs1.o` de **8.8 MB**. Cada `stubs*.cpp` inclui ~240 headers de skill (ex.: `stubs1.cpp` tem ~240 `#include "skillNNNN.h"`); 12 TUs × 240 classes ≈ 2879 classes.
  - `skill/makefile` não tem regra de dependência `stubs*.o: ../skills/*.h` (só `$(OBJS) : $(HEADERS)`). `Makefilelib:40` tem `$(STUBOBJS) : $(HEADERS) ../skills/*.h` mas o build ativo usa `skill/makefile`.
- Failure Scenario: (1) Tempo de compilação elevado (12 TUs grandes, `-O0`); com `-j8` é tolerável mas lento. (2) Se um `skillNNNN.h` mudar, `make` via `skill/makefile` **não** reconstrói os `stubs*.o` (sem dependência de header) → binário stale com skill desatualizado. (3) `-w` esconde warnings reais das fórmulas geradas (ex.: conversões int↔float implícitas, comparações signed/unsigned).
- Impact: Builds stale silenciosos; warnings de qualidade ocultos; compilação lenta.
- Validation: `ls -la /tmp/stubs1_test.o` (8.8 MB). `grep -n 'STUBOBJS : ' skill/makefile` (ausente). `grep -- '-w' skill/makefile`.
- Fix Direction: Adicionar geração de dependências (`-MMD -MP`) para os stubs; reduzir `-O0` para `-O1`/`-O2` em release; não suprimir warnings dos stubs (corrigir o gerador para emitir código limpo); considerar dividir os stubs em mais TUs menores ou compilar headers pré-compilados (PCH).

---

## Otimizações e melhorias

1. **Eliminação de código morto (`skill2/`, `skills2/`)**: confirmado que o build ativo usa `skill/`+`skills/` (`server153/Makefile:3,117`). `skill2/`/`skills2/` (com `*.bak`) divergem e só confundem manutenção. Remover ou mover para `references/` com nota de que são snapshot alternativo.
2. **Fonte única de `SkillWrapper`**: fazer `cgame/gs/skillwrapper.h` apontar para `cskill/skill/skillwrapper.h` (symlink/include) para eliminar risco de ABI drift entre gs e `libskill.so`.
3. **Unificação do build do cskill**: um único makefile produzindo `libskill.so` 32-bit (`-m32`) consumido pelo gs; remover `skill/makefile`/`Makefilelib`/`Makefile2` duplicados ou tornar um wrapper.
4. **Guardas no gerador de skills**: o gerador Java (`gen/`) deve emitir `(level>0 ? ... : 0)` em divisões por `GetLevel()` e clampar divisors dependentes de level. Hoje a robustez depende de pré-condições do caller, frágeis.
5. **Bounds em `GetState`**: retornar `NULL` em OOB (em vez de `ASSERT`+UB) e tratar `NULL` nos callers (`NpcStart`/`NpcRun`/`FirstRun`/`Run`).
6. **Correção do typo de `Swap`** (linha 1163): troca de 1 caractere, alto valor.
7. **Parser interpretado (se reativado)**: `printf("%s", errinfo)`, terminação nula em `linebuf`, limite de profundidade, guarda de divisão por zero.
8. **Rastreabilidade de dependências nos stubs**: `-MMD -MP` para evitar builds stale quando `skillNNNN.h` muda.

---

## Fatos verificados vs inferências

### Verificados (evidência direta no repo)
- O build ativo compila `cskill/skill/` + `cskill/skills/` (NÃO `skill2/`/`skills2/`); `expr/` (modo interpretado) **não** é compilado no build ativo (`INTEPRETED_EXPR` indefinido em `skill/makefile`; `Makefilelib` define `INTEPRETED_EXPR = 0`). — `server153/Makefile:3`, `skill/makefile`, `Makefilelib:4`.
- Typo em `SkillWrapper::Swap` (`cd_adjust_count` ↔ `sw.cd_adjust`): `skill/skillwrapper.cpp:1162-1163`.
- `GetState(1)` chamado sem checar `GetStateSize()` em `NpcStart`/`NpcRun`; 389 skills gerados têm só 1 estado; `GetState` só protege com `ASSERT` (ativo em build sem `NDEBUG`, que é o caso: `mk/gcc.defs.mk:51` define `-D_DEBUG`, sem `NDEBUG`).
- Divisão por `skill->GetLevel()` sem guarda local em `skills/skill1522.h:79`, `skills/skill2696.h:79`; divisor level-dependente em `skills/skill234.h:81`.
- `SkillWrapper::Run` (`:209`,`:280`) e `NpcStart` (`:970`) não revalidam `level>0` antes de executar `Calculate`; `StartSkill` (`:248-251`) valida `if(level<=0) return -1`.
- `cgame/gs/skillwrapper.h` é arquivo regular (não symlink), divergente só em whitespace hoje de `cskill/skill/skillwrapper.h`; gs linka `libskill.so` e inclui a cópia local.
- `skill/makefile` usa `g++` sem `-m32` (produz 64-bit neste host); `Makefilelib`/`gcc.defs.mk` usam `-m32`; `libskill.so` em disco é 32-bit; `subs` (top Makefile) só constrói `.a` via `skill/makefile`.
- `printf(errinfo)` com `errinfo` contendo `linebuf` (não terminado se size≥256) em `expr/exprparser.hpp:99` e `expr/ogparser.hpp:101`.
- Recursão sem limite em `expr/exprparser.cpp` e `expr/executor.cpp`; divisão/módulo por zero sem guarda em `expr/executor.cpp:447,449`.
- `PWSL/compatible/` contém só `readme.txt`; nenhum `.so`/`.a` commitado nem presente; PWSL não é referenciado por nenhum Makefile do servidor; headers `PWSL/include` divergem de `cskill/header` (`hashmap.h`, `hashtab.h`).
- `SkillWrapper::Interrupt`/`Cancel` retornam `-1` (bool true) em falha de `Skill::Create`: `skill/skillwrapper.cpp:395,425`.
- `Set*Damage` faz `(int)(attack*(1.0+inc))` sem clamp: `skill/skill.cpp:895-899`.
- `SkillStub::Initialize` (`skill/skill.h:330-341`) é **O(N log N)** (loop único + `map::find` O(log N)), **não O(N²)** — hipótese da tarefa não confirmada.
- `stubs1.o` de teste = 8.8 MB; `skill/makefile` compila stubs com `-O0 -w` e sem dependência de `../skills/*.h`.
- Nenhum skill gerado tem `State1::Bypass` retornando true (FirstRun sempre executa estado 0 é seguro na prática).
- `gen.jar` (gerador Java de skills) e jars de `share/` (commons-collections-3.1, commons-dbcp-1.2.1, commons-pool-1.2) estão commitados — binários de terceiros sem versão pinada além do nome (nota de supply-chain menor, fora do escopo cskill).

### Inferências (não confirmadas por execução)
- A consequência prática do typo de `Swap` (cooldown duplicado/perdido em troca de forma) é inferida do fluxo de `GetCDAdjust`/`GetCDAdjustCount` e do uso em `skillfilter.h:16957`; não reproduzida em runtime.
- O crash de `GetState(1)` para NPC com skill de 1 estado é inferido (389 skills de 1 estado existem; não confirmei quais IDs NPCs usam de fato — requer gamedata/AI do gs, fora do escopo cskill).
- O path de divisão por zero em `Calculate` via `SkillWrapper::Run` com level mudado a meio do cast é inferido (exige corrida entre `GodEvilConvert`/`Remove` e `Run`); não reproduzido.
- O impacto do overflow de `Set*Damage` é inferido (exige buff stacking extremo além dos valores típicos de PW 1.5.3).
- A quebra do `make all` limpo (sem `.so` para o gs) é inferida da topologia dos makefiles; não executei um `make all` completo do servidor.
