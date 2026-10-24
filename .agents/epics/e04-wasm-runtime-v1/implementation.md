# Implementação — Toolchain C++/Emscripten e runtime WebAssembly

Loader assíncrono → instância WASM pronta → bridge validada → tick cooperativo → host. Runtime é dono da instância; caller possui inputs e o contrato define quando views são recriadas. Failed/disposed impedem novos ticks; recuperação é inicialização explícita.

## Ordem

Seguir o DAG do JSON, não a ordem numérica dos arquivos. IDs antigos são estáveis;
WO-05 continua sendo gate terminal mesmo quando depende de WO-06 ou posteriores.
Implementação espera o contrato da dependência e ferramenta real disponível.

| WO | Entrega | Automação | Dependências |
| --- | --- | --- | --- |
| [WO-01](workorders/wo-01.md) | Fixar Emscripten e builds reproduzíveis de prova | sim, após gates | e01-agent-orchestration-v1/wo-05 |
| [WO-02](workorders/wo-02.md) | Implementar binding ABI com typed buffers e erros explícitos | sim, após gates | e04-wasm-runtime-v1/wo-01, e04-wasm-runtime-v1/wo-06, e01-agent-orchestration-v1/wo-05 |
| [WO-03](workorders/wo-03.md) | Implementar loop cooperativo com pause, resume e dispose | sim, após gates | e04-wasm-runtime-v1/wo-06, e03-platform-portability-v1/wo-07, e01-agent-orchestration-v1/wo-05 |
| [WO-04](workorders/wo-04.md) | Medir heap, exceções e requisitos de threads sem habilitá-las | sim, após gates | e04-wasm-runtime-v1/wo-02, e04-wasm-runtime-v1/wo-03, e04-wasm-runtime-v1/wo-07, e01-agent-orchestration-v1/wo-05 |
| [WO-05](workorders/wo-05.md) | Homologar testes WASM e paridade nativo/browser | sim, após gates | e04-wasm-runtime-v1/wo-02, e04-wasm-runtime-v1/wo-03, e04-wasm-runtime-v1/wo-04, e04-wasm-runtime-v1/wo-08, e04-wasm-runtime-v1/wo-09, e04-wasm-runtime-v1/wo-10, e01-agent-orchestration-v1/wo-05 |
| [WO-06](workorders/wo-06.md) | Especificar ABI, lifecycle e erros do runtime browser | sim, após gates | e04-wasm-runtime-v1/wo-01, e01-agent-orchestration-v1/wo-05 |
| [WO-07](workorders/wo-07.md) | Recriar views tipadas após crescimento de memória | sim, após gates | e04-wasm-runtime-v1/wo-02, e03-platform-portability-v1/wo-11, e01-agent-orchestration-v1/wo-05 |
| [WO-08](workorders/wo-08.md) | Implementar bootstrap assíncrono com falha visível | sim, após gates | e04-wasm-runtime-v1/wo-06, e01-agent-orchestration-v1/wo-05 |
| [WO-09](workorders/wo-09.md) | Propagar abort e liberar recursos na falha do runtime | sim, após gates | e04-wasm-runtime-v1/wo-02, e04-wasm-runtime-v1/wo-03, e04-wasm-runtime-v1/wo-08, e01-agent-orchestration-v1/wo-05 |
| [WO-10](workorders/wo-10.md) | Automatizar smoke browser com cenários de sucesso e falha | sim, após gates | e04-wasm-runtime-v1/wo-07, e04-wasm-runtime-v1/wo-08, e04-wasm-runtime-v1/wo-09, e01-agent-orchestration-v1/wo-05 |

## Processo

Ler o JSON da tarefa e seus inputs; executar WI-01 a WI-04; escrever teste RED para
comportamento novo; implementar GREEN; revisar diff e anexar evidência. Nunca alterar
o controller/testes para fabricar sucesso. Follow @implementation-loop e @code-review.
Apenas o mantenedor atualiza checkboxes/manifesto após homologação com evidência.
