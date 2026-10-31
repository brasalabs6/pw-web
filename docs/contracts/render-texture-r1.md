# R1 — proposed RGBA8 texture and sampler contract

**DRAFT / SPEC ONLY** for e06 WO-07/08/09/11. This is a synthetic WebGL2
contract, not an Angelica texture decoder. P2 must pass pixel/lifecycle tests
and the required workorder dependencies before implementation is accepted.
See [capabilities](render-capabilities.md) and [R0](../../.agents/epics/e06-render-webgl2-v1/contracts/render-r0.md).

## Data and ownership

Suggested internal C++17 request fields: width and height (uint32), row_stride
(uint32 bytes), a read-only byte span and a SamplerR1 enum. This is **not**
a JS ABI, source-asset format or D3D-compatible binary struct.

- Format: exactly uncompressed RGBA8, byte order R,G,B,A. No BGRA, DDS, PCK,
  compressed formats, mipmaps, SRGB conversion, color-key or guessed legacy fields.
- Width/height: integers from 1 to the actual GL_MAX_TEXTURE_SIZE.
  Validate multiplication with checked size_t arithmetic; no unsigned wrap.
- Stride >= 4 * width and exact source size =
  (height - 1) * stride + 4 * width. Extra or truncated bytes are InvalidInput;
  explicit padding between rows is permitted but never copied into texture.
- Caller owns the input bytes until the synchronous upload/update returns.
  Backend stages contiguous 4*width*height bytes (checked allocation) and
  uploads with GL_UNPACK_ALIGNMENT=1, GL_RGBA8/GL_RGBA/GL_UNSIGNED_BYTE.
  Save and restore previous unpack alignment and texture binding.
- Input rows start at **top-left**, advancing downward. GLSL UV convention is
  u=0 left, v=0 **bottom**, v=1 top. CPU upload inverts source row order before
  glTexImage2D; no hidden UNPACK_FLIP_Y_WEBGL. Renderer never flips the screen Y.
- Supported sampler at R1 scope: GL_NEAREST for min and mag,
  GL_CLAMP_TO_EDGE for S and T, no mipmaps. Any other sampler/format is
  UnsupportedFeature (never silently downgraded).
- Successful creation publishes an opaque owner/generation handle only after
  complete allocation/upload and GL error checks. On failure free partial GPU
  resources and leave the caller's resource state untouched. Update is explicit,
  rectangular and bounds checked, with the same orientation and source stride.
  Updates must not change pixels outside the specified rectangle.
- Dispose on a valid already-disposed handle of the same generation is
  idempotent; unknown/stale handle is InvalidInput. Context loss invalidates
  every texture handle, prohibits updates/draw and requires explicit recreation
  after restore. No automatic resurrection of copyrighted assets.

## Canonical 2x2 RGBA8 fixture

CPU input byte sequence is four consecutive RGBA pixels, stride 8, length 16.

| Input row | Pixel 0 | Pixel 1 |
| --- | --- | --- |
| top (row 0) | red [255,0,0,255] | green [0,255,0,255] |
| bottom (row 1) | blue [0,0,255,255] | yellow [255,255,0,128] |

Draw a unit quad with nearest/clamp and no blend/depth interference.
Expected texture-center samples:

| UV sample | Expected pixel | GL readback coordinate on 2x2 target |
| --- | --- | --- |
| (0.25, 0.75) | red | (0,1) |
| (0.75, 0.75) | green | (1,1) |
| (0.25, 0.25) | blue | (0,0) |
| (0.75, 0.25) | yellow, alpha 128 | (1,0) |

Use tolerance <=2 per channel, check *all* four quadrants and assert top/bottom
are not exchanged. Screenshots/HTTP responses alone are not oracle evidence.
Also update only bottom-right 1x1 to magenta and assert other three untouched.

## Candidate pipeline extensions; require original mapping before implementation

R0 fixed state is opaque, blending/culling/alpha-test disabled, depth LEQUAL
with writes enabled. Restrict a future approved R1 state set to
Blend {Opaque, SrcAlphaOneMinusSrcAlpha}; Cull {Disabled, BackCCW};
Depth {Disabled, LequalWrite, LequalReadOnly}; and
AlphaTest {Disabled, GreaterEqualByteReference 0..255}, implemented with
explicit shader discard before depth write. Alpha-test is not equivalent to
blend. GL_CCW means front-facing; GL_BACK is culled for BackCCW. Each draw
sets **all** states explicitly; unsupported combinations or enums return
UnsupportedFeature before rendering. Actual compatibility with Angelica
materials is **BLOCKED** until WO-02 mapping and WO-09/11 test coverage.

## Required negative fixtures and status

- InvalidInput: width/height zero; width exceeds context limit; stride below
  width*4; arithmetic overflow; bytes too short/long; update rect out of
  bounds; stale/unknown texture handle; invalid matrix or vertices at draw.
- UnsupportedFeature: BGRA/DDS; Linear/Repeat/mipmap; unknown blend/cull/depth
  or shader material variants.
- ContextLost: create/update/draw while lost; handles stale after restore.
- GpuError: create/compile/link/upload failure; no published half-initialized
  texture or shader program.
- Positive: four colors in quadrants, alpha preserved, update rect,
  dispose/recreate and loss/restore explicit. Browser, GL version, renderer,
  2x2 bytes and case counts must be included in actual future test evidence.

**Tests executed for R1: NOT RUN (0 cases).** No WO status, schedule or
completion flag is changed by this proposal.
