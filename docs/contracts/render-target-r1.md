# R1 — proposed offscreen render-target contract

**DRAFT / SPEC ONLY** for e06 WO-10/12. No GPU implementation, proof,
or completion claim. This is a synthetic WebGL2 target, **not** the original
Angelica backbuffer. Dependencies: [capabilities](render-capabilities.md),
[RGBA8 texture](render-texture-r1.md), validated R0 P2.

## Attachment and lifecycle

- RenderTargetR1 request: width and height as uint32, each >=1 and <=
  the smallest applicable runtime GL_MAX_TEXTURE_SIZE and
  GL_MAX_RENDERBUFFER_SIZE. Validate 4*width*height with checked arithmetic.
- Color attachment: GL_RGBA8 texture (GL_RGBA/GL_UNSIGNED_BYTE).
  Depth attachment: GL_DEPTH_COMPONENT16 renderbuffer. No stencil, MRT, MSAA,
  SRGB, compressed format, or implicit resolve in this first subset.
- Bind color and depth to a new framebuffer; only publish a handle if
  glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
  with no GL error. Any other status is GpuError; dispose partial GPU
  objects while preserving previously committed target.
- Creation and resize are **two-phase commit** operations: validate request,
  allocate/check independent new attachments, then atomically switch owner
  handle and release old objects. InvalidInput/GpuError must preserve the
  prior target, its content, and previous GL bindings/viewport.
- Clear explicitly sets RGBA (0,0,0,255) and depth 1; readback returns RGBA8
  bytes from glReadPixels, GL_PACK_ALIGNMENT=1, origin bottom-left, row stride
  exactly width*4. Bounds/output capacity checked before mutating GL.
- Save and restore GL_DRAW_FRAMEBUFFER_BINDING, GL_READ_FRAMEBUFFER_BINDING,
  viewport, pixel pack alignment, depth mask and clear color/depth values
  touched by these operations. Do not leak offscreen state to default canvas.
- The backend owns the texture/renderbuffer/framebuffer. Dispose is idempotent
  for its valid already-disposed handle. Unknown or stale generation returns
  InvalidInput. Context loss invalidates attachments, and restored context
  requires explicit reinitialization/recreation (no old GLuint reuse).

## Required golden fixtures

| Case | Action | Expected |
| --- | --- | --- |
| 1 | Create 2x2, clear black/depth1, read (0,0) and (1,1) | RGBA [0,0,0,255] at both |
| 2 | Draw four quadrants with RGBA8 R1 input | At GL (0,1)=red, (1,1)=green, (0,0)=blue, (1,0)=yellow A128 |
| 3 | Resize 2x2 to 4x4, clear, read | Complete FBO, correct 4x4 output and clear; old content not implicitly preserved |
| 4 | Force incompatible attachment / zero / overflow / oversize | Non-Ok, old attachment/content/bindings/viewport intact |
| 5 | WEBGL_lose_context and restore | Draw/read in Lost => ContextLost; stale handle rejected; explicit reconstruction works |
| 6 | Dispose twice, create new target | No GL error, no stale handle reuse |

Each pixel channel accepts absolute error <=2; assert exact 16-byte
output length for 2x2 and exact 64-byte length for 4x4. Negative cases
must inspect explicit statuses, framebuffer completeness and restoration,
not just count test callbacks. Record driver/browser/version and context
attributes; SwiftShader is software, not a benchmark of GPU throughput.

## Status and prerequisite gates

InvalidInput for zero/oversize/overflow/truncated readback buffer or bad
rectangle; UnsupportedFeature for unlisted attachment/samples/formats;
ContextLost for any work during loss; GpuError for creation/incomplete FBO/
GL error. No silent fallback to default framebuffer or implicit depth omission.

**Execution: NOT RUN.** WO-10 remains uncompleted until prerequisites,
implementation, real WebGL2 readback, negative tests, independent review and
manual gates are satisfied. This contract does not redefine Angelica formats.
