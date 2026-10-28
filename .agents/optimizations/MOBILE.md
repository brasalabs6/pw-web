# Estratégia mobile e perfis adaptativos

## Metas aspiracionais (não medidas)
Desktop gamer 120+ FPS; notebook intermediário 60; PC básico 30–60; Android intermediário 30–60; Android básico 30; topo de linha 60. Metas dependem de resolução, cena e condições térmicas.

## Perfis
- Ultra: alta resolução, LOD distante, sombras e efeitos altos.
- Balanced: 60 FPS quando sustentável, resolução dinâmica, efeitos moderados.
- Battery: 30 FPS, cap de render, menor frequência de tarefas visuais.
- Low: baixa densidade de vegetação/partículas, LOD agressivo, texturas reduzidas.

## Adaptação
Detectar capacidades sem inferir potência somente por user agent. Controlar resolução interna, sombras, partículas, anisotropia, LOD, skinning e prefetch por orçamento de frame. Usar histerese para evitar oscilação de qualidade. Preservar legibilidade, controles touch, responsividade e acessibilidade.

## Validação
Android Chrome, iOS Safari, notebooks com GPU integrada e desktops. Testar aquecimento, throttling após 20–40 min, background/resume, memória limitada, queda de rede e carregamento em conexão móvel. Verificar disponibilidade de WebGL2/WebGPU por dispositivo.
