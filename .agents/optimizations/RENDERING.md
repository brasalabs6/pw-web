# Renderer — plano de otimização

## Hipóteses a validar
Excesso de draw calls, state changes, overdraw, skinning na CPU, terreno com geometria excessiva, shaders não especializados e uploads síncronos podem limitar o cliente. Nenhum gargalo foi comprovado ainda.

## Etapas
1. Instrumentar frame CPU/GPU, draw calls, triângulos, shader switches, texture binds, uploads e VRAM estimada.
2. Ordenar draw commands por material e pipeline; agrupar geometria compatível; garantir transparência correta.
3. GPU instancing para vegetação e props repetidos; medir custo de culling versus ganho.
4. Frustum culling, hierarchical spatial partitioning e occlusion culling quando a plataforma justificar.
5. LOD de terreno, meshes e efeitos com histerese; evitar popping perceptível.
6. Texturas comprimidas com fallback por capacidade; mipmaps, anisotropia e resolução dinâmica.
7. Shaders e pipelines pré-compilados/aquecidos quando possível; minimizar stalls.
8. WebGPU experimental atrás de flag, comparando WebGL2 em mesmas cenas.

## Guardrails
Manter correção de blending, sorting, sombras, animações, partículas e efeitos originais. Medir qualidade visual por capturas de referência. Context loss/restoration obrigatório. Não usar instancing em objetos que exigem estados distintos incompatíveis. Não presumir que WebGPU é sempre mais rápido.

## Aceite
Comparação A/B no mesmo commit, hardware e cena: p50/p95/p99 frame time, draw calls, memória GPU estimada, bugs visuais e tempo de compilação de shaders; nenhuma regressão funcional.
