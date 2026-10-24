# Epic Charter: Compatibilidade de navegadores, acessibilidade e dispositivos

- **Identificador:** `e18-browser-ux-accessibility-v1`
- **Versão:** 1.0.0 (alterações semânticas exigem PR + ADR)
- **Status:** planned
- **Objetivo mensurável:** Assegurar execução em Chromium/Firefox/Safari elegíveis, fallback e diagnóstico WebGL, layouts escaláveis, teclado/gamepad/toque, i18n e UX para sessões longas.
- **Principal dependência:** `e17-security-deployment-v1`, salvo workorders independentes documentados
- **Proprietário da decisão:** Tech Lead do PW Web; gates jurídicos pelo responsável de IP.

## Contexto e razão de existir
A conversão para WebAssembly requer validação progressiva de compatibilidade, direitos, API de plataforma, recursos originais, cliente e realm. Este épico reduz especificamente os riscos de **WebGL2 não suportado ou driver instável; limites iOS/OPFS; acessibilidade insuficiente e ações presas no browser**. A execução não significa entrega do jogo inteiro: separa componentes demonstráveis de conjecturas.

## Entradas e artefatos
- web/
- tests/browser
- docs/

## Contratos de saída obrigatórios
- CapabilityMatrix
- GraphicsFallback
- AccessibilityState
- InputBindings
- LocaleBundle

## Critérios executivos de sucesso
- matriz browser/dispositivo preenchida
- fluxos HUD navegáveis por teclado
- sem crashes após pause/resume

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
