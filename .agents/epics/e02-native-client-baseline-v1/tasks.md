# Workitems — Baseline do cliente Windows e seleção validada da Angelica

10 workorders, 40 workitems. Checkboxes não concluídas. Follow
@implementation-loop para código e @architecture-spec-authoring para contratos.
Os JSONs são os contratos do dispatcher; cada WI pertence ao seu WO, sem nova
semântica de agendamento. Executar pelo DAG, não por ordenação numérica.

## [WO-01 — Construir inventário estático de solutions, arquivos e bibliotecas 1.5.3/1.5.2](workorders/wo-01.md)

- [ ] **WI-01** — Ler soluções e projetos como dados; registrar SHA e configuração, sem executar binários da referência.
- [ ] **WI-02** — Resolver paths Windows sem sensibilidade a case; distinguir arquivo ausente de nome em outra capitalização e macro não resolvida.
- [ ] **WI-03** — Separar fontes, SDKs, bibliotecas compiladas, assets e intermediários; contar fontes de terceiros sem chamá-las de gameplay exclusivo.
- [ ] **WI-04** — Testar scanner com projetos XML sintéticos e emitir inventário sanitizado dos dois candidatos.

## [WO-02 — Criar matriz cliente, SDK, ABI, assets e protocolo por versão](workorders/wo-02.md)

- [ ] **WI-01** — Separar Release/Debug D3D8 das configurações que apontam para Angelica2.2; registrar os caminhos reais.
- [ ] **WI-02** — Cruzar WORD/DWORD/LONG, wchar_t, packing, tempo de 32 bits e arquitetura das bibliotecas; campos não medidos ficam UNVERIFIED.
- [ ] **WI-03** — Relacionar cada candidato a assets e realm somente quando houver evidência, sem equiparar 1.5.3 a 1.5.5.
- [ ] **WI-04** — Produzir matriz por campo com fonte:linha/hash, ferramenta de verificação e bloqueio observável.

## [WO-03 — Compilar cliente Windows limpo contra engine e bibliotecas verificadas](workorders/wo-03.md)

- [ ] **WI-01** — Preparar checkout isolado autorizado e diretório de build vazio, mantendo references congeladas.
- [ ] **WI-02** — Configurar Release Win32 sem misturar D3D8/D3D9 e sem reutilizar .obj/.pch/.ncb do snapshot.
- [ ] **WI-03** — Compilar ElementClient com os outputs de WO-07 e os caminhos de WO-08; parar no primeiro erro de compilação/link real.
- [ ] **WI-04** — Registrar comando Windows completo, toolchain/SDKs, log, hashes e classificação PASS/FAIL/BLOCKED.

## [WO-04 — Validar inicialização, cena e handshake da baseline Windows](workorders/wo-04.md)

- [ ] **WI-01** — Executar somente o binário identificado por SHA de WO-03 com o manifesto de assets de WO-09.
- [ ] **WI-02** — Registrar startup, janela, dispositivo, primeiro frame e falhas de carregamento separadamente.
- [ ] **WI-03** — Testar autenticação e entrada no mundo apenas contra realm de laboratório identificado; registrar versões sem credenciais.
- [ ] **WI-04** — Produzir matriz startup/cena/handshake/mundo e capturas autorizadas para futuros comparadores.

## [WO-05 — Decidir seleção de versão com evidência, fallback e bloqueios](workorders/wo-05.md)

- [ ] **WI-01** — Consolidar inventários, build, runtime e comparação do candidato alternativo.
- [ ] **WI-02** — Preencher a matriz ponderada de VALIDATION sem substituir campos não verificados por pontuação presumida.
- [ ] **WI-03** — Submeter decisão de versão e autorização ao responsável; manter proposta se falta prova.
- [ ] **WI-04** — Registrar versão/configuração/SDK/assets/realm selecionados e atualizar somente estados demonstrados.

## [WO-06 — Provisionar matriz reproduzível de VS2005 e SDKs no laboratório](workorders/wo-06.md)

- [ ] **WI-01** — Identificar versões de Visual C++, Windows SDK, DirectX e SDKs middleware exigidos por Release Win32.
- [ ] **WI-02** — Registrar origem autorizada, arquitetura e hashes dos instaladores sem redistribuí-los.
- [ ] **WI-03** — Criar preflight que verifica ferramentas e compila um programa Win32 trivial com o SDK selecionado.
- [ ] **WI-04** — Documentar restauração do ambiente e comando real do builder; nenhum segredo entra no relatório.

## [WO-07 — Reconstruir bibliotecas Angelica/Common na ordem de dependência](workorders/wo-07.md)

- [ ] **WI-01** — Derivar ordem de build das referências entre projetos/headers/libs, sem presumir apenas ordem da solução.
- [ ] **WI-02** — Separar bibliotecas que têm fonte das que só têm binário e contrato ABI externo.
- [ ] **WI-03** — Compilar módulos autorizados em diretório vazio e registrar configurações Unicode/CRT correspondentes.
- [ ] **WI-04** — Relacionar cada .lib usada pelo cliente ao output produzido ou à dependência externa verificada.

## [WO-08 — Resolver paths de build, exports e closure de link de Release Win32](workorders/wo-08.md)

- [ ] **WI-01** — Comparar include/library directories com o inventário case-insensitive e macros do ambiente.
- [ ] **WI-02** — Classificar bibliotecas fornecidas pelo SDK versus produzidas localmente e localizar símbolos não resolvidos.
- [ ] **WI-03** — Descrever ajustes apenas no checkout de laboratório, incluindo output path específico da máquina original.
- [ ] **WI-04** — Registrar patch de configuração autorizado e justificativa por alteração; não trocar versão para silenciar erro.

## [WO-09 — Identificar recursos autorizados e realm da baseline](workorders/wo-09.md)

- [ ] **WI-01** — Listar arquivos de dados e pacotes requeridos pela inicialização, sem anexar seus conteúdos.
- [ ] **WI-02** — Registrar hashes, versão, proveniência e autorização do conjunto recebido para laboratório.
- [ ] **WI-03** — Comparar schemas/layouts esperados pelo cliente ao conjunto de recursos; não converter dados por adivinhação.
- [ ] **WI-04** — Definir realm de teste e evidência de protocolo compatível, mantendo credenciais fora do repositório.

## [WO-10 — Comparar alternativa 1.5.2 moderna sem misturar árvores](workorders/wo-10.md)

- [ ] **WI-01** — Comparar toolchain, APIs D3D9, dependências proprietárias e organização de projetos com 1.5.3.
- [ ] **WI-02** — Separar ganhos documentados de modernização das capacidades ainda não compiladas/executadas.
- [ ] **WI-03** — Definir experimento Windows equivalente para o candidato e registrar bloqueios sem exigir migração imediata.
- [ ] **WI-04** — Fornecer insumos à ADR de seleção com rastreabilidade por commit, preservando ambos os submódulos.

## Tarefas humanas e gate

(HUMAN) Provisionar Windows/SDKs e recursos autorizados (WO-06/09); decidir versão/direitos no gate WO-05. Bloqueia laboratório e seleção, não matemática própria.

Todos os WOs precisam de evidência homologada; não preencher completed-workorders.json
por inferência da POC. Nenhuma aprovação passada é automaticamente prova da revisão 1.1.
