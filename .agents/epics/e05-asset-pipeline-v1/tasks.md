# Workitems — Pipeline de recursos PCK/PKX, formatos e proveniência

12 workorders, 48 workitems. Checkboxes não concluídas. Follow
@implementation-loop para código e @architecture-spec-authoring para contratos.
Os JSONs são os contratos do dispatcher; cada WI pertence ao seu WO, sem nova
semântica de agendamento. Executar pelo DAG, não por ordenação numérica.

## [WO-01 — Catalogar versões e layouts observados dos formatos de assets](workorders/wo-01.md)

- [ ] **WI-01** — Localizar leitores originais de PCK/PKX e famílias ECM/SKI/BON/terrain sem copiar implementações.
- [ ] **WI-02** — Registrar magic, versões, offsets, compressão e encoding somente quando constatados em fonte ou fixture autorizada.
- [ ] **WI-03** — Separar variantes por cliente e marcar campos/formatos desconhecidos como UNVERIFIED.
- [ ] **WI-04** — Selecionar um único formato/variante para a primeira fatia e enumerar dependências de recursos.

## [WO-02 — Fechar contratos de parsing, encoding e manifesto da fatia](workorders/wo-02.md)

- [ ] **WI-01** — Materializar schema binário de uma variante escolhida, endian, offsets, overflow e limites de tamanho.
- [ ] **WI-02** — Definir política GBK/Unicode, paths, colisão de nomes, arquivos duplicados e dependências.
- [ ] **WI-03** — Definir manifesto com versão, hash SHA-256, byte length, dependências e provenance sem paths privados.
- [ ] **WI-04** — Especificar cancelamento, erro de descompressão/truncamento e fixtures sintéticas construídas a partir do schema.

## [WO-03 — Integrar parsers defensivos e fuzzing da variante escolhida](workorders/wo-03.md)

- [ ] **WI-01** — Compor reader, índice, decompressão e nomes, mantendo validação antes de alocação/cópia.
- [ ] **WI-02** — Executar corpus sintético mínimo, truncamentos em todos os pontos relevantes e mutações determinísticas.
- [ ] **WI-03** — Rodar sanitizers e a mesma suite nativa/WASM, registrando consumo máximo por entrada.
- [ ] **WI-04** — Documentar variantes aceitas/rejeitadas; não expandir automaticamente para outro formato.

## [WO-04 — Gerar manifestos determinísticos com hashes e dependências](workorders/wo-04.md)

- [ ] **WI-01** — Implementar schema aprovado em assets-v0 com ordenação e serialização determinísticas.
- [ ] **WI-02** — Calcular hashes/tamanhos dos bytes aceitos e registrar dependências explícitas sem inferir paths do host.
- [ ] **WI-03** — Validar identificadores, duplicatas, versão e ciclos segundo contrato antes de emitir manifesto.
- [ ] **WI-04** — Testar repetição e falhas sem publicar pacotes ou conteúdo original.

## [WO-05 — Homologar pipeline com recursos reais autorizados](workorders/wo-05.md)

- [ ] **WI-01** — Receber apenas o conjunto autorizado com hashes e cliente/engine compatíveis identificados.
- [ ] **WI-02** — Executar pipeline sem adicionar arquivos originais ao Git/CI público e comparar com o leitor de referência.
- [ ] **WI-03** — Registrar limites, falhas e diferenças de encoding/versão; não ajustar schema para um arquivo corrupto sem evidência.
- [ ] **WI-04** — Emitir gate com proveniência e escopo de uso, distinguindo leitura correta de autorização para distribuição.

## [WO-06 — Definir limites de leitura e budgets para entradas binárias](workorders/wo-06.md)

- [ ] **WI-01** — Definir API de byte reader com uint64_t para offsets/tamanhos e conversão verificada para memória wasm32.
- [ ] **WI-02** — Definir limite por arquivo, entrada, nome, número de registros e tamanho descomprimido como parâmetros explícitos.
- [ ] **WI-03** — Especificar EOF/truncamento/overflow e output/cursor preservados em erro, com exemplos aritméticos.
- [ ] **WI-04** — Documentar fixtures virtuais acima de 2GiB sem criar blobs gigantes e orçamento do fuzzer.

## [WO-07 — Implementar byte reader com limites e endian explícitos](workorders/wo-07.md)

- [ ] **WI-01** — Implementar operações declaradas no contrato de limites usando tamanhos fixos.
- [ ] **WI-02** — Checar intervalos sem soma que possa transbordar; leitura de inteiros não usa cast desalinhado de struct.
- [ ] **WI-03** — Preservar cursor/outputs em erro e não alocar proporcionalmente a tamanho não validado.
- [ ] **WI-04** — Testar vazio, truncamento, endian, desalinhamento e offsets virtuais grandes nos dois alvos.

## [WO-08 — Implementar índice de uma variante PCK/PKX confirmada](workorders/wo-08.md)

- [ ] **WI-01** — Implementar somente a variante e layout aprovados em assets-v0 usando o byte reader.
- [ ] **WI-02** — Validar magic/version, quantidade de registros e intervalos antes de construir o índice.
- [ ] **WI-03** — Tratar fronteira PCK/PKX de acordo com o schema, sem truncar offsets para uint32.
- [ ] **WI-04** — Construir fixtures pequenas próprias válidas e inválidas sem copiar conteúdo comercial.

## [WO-09 — Implementar descompressão com tamanho de saída limitado](workorders/wo-09.md)

- [ ] **WI-01** — Usar algoritmo/biblioteca identificado no schema; justificar dependência se não existir solução apropriada.
- [ ] **WI-02** — Impor limite de bytes de saída e erro por stream truncado/inconsistente antes de consumir memória excessiva.
- [ ] **WI-03** — Verificar bytes consumidos, trailing data e checksum conforme o formato confirmado.
- [ ] **WI-04** — Comparar saída nativa/WASM em fixtures comprimidas próprias e fuzz determinístico.

## [WO-10 — Implementar conversão de nomes e detecção de colisões](workorders/wo-10.md)

- [ ] **WI-01** — Converter encoding legado somente como especificado em assets-v0, usando implementação existente apropriada.
- [ ] **WI-02** — Aplicar regras de caminho lógico depois da conversão e manter rastreabilidade por identificador sanitizado.
- [ ] **WI-03** — Detectar colisões de case/normalização e nomes inválidos sem renomear silenciosamente.
- [ ] **WI-04** — Testar bytes inválidos, caracteres multibyte, nomes repetidos e traversal após decodificação.

## [WO-11 — Validar manifesto e grafo de dependências no consumidor](workorders/wo-11.md)

- [ ] **WI-01** — Validar schema/versão do manifesto antes de agendar qualquer fetch.
- [ ] **WI-02** — Conferir grafo de dependências e computar ordem determinística de preparo da cena.
- [ ] **WI-03** — Verificar hash e byte length dos dados recebidos antes de expor recurso ao consumidor.
- [ ] **WI-04** — Testar manifesto ausente, desconhecido, cíclico e conteúdo adulterado.

## [WO-12 — Implementar pré-carregamento assíncrono limitado da primeira cena](workorders/wo-12.md)

- [ ] **WI-01** — Agendar apenas recursos da pequena cena e limites definidos em assets-v0, sem carregar todo o jogo.
- [ ] **WI-02** — Entregar bytes já disponíveis ao leitor síncrono; não bloquear o tick esperando fetch.
- [ ] **WI-03** — Propagar cancelamento/falha de dependência e descartar resposta tardia após unload.
- [ ] **WI-04** — Testar carga local válida, interrupção, falta de memória e concorrência limitada.

## Tarefas humanas e gate

(HUMAN) Fornecer assets autorizados e decidir uso/distribuição em WO-05. Testes sintéticos não aprovam direitos nem compatibilidade de versão.

Todos os WOs precisam de evidência homologada; não preencher completed-workorders.json
por inferência da POC. Nenhuma aprovação passada é automaticamente prova da revisão 1.1.
