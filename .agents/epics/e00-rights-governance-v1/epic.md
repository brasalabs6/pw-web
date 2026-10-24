# Epic Charter: Direitos, proveniência, licenciamento e decisões de produto

- **Identificador:** `e00-rights-governance-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Obter base de autorização verificável por componente antes de usar ou distribuir código, conteúdo, trilha sonora, formatos ou marca relacionados a Perfect World.
- **Principal dependência:** nenhuma, exceto governança técnica
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **licença de terceiro não comprova titularidade; redistribuição pública de assets; licença incompatível com distribuição comercial**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- docs/LEGAL.md
- docs/RESEARCH.md
- matriz de proveniência
- registro de deliberações ADR

## Contratos de saída obrigatórios
- registro origem→hash→titular→uso permitido
- gate de publicação por artefato
- planos de substituição por componente

## Critérios executivos de sucesso
- 100% componentes distribuídos autorizados
- zero binários/arquivos originais no Git
- documentos de aprovação por release

## Fora do escopo
- Distribuir código/assets de terceiros sem permissão documentada.
- Substituir testes por declarações verbais ou prints sem contexto.
- Liberar alterações diretamente em produção; workorders entram em PR, com revisão e CI.
- Tratar fixtures sintéticas como prova da funcionalidade integral do Perfect World.

## Dependências e desbloqueio
- Referência de cliente/engine/protocolo explicitamente identificada em testes.
- Contratos e controles em `AGENTS.md` seguidos integralmente.
- Ambientes de desenvolvimento isolados e artefatos reproduzíveis.
- Decisões de mudança de stack, versão ou direitos documentadas em ADR.

## Definition of done
Cinco workorders com evidência verificável; aceitação e limites documentados; matriz de riscos atualizada; testes unitários/integrados pertinentes executados; revisão técnica e, quando aplicável, revisão jurídica.
