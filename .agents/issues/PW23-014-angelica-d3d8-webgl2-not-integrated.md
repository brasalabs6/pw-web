---
id: PW23-014
title: "Backend da Angelica original ainda não desenha os frames do jogo"
priority: P0
status: open
kind: review-finding
first_observed: 2026-10-27
verified_against: e0cf055be10f9bab95aa9429c90f977c5cd00c8f
epic: e06-render-webgl2-v1
scope: pr-23
---

# PW23-014 — Backend da Angelica original ainda não desenha os frames do jogo

**Status:** `open` para triagem no HEAD local. Registro da revisão: a evidência descreve código e testes vistos, não constitui confirmação de que o defeito persiste em commits posteriores.

## Evidência verificável
`platform/web/runtime.cpp` implementa viewer GLSL MOX/DDS e câmera orbital; target `client153_engine_original` OBJECT não está linkado ao browser game; docs evidenciam rotas PURESERVER/HWI com falso sucesso sem desenho.

## Impacto / por que importa
Não há equivalência da pipeline D3D8 Angelica: buffers/FVF, estados, blending, fog, iluminação, skinning e render de mundo.

## Solução proposta
Conectar A3DEngine/A3DDevice/A3DStream/A3DSkinModel ao backend WebGL2 sem duplicar engine; implementar estados usados e comparar frames com baseline Win32/Wine.

## Testes e critérios de aceite
Original Angelica produz pixels de cena/modelo animado e mundo via seu call graph; testes excluem caminhos que retornam sucesso sem draw.

## Rastreabilidade e execução
- PR [#23](https://github.com/brasalabs6/pw-web/pull/23), baseline `e0cf055be10f9bab95aa9429c90f977c5cd00c8f`.
- [Review arquitetural inicial](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6451050045) e [review incremental](https://github.com/brasalabs6/pw-web/pull/23#pullrequestreview-5676905878).
- Épico relacionado: [`e06-render-webgl2-v1`](../epics/e06-render-webgl2-v1/README.md).
- Resolução requer revalidação no HEAD, registro do commit de correção, comando, ambiente e evidência dos testes. **Não apagar** este arquivo ao corrigir; atualizar `status: resolved` ou `wont_fix` com justificativa e SHA.
- Não abrir PR adicional: observar `AGENTS.md` (PR único do cliente e merge humano).

## Revalidação 2026-10-29 — Present ainda impede mostrar os pixels da cena

**HEAD de código inspecionado:** `982ad80682ba866a175ce1e75051a14a0b178ede` ([commit](https://github.com/brasalabs6/pw-web/commit/982ad80682ba866a175ce1e75051a14a0b178ede)). O commit relata que `Present #1` foi alcançado no probe Chromium, mas o canvas permaneceu vazio. Trata-se de **relato do executor**, não de execução independente desta auditoria.

**Achado determinístico no código:** em [`d3d8_device.cpp:106–118`](https://github.com/brasalabs6/pw-web/blob/982ad80682ba866a175ce1e75051a14a0b178ede/src/client153/platform/web/d3d8_device.cpp#L106-L118), `Present` faz `glBlitFramebuffer` do framebuffer da cena para o canvas **e logo em seguida** chama `glClearColor(0,0,0.3,1)` e `glClear(GL_COLOR_BUFFER_BIT)` com o framebuffer padrão ainda vinculado para desenho. Com scissor desabilitado e máscara de cor gravável, o clear substitui os pixels recém-copiados por azul escuro. Portanto, **mesmo que a Angelica desenhe corretamente no framebuffer de origem, este caminho não pode apresentar sua imagem final**. É uma sobreposição de diagnóstico intencional, mas impede o critério de aceite de renderização original. O canvas vazio relatado não é explicado *necessariamente* por isso: o worker/OffscreenCanvas pode ter outro problema simultâneo.

Além disso, [`DrawPrimitive/DrawPrimitiveUP`](https://github.com/brasalabs6/pw-web/blob/982ad80682ba866a175ce1e75051a14a0b178ede/src/client153/platform/web/d3d8_device.cpp#L226-L255) ainda não conectam layout FVF, atributos e VBO ao shader; `DrawIndexedPrimitive` não liga o index buffer em [linhas 234–240](https://github.com/brasalabs6/pw-web/blob/982ad80682ba866a175ce1e75051a14a0b178ede/src/client153/platform/web/d3d8_device.cpp#L234-L240). Isso reforça que o backend gráfico ainda é parcial; **não** é prova de erro GL reproduzido.

**Ação de port (não tuning):** remover/condicionar o clear de diagnóstico fora do fluxo de apresentação normal, verificar `glGetError`/estado de framebuffer e capturar o resultado visível do blit com um padrão de cores controlado. Depois conectar atributos/índices/FVF e testar uma primitiva original da Angelica; comparar pixels antes/depois e verificar no Chrome desktop com e sem worker. Validar celular real separadamente. Não declarar a issue resolvida até a cena original aparecer, sem bypass de assets.

**Deduplicação:** o problema fica neste **PW23-014** porque pertence exatamente ao blocker existente de integração D3D8/WebGL2. Não criar PW23-042 para o mesmo subsistema. As falhas de `elements.data` e `Login.ecwld` continuam separadas em PW23-040/041. Não foram executados benchmarks ou testes de navegador/mobile nesta inspeção.

## Complemento de contrato (29/10) — surfaces do canvas e fixed function

**Novo bug de boundary no HEAD `1951ded5dc37c32451f9cca5a6b4a3001bef420e`:** `platform/web/d3d8_device.cpp::SetRenderTarget` vincula `framebuffer`, que em `pw153_create_web_device` está fixado a **0**, e quando `target != nullptr` tenta executar `glFramebufferRenderbuffer(GL_COLOR_ATTACHMENT0, ...)` no framebuffer padrão. O framebuffer 0 é o **canvas do browser**, não um FBO criado pelo jogo com attachments configuráveis. Criar `PW153Surface` com renderbuffer zero **não basta**. Implementar wrappers `BackbufferSurface` e `DepthSurface` com identidade de default framebuffer, `GetDesc`, refs COM e dimensões válidas; ao selecionar o wrapper default, vincular FBO 0 sem anexos; offscreen requer FBO/storage próprios. `Clear` deve respeitar o target ativo, hoje ele vincula sempre FBO 0.

**Outro bloqueio determinístico:** `DrawPrimitive` recebe `primitiveCount` D3D8 mas o passa como número de vértices a `glDrawArrays`; `DrawIndexedPrimitive` usa `count*3` para todas as topologias, `SetIndices` é `E_NOTIMPL`, e as chamadas ainda não configuram VAO/atributos FVF. O shader `gl_Position=a_position` não converte as coordenadas em pixels dos vértices `XYZRHW` para NDC. Não há prova de geometria original desenhada apenas com um `Clear`.

**Implementação escalável sem inventar engine nova:** fixed-function emulado **incrementalmente** por variantes de pipeline FVF+estado (transform, textura, blending, alpha, fog, luzes conforme traços de uso); começar com `XYZRHW|DIFFUSE|TEX1`, quads/textos da AUI e teste de readPixels, depois mundo/skin. Critérios: canvas com draw real e pixel check, refs de surfaces, FBO0 vs offscreen, formato/backbuffer em resize e topologia correta para primitivas D3D. [Análise de arquitetura no PR](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6499021552).

## Revalidação incremental 29/10/2026 (13:40–13:41) — D3DX8 é bloqueador funcional explícito de texturas

**Origem da evidência:** trecho de `/Logs/A3D.log` enviado pelo operador (127 linhas anteriores omitidas), com centenas de erros de `A3DTexture::LoadNormalTexture, Can't load`, `D3DXCreateTextureFromFileInMemoryEx()` e `CECFace::InitPartShaders`. `pw153_browser_probe=0` nesta execução comprova encerramento de `wWinMain` com 0, **não** gameplay/visual válido. Sem execução independente e sem comprovação de identidade SHA da build local.

**Achado determinístico no HEAD `7c3f67a0f9aa3c7982a0ef1542bdb6cf1f8e6c1f`:** `src/client153/platform/common/d3dx8.h:183-208` define `D3DXCreateTextureFromFileInMemoryEx` retornando incondicionalmente `D3DERR_INVALIDCALL`; `d3dx8.h:313-327` faz o mesmo para `D3DXGetImageInfoFromFileInMemory`, com `D3DXIMAGE_INFO` zerado. Essas funções são chamadas por `Angelica2/Angelica3D/Source/A3DTexture.cpp:511+,595+`. Nesse código, nos branches de erro a engine pode retornar `true` com `m_pDXTexture=NULL`, portanto sucesso do caller não comprova textura criada. `platform/web/d3d8_texture.cpp` contém upload parcial para texturas A8R8G8B8/A4R4G4B4 com `levels==1`, não implementa os dois contratos D3DX, DDS mip-chain e todos os formatos exigidos.

**Impacto:** mesmo com arquivo binário original presente, o backend sempre falha no caminho D3DX de decodificação/texture creation. Separar do erro de `AFileImage::Open` para file missing/path e do encoding GB18030 (PW23-012). O texto chinês substituído por `����` também pode ser artefato do dump UTF-8 de bytes GB18030; capturar bytes hex antes de inferir corrupção do VFS.

**Plano técnico:** implementar `D3DXGetImageInfoFromFileInMemory` e `D3DXCreateTextureFromFileInMemoryEx` sobre leitura/decodificação real e `IDirect3DDevice8::CreateTexture/LockRect/UnlockRect`; priorizar fixtures autorizadas DDS BGRA, DXT1/3/5 com fallback de CPU quando WebGL2 não tiver S3TC, mipmaps, e posteriormente TGA/BMP/JPG/PNG usados pelo cliente. Distinguir: arquivo localizado → header parse válido → conversão de formato → texture upload GL `glGetError` → ponteiro não nulo. Manter falha explícita para combinações não implementadas; não inventar dummy textures como PASS. Verificar `D3DXGetImageInfoFromFileInMemory` em qualquer TU que compile esse header.

**Critério de aceite:** teste nativo/WASM e Chromium com textura real lida do VFS; `D3DXIMAGE_INFO` verdadeiro, `IDirect3DTexture8*` não nulo com desc/mips e pixels texturizados via readPixels/captura do draw original; caso negativo deve falhar com diagnóstico distinto. Primeiro frame original e AUI/login não podem depender de clear JS. [Comentário técnico no PR](https://github.com/brasalabs6/pw-web/pull/23#issuecomment-6502786138).
