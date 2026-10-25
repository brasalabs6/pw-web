# Render capability matrix — R0 implementation / R1 proposal

Status 2026-10-25: R0 P1 native + WASM reported PASS in previous evidence;
P2 C++/GLES3 is implemented but **its own WASM build and browser tests are NOT RUN**.
R1 is a specification proposal. This document does not complete e06 workorders.

Contract of record: [R0](../../.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md).

| Operation | Current R0 | R1 candidate | Required verification |
| --- | --- | --- | --- |
| 16-byte VertexPC, indexed uint16 triangle list | P1 validated | Retain | Native/WASM |
| Row-major transform and D3D -> GL depth | P1 validated, GPU unproven | Retain | Pixel/translation |
| ARGB channel extraction | P1 validated, GPU unproven | Retain | RGBA8 pixel |
| Depth LEQUAL/write on, blending/culling off | Backend implemented, not browser-proven | Retain | Draw order |
| Clear/read RGBA8, context loss/restore/dispose | Backend implemented, not browser-proven | Retain | Chrome proof |
| Texture RGBA8 nearest/clamp and UV | UnsupportedFeature | Proposed | R1 2x2 fixture |
| Blend, alpha-test and culling | UnsupportedFeature | Enum-restricted proposal only | Original mapping + browser fixture |
| Offscreen RGBA8/depth16 | UnsupportedFeature | Proposed | FBO/resize fixture |
| Fog, lighting, skinning, DDS and D3D shader translation | UnsupportedFeature | UnsupportedFeature | Later approved scope |
| Original Angelica mesh/material/asset/protocol | Not available | Not part of synthetic R1 | Authorized source/baseline |

R0 status encoding: **Ok 0**, **InvalidInput 1**, **UnsupportedFeature 2**,
**ContextLost 3**, **GpuError 4**. No feature request may silently map to an
empty draw or a visually approximate fallback. Unknown enum is UnsupportedFeature.
Failed pointer/count/stride/viewport/index validation is InvalidInput before
GL upload. GPU initialization, shader, buffer and framebuffer failure is GpuError;
lost context is ContextLost. Zero geometry emitted does not count as success.

Consult the real context's GL_MAX_VIEWPORT_DIMS, GL_MAX_TEXTURE_SIZE and
GL_MAX_RENDERBUFFER_SIZE as relevant; reject rather than clamp. GPU resources
have an explicit owner and generation; partial initialization is rolled back,
context loss invalidates all handles, restoration requires reinitialization.
Do not silently reuse state between draws or restore stale texture contents.

Evidence: [render R0](../evidence/render-r0.md). Complete P2 requires
bash scripts/probe-render-browser.sh after official Emscripten build, passing
both the exact 10-group positive and deliberate negative control. R1 contract
references: [RGBA8 sampler](render-texture-r1.md) and
[offscreen target](render-target-r1.md). No result of these proposed R1
fixtures is represented as an executed test.
