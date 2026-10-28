# Personagens, skeleton e animação

## Estado
PR #23 declara leitura de trilhas BON; skeleton, skinning e personagem animado integral ainda são milestones pendentes. Priorizar correção antes de otimização agressiva.

## Otimizações candidatas
- Animation LOD por distância, área projetada e importância de gameplay.
- Atualização de poses menos frequente para entidades distantes com interpolação visual.
- Culling de animações invisíveis sem afetar eventos lógicos ou combate.
- Cache de poses/curvas compartilháveis com invalidação explícita.
- Skinning na GPU versus CPU com benchmark por perfil; limitar bones/influences por qualidade.
- Processamento em lotes e workers quando custo de sincronização for compensado.
- Limites adaptativos para partículas e efeitos cosméticos sem alterar hitboxes ou eventos.

## Invariantes
Hit registration, posição autoritativa, timing de habilidades, root motion, sockets, IK e blending precisam permanecer corretos. Animação visual pode ser simplificada; estado de gameplay não. Verificar bone matrices, deformações e sincronização com cenas golden.
