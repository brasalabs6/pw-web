#include "d3d8_surface.h"
#include "d3d8_buffer.h"
#include "d3d8_texture.h"
#include <emscripten/html5.h>
#include <new>
#include <cstdio>

namespace {
// This first device owns real color/depth buffers and supports full clears.
// Unsupported device operations return E_NOTIMPL; it is not a full renderer.
class Device final : public IDirect3DDevice8 {
  ULONG refs = 1;
  PW153Surface *color, *depth;
  GLuint framebuffer;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
  UINT backWidth = 0, backHeight = 0;
  // Rendering pipeline state (the D3D8 fixed-function pipeline emulation).
  IDirect3DVertexBuffer8 *streamSource = nullptr;
  UINT streamOffset = 0;
  IDirect3DIndexBuffer8 *indexBuffer = nullptr;
  UINT indexOffset = 0;
  DWORD currentFVF = 0;
  DWORD srcBlend = 2, dstBlend = 1;
  IDirect3DBaseTexture8 *textures[8] = {};
  // The transform states: view (2), projection (3), texture (16-23) and
  // the world matrices (D3DTS_WORLDMATRIX(n) = 256+n, the game's
  // SetTransform(D3DTS_WORLD) lands at 256). The old 32-entry array
  // silently dropped the world matrix — SetTransform returned success
  // without storing it.
  D3DMATRIX transforms[264] = {};
  // The texture-stage states the game sets (address modes and filters
  // per stage; stage 0 is the one the draws bind).
  struct TextureStage {
    DWORD addressU = 1, addressV = 1;  // D3DTADDRESS_WRAP
    DWORD magFilter = 1, minFilter = 1;  // D3DTEXF_POINT
  } textureStages[8];
  // The default framebuffer attachments (the canvas), created once and
  // handed out by the surface getters.
  PW153DefaultSurface *defaultColor = nullptr;
  PW153DefaultSurface *defaultDepth = nullptr;
  // The FVF shader program (created lazily on the first draw call).
  GLuint shaderProgram = 0;
  GLint aPosition = -1, aColor = -1, aTexCoord = -1, uViewport = -1, uTextured = -1;
  GLint uTransformed = -1, uWorld = -1, uView = -1, uProjection = -1;
public:
  Device(PW153Surface *c, PW153Surface *d, GLuint f, EMSCRIPTEN_WEBGL_CONTEXT_HANDLE owner,
         UINT w, UINT h)
    : color(c), depth(d), framebuffer(f), context(owner), backWidth(w), backHeight(h) {
    // The default framebuffer attachments (the canvas): the colour format
    // is the real RGBA8 backend (alpha is present), the depth matches the
    // client's chosen D24X8 stencil-less request.
    defaultColor = new PW153DefaultSurface(w, h, D3DFMT_A8R8G8B8);
    defaultDepth = new PW153DefaultSurface(w, h, D3DFMT_D24X8);
  }
  ~Device() override {
    if (defaultColor) defaultColor->Release();
    if (defaultDepth) defaultDepth->Release();
  }
  HRESULT QueryInterface(const void *, void **out) override { if (out) *out = nullptr; return E_NOINTERFACE; }
  ULONG AddRef() override { return ++refs; }
  ULONG Release() override { auto count = --refs; if (!count) delete this; return count; }
  // The getters hand out real surface objects for the canvas attachments
  // (never D3D_OK with a null out pointer — the A3DDevice::Init contract
  // requires non-null render target, depth stencil and back buffer).
  HRESULT GetRenderTarget(IDirect3DSurface8 **out) override {
    if (!out) return E_FAIL;
    defaultColor->AddRef();
    *out = defaultColor;
    return D3D_OK;
  }
  HRESULT GetDepthStencilSurface(IDirect3DSurface8 **out) override {
    if (!out) return E_FAIL;
    defaultDepth->AddRef();
    *out = defaultDepth;
    return D3D_OK;
  }
  HRESULT GetBackBuffer(UINT index, DWORD type, IDirect3DSurface8 **out) override {
    if (index || type) { if (out) *out = nullptr; return E_FAIL; }
    return GetRenderTarget(out);
  }
  HRESULT Clear(DWORD count, const D3DRECT *rects, DWORD flags, DWORD value, float z, DWORD) override {
    static int clearCount = 0;
    if (clearCount < 5) {
      printf("pw153: Clear #%d flags=0x%x color=0x%08x (A=%d R=%d G=%d B=%d) z=%.2f\n",
             ++clearCount, flags, value,
             (value >> 24) & 255, (value >> 16) & 255, (value >> 8) & 255, value & 255, z);
    }
    if (count || rects || !flags || flags & ~(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER) ||
        emscripten_webgl_get_current_context() != context ||
        emscripten_is_webgl_context_lost(context)) return E_FAIL;
    GLint previous = 0; GLfloat oldColor[4], oldDepth; GLboolean oldMask[4], oldDepthMask;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previous); glGetFloatv(GL_COLOR_CLEAR_VALUE, oldColor);
    glGetFloatv(GL_DEPTH_CLEAR_VALUE, &oldDepth); glGetBooleanv(GL_COLOR_WRITEMASK, oldMask);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &oldDepthMask);
    const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer); glDisable(GL_SCISSOR_TEST);
    GLbitfield mask = 0;
    if (flags & D3DCLEAR_TARGET) { glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      //\tForce alpha to 1 (opaque): the D3D8 back buffer is opaque but the
      //\tclient passes A=0 in the clear color. Without this, the canvas
      //\tshows the page background (white) through the transparent pixels.
      glClearColor(((value >> 16) & 255) / 255.0f, ((value >> 8) & 255) / 255.0f,
                   (value & 255) / 255.0f, 1.0f); mask |= GL_COLOR_BUFFER_BIT; }
    if (flags & D3DCLEAR_ZBUFFER) { glDepthMask(GL_TRUE); glClearDepthf(z); mask |= GL_DEPTH_BUFFER_BIT; }
    glClear(mask);
    glClearColor(oldColor[0], oldColor[1], oldColor[2], oldColor[3]); glClearDepthf(oldDepth);
    glColorMask(oldMask[0], oldMask[1], oldMask[2], oldMask[3]); glDepthMask(oldDepthMask);
    if (scissor) glEnable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(previous));
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT CopyRects(IDirect3DSurface8 *source, const RECT *rects, UINT count,
                    IDirect3DSurface8 *destination, const POINT *points) override {
    if (!source || !destination || source == destination || (!rects && count)) return E_FAIL;
    D3DSURFACE_DESC src{}, dst{};
    if (source->GetDesc(&src) != D3D_OK || destination->GetDesc(&dst) != D3D_OK || src.Format != dst.Format) return E_FAIL;
    unsigned bytes = 0;
    switch (src.Format) {
      case D3DFMT_A8R8G8B8: case D3DFMT_X8R8G8B8: bytes = 4; break;
      case D3DFMT_A4R4G4B4: case D3DFMT_A1R5G5B5: case D3DFMT_R5G6B5: bytes = 2; break;
      default: return E_FAIL;
    }
    const UINT total = rects ? count : 1;
    for (UINT i = 0; i < total; ++i) {
      RECT r = rects ? rects[i] : RECT{0, 0, static_cast<LONG>(src.Width), static_cast<LONG>(src.Height)};
      POINT p = points ? points[i] : POINT{0, 0};
      if (r.left < 0 || r.top < 0 || r.right <= r.left || r.bottom <= r.top ||
          r.right > static_cast<LONG>(src.Width) || r.bottom > static_cast<LONG>(src.Height) ||
          p.x < 0 || p.y < 0 || static_cast<uint64_t>(p.x) + (r.right - r.left) > dst.Width ||
          static_cast<uint64_t>(p.y) + (r.bottom - r.top) > dst.Height) return E_FAIL;
    }
    for (UINT i = 0; i < total; ++i) {
      RECT r = rects ? rects[i] : RECT{0, 0, static_cast<LONG>(src.Width), static_cast<LONG>(src.Height)};
      POINT p = points ? points[i] : POINT{0, 0};
      RECT target{p.x, p.y, p.x + r.right - r.left, p.y + r.bottom - r.top};
      D3DLOCKED_RECT from{}, to{};
      if (source->LockRect(&from, &r, D3DLOCK_READONLY) != D3D_OK) return E_FAIL;
      if (destination->LockRect(&to, &target, 0) != D3D_OK) { source->UnlockRect(); return E_FAIL; }
      for (LONG y = 0; y < r.bottom - r.top; ++y)
        std::memcpy(static_cast<BYTE *>(to.pBits) + size_t(y) * to.Pitch,
                    static_cast<const BYTE *>(from.pBits) + size_t(y) * from.Pitch,
                    static_cast<size_t>(r.right - r.left) * bytes);
      HRESULT a = destination->UnlockRect(), b = source->UnlockRect();
      if (a != D3D_OK || b != D3D_OK) return E_FAIL;
    }
    return D3D_OK;
  }
  HRESULT GetRenderState(DWORD, DWORD *) override { return E_NOTIMPL; }
  HRESULT SetTexture(DWORD stage, IDirect3DBaseTexture8 *texture) override {
    if (stage >= 8) return E_FAIL;
    if (texture) texture->AddRef();
    if (textures[stage]) textures[stage]->Release();
    textures[stage] = texture;
    return D3D_OK;
  }
  HRESULT GetClipPlane(DWORD, float *) override { return E_NOTIMPL; }
  HRESULT SetClipPlane(DWORD, const float *) override { return E_NOTIMPL; }
  HRESULT CreateRenderTarget(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE, BOOL, IDirect3DSurface8 **surface) override {
    if (!surface) return E_FAIL;
    *surface = pw153_create_surface(width, height, format);
    return *surface ? D3D_OK : E_FAIL;
  }
  HRESULT CreateDepthStencilSurface(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE, IDirect3DSurface8 **surface) override {
    if (!surface) return E_FAIL;
    *surface = pw153_create_surface(width, height, format);
    return *surface ? D3D_OK : E_FAIL;
  }
  HRESULT CreateTexture(UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture8 **out) override {
    if (!out) return E_FAIL;
    *out = nullptr;
    if (emscripten_webgl_get_current_context() != context || emscripten_is_webgl_context_lost(context)) return E_FAIL;
    *out = pw153_create_texture(this, width, height, levels, usage, format, pool);
    return *out ? D3D_OK : E_FAIL;
  }
  HRESULT CreateCubeTexture(UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DCubeTexture8 **cubeTexture) override { if (cubeTexture) *cubeTexture = nullptr; return E_NOTIMPL; }
  HRESULT CreateImageSurface(UINT width, UINT height, D3DFORMAT format, IDirect3DSurface8 **surface) override {
    if (!surface) return E_FAIL;
    *surface = pw153_create_image_surface(width, height, format);
    return *surface ? D3D_OK : E_FAIL;
  }
  HRESULT TestCooperativeLevel() override { return E_NOTIMPL; }
  HRESULT BeginScene() override { return D3D_OK; }
  HRESULT EndScene() override { return D3D_OK; }
  HRESULT Present(const RECT *, const RECT *, HWND, const void *) override {
    // Rendering goes directly to the canvas's default framebuffer
    // (framebuffer 0). No blit or swap is needed — the canvas already
    // displays the rendered content.
    return D3D_OK;
  }
  HRESULT Reset(D3DPRESENT_PARAMETERS *) override { return E_NOTIMPL; }
  HRESULT SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX *matrix) override {
    if (!matrix) return E_FAIL;
    if (state >= 0 && state < 264) {
      memcpy((void*)&transforms[state], matrix, sizeof(D3DMATRIX));
    }
    return D3D_OK;
  }
  HRESULT SetVertexShader(DWORD fvf) override {
    currentFVF = fvf;
    return D3D_OK;
  }
  HRESULT SetRenderState(DWORD state, DWORD value) override {
    if (emscripten_webgl_get_current_context() != context || emscripten_is_webgl_context_lost(context)) return E_FAIL;
    if (state == D3DRS_ALPHABLENDENABLE) {
      if (value) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    } else if (state == D3DRS_SRCBLEND || state == D3DRS_DESTBLEND) {
      static const GLenum factors[] = {0, GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
                                      GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA,
                                      GL_ONE_MINUS_DST_ALPHA, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR,
                                      GL_SRC_ALPHA_SATURATE};
      if (value < 1 || value > 11 || (state == D3DRS_DESTBLEND && value == 11)) return E_FAIL;
      const DWORD nextSrc = state == D3DRS_SRCBLEND ? value : srcBlend;
      const DWORD nextDst = state == D3DRS_DESTBLEND ? value : dstBlend;
      glBlendFunc(factors[nextSrc], factors[nextDst]);
      if (glGetError() != GL_NO_ERROR) return E_FAIL;
      srcBlend = nextSrc; dstBlend = nextDst;
    } else if (state == D3DRS_CULLMODE) {
      // D3DCULL_NONE=1 (no culling), D3DCULL_CW=2 (front faces wind CW —
      // cull the CCW ones), D3DCULL_CCW=3 (front faces wind CCW). The
      // device's front-face convention is CW (set with the shader), so
      // culling CW faces means glFrontFace(GL_CCW).
      switch (value) {
        case D3DCULL_NONE: glDisable(GL_CULL_FACE); break;
        case D3DCULL_CW:
          glEnable(GL_CULL_FACE); glFrontFace(GL_CCW); glCullFace(GL_BACK); break;
        case D3DCULL_CCW:
          glEnable(GL_CULL_FACE); glFrontFace(GL_CW); glCullFace(GL_BACK); break;
        default: return E_FAIL;
      }
      if (glGetError() != GL_NO_ERROR) return E_FAIL;
    } else return E_NOTIMPL;
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  // The texture-stage states the game sets: the address modes and the
  // sampler filters. Stored per stage; stage 0's states apply to the
  // bound texture at draw time.
  HRESULT SetTextureStageState(DWORD stage, DWORD type, DWORD value) override {
    if (stage >= 8) return E_FAIL;
    TextureStage &s = textureStages[stage];
    switch (type) {
      case D3DTSS_ADDRESSU: s.addressU = value; return D3D_OK;
      case D3DTSS_ADDRESSV: s.addressV = value; return D3D_OK;
      case D3DTSS_MAGFILTER: s.magFilter = value; return D3D_OK;
      case D3DTSS_MINFILTER: s.minFilter = value; return D3D_OK;
      default: return E_NOTIMPL;
    }
  }
  HRESULT GetTextureStageState(DWORD stage, DWORD type, DWORD *value) override {
    if (!value || stage >= 8) return E_FAIL;
    const TextureStage &s = textureStages[stage];
    switch (type) {
      case D3DTSS_ADDRESSU: *value = s.addressU; return D3D_OK;
      case D3DTSS_ADDRESSV: *value = s.addressV; return D3D_OK;
      case D3DTSS_MAGFILTER: *value = s.magFilter; return D3D_OK;
      case D3DTSS_MINFILTER: *value = s.minFilter; return D3D_OK;
      default: return E_NOTIMPL;
    }
  }
  HRESULT SetLight(DWORD, const D3DLIGHT8 *) override { return E_NOTIMPL; }
  HRESULT LightEnable(DWORD, BOOL) override { return E_NOTIMPL; }
  HRESULT SetMaterial(const D3DMATERIAL8 *) override { return E_NOTIMPL; }
  HRESULT SetViewport(const D3DVIEWPORT8 *vp) override {
    if (!vp || emscripten_webgl_get_current_context() != context) return E_FAIL;
    glViewport(vp->X, vp->Y, vp->Width, vp->Height);
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT SetStreamSource(UINT stream, IDirect3DVertexBuffer8 *buffer, UINT offset) override {
    // Store the vertex buffer for the next draw call. Only stream 0 is
    // tracked (the original client uses a single stream for FVF data).
    if (stream != 0) return E_FAIL;
    if (buffer) buffer->AddRef();
    if (streamSource) streamSource->Release();
    streamSource = buffer;
    streamOffset = offset;
    return D3D_OK;
  }
  HRESULT CreateVertexBuffer(UINT size, DWORD usage, DWORD, D3DPOOL pool, IDirect3DVertexBuffer8 **out) override {
    if (!out) return E_FAIL;
    *out = nullptr;
    if (emscripten_webgl_get_current_context() != context || emscripten_is_webgl_context_lost(context)) return E_FAIL;
    *out = pw153_create_vertex_buffer(size, usage, pool);
    return *out ? D3D_OK : E_FAIL;
  }
  HRESULT CreateIndexBuffer(UINT size, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DIndexBuffer8 **out) override {
    if (!out) return E_FAIL;
    *out = nullptr;
    if (emscripten_webgl_get_current_context() != context || emscripten_is_webgl_context_lost(context)) return E_FAIL;
    *out = pw153_create_index_buffer(size, usage, format, pool);
    return *out ? D3D_OK : E_FAIL;
  }
  HRESULT SetIndices(IDirect3DIndexBuffer8 *buffer, UINT baseVertexIndex) override {
    // Store the index buffer for the next indexed draw. The D3D8 contract:
    // SetIndices persists until the next SetIndices, and the second
    // parameter is BaseVertexIndex — a vertex base added to every index
    // (not a byte offset); the indexed draw shifts the attribute pointers
    // by baseVertexIndex * stride to map it onto WebGL.
    if (buffer) buffer->AddRef();
    if (indexBuffer) indexBuffer->Release();
    indexBuffer = buffer;
    indexOffset = baseVertexIndex;
    return D3D_OK;
  }
  HRESULT SetPixelShader(DWORD) override { return E_NOTIMPL; }
  HRESULT SetVertexShaderConstant(DWORD, const void *, DWORD) override { return E_NOTIMPL; }
  HRESULT SetPixelShaderConstant(DWORD, const void *, DWORD) override { return E_NOTIMPL; }
  HRESULT CreateVertexShader(const DWORD *, const DWORD *, DWORD *, DWORD) override { return E_NOTIMPL; }
  HRESULT DeleteVertexShader(DWORD) override { return E_NOTIMPL; }
  HRESULT CreatePixelShader(const DWORD *, DWORD *) override { return E_NOTIMPL; }
  HRESULT DeletePixelShader(DWORD) override { return E_NOTIMPL; }
  void SetGammaRamp(DWORD, const D3DGAMMARAMP *) override {
    static bool warned = false;
    if (!warned) { std::fputs("pw-browser: D3D gamma ramp is not implemented\n", stderr); warned = true; }
  }
  HRESULT ResourceManagerDiscardBytes(DWORD) override { return E_NOTIMPL; }
  // The device reports the same implemented-capability subset the factory
  // reports: this device exists only as a HAL on adapter 0 over the current
  // WebGL2 context, and texture/shader caps stay zero until implemented.
  // A3DDevice::ConfigDevice requires this call to succeed to set up the
  // render-state tables.
  HRESULT GetDeviceCaps(D3DCAPS8 *caps) override {
    if (!caps || emscripten_is_webgl_context_lost(context)) return E_FAIL;
    *caps = {};
    caps->DeviceType = D3DDEVTYPE_HAL;
    caps->AdapterOrdinal = 0;
    return D3D_OK;
  }
  // The FVF shader: the sprite FVF (XYZRHW) carries pretransformed screen
  // coordinates; the XYZ FVF goes through the world/view/projection
  // matrices the game sets with SetTransform (uploaded raw: the D3D
  // row-major array read as GL column-major is the transpose the
  // row-vector convention needs). The D3D depth range [0,1] maps to GL's
  // [-1,1] with z' = 2z-1.
  void ensureShaderProgram() {
    if (shaderProgram) return;
    static const char *vsSource =
        "#version 300 es\n"
        "layout(location=0) in vec4 a_position;\n"
        "layout(location=1) in vec4 a_color;\n"
        "layout(location=2) in vec2 a_texcoord;\n"
        "uniform vec4 u_viewport;\n"
        "uniform bool u_transformed;\n"
        "uniform mat4 u_world;\n"
        "uniform mat4 u_view;\n"
        "uniform mat4 u_projection;\n"
        "out vec4 v_color;\n"
        "out vec2 v_texcoord;\n"
        "void main() { if (u_transformed) { float w=1.0/a_position.w; "
        "vec2 p=(a_position.xy-u_viewport.xy)/u_viewport.zw; "
        "gl_Position=vec4((p.x*2.0-1.0)*w,(1.0-p.y*2.0)*w,(a_position.z*2.0-1.0)*w,w); } "
        "else { vec4 clip=u_projection*u_view*u_world*vec4(a_position.xyz,1.0); "
        "gl_Position=vec4(clip.x,clip.y,2.0*clip.z-clip.w,clip.w); } "
        "v_color=a_color.bgra; v_texcoord=a_texcoord; }\n";
    static const char *fsSource =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 v_color;\n"
        "in vec2 v_texcoord;\n"
        "uniform sampler2D u_texture;\n"
        "uniform bool u_textured;\n"
        "out vec4 fragColor;\n"
        "void main() { fragColor = v_color * (u_textured ? texture(u_texture,v_texcoord) : vec4(1.0)); }\n";
    auto compile = [](GLenum type, const char *source) -> GLuint {
      GLuint shader = glCreateShader(type);
      glShaderSource(shader, 1, &source, nullptr);
      glCompileShader(shader);
      GLint status = 0;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
      if (!status) { glDeleteShader(shader); return 0; }
      return shader;
    };
    GLuint vs = compile(GL_VERTEX_SHADER, vsSource);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsSource);
    if (!vs || !fs) { if (vs) glDeleteShader(vs); if (fs) glDeleteShader(fs); return; }
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint linked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (!linked) { glDeleteProgram(shaderProgram); shaderProgram = 0; return; }
    aPosition = glGetAttribLocation(shaderProgram, "a_position");
    aColor = glGetAttribLocation(shaderProgram, "a_color");
    aTexCoord = glGetAttribLocation(shaderProgram, "a_texcoord");
    uViewport = glGetUniformLocation(shaderProgram, "u_viewport");
    uTextured = glGetUniformLocation(shaderProgram, "u_textured");
    uTransformed = glGetUniformLocation(shaderProgram, "u_transformed");
    uWorld = glGetUniformLocation(shaderProgram, "u_world");
    uView = glGetUniformLocation(shaderProgram, "u_view");
    uProjection = glGetUniformLocation(shaderProgram, "u_projection");
    // D3D's front faces wind clockwise in its NDC; with the same NDC
    // orientation in GL the front face stays CW.
    glFrontFace(GL_CW);
  }

  HRESULT DrawPrimitive(DWORD type, UINT startVertex, UINT count) override {
    if (emscripten_webgl_get_current_context() != context || !streamSource) return E_FAIL;
    static const GLenum modes[] = {0, GL_POINTS, GL_LINES, GL_LINE_STRIP,
                                   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    if (type < 1 || type > 6) return E_FAIL;
    uint64_t vertices = CountsForType(type, count);
    auto *buffer = static_cast<PW153VertexBuffer *>(streamSource);
    if (!count || vertices > INT32_MAX || (uint64_t(startVertex) + vertices) * streamOffset > buffer->size()) return E_FAIL;
    if (!PrepareFvfDraw()) return E_FAIL;
    uintptr_t baseOffset = 0;
    if (SetupFvfAttributes(currentFVF, buffer->buffer(), streamOffset, 0, baseOffset) != D3D_OK) return E_FAIL;
    glDrawArrays(modes[type], startVertex, static_cast<GLsizei>(vertices));
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  //	Set up the vertex attributes for the current FVF. The sprite FVF
  //	(XYZRHW|DIFFUSE|SPECULAR|TEX1) is the layout the shader implements
  //	(pretransformed screen coordinates); the XYZ FVF goes through the
  //	world/view/projection pipeline the shader reads from the stored
  //	transforms. Returns the byte offset of the vertex base (the D3D8
  //	SetIndices BaseVertexIndex shifts every index, which WebGL maps to
  //	shifting the attribute pointers). `arrayBuffer` is the caller's
  //	bound vertex storage (the stream source or a temporary UP buffer).
  HRESULT SetupFvfAttributes(DWORD fvf, GLuint arrayBuffer, UINT stride, UINT baseVertexIndex, uintptr_t &baseOffset) {
    const UINT positionSize = (fvf & D3DFVF_XYZRHW) ? 4 : (fvf & D3DFVF_XYZ) ? 3 : 0;
    if (!positionSize) return E_FAIL;
    UINT offset = 0;
    baseOffset = static_cast<UINT>(baseVertexIndex) * stride;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
    glEnableVertexAttribArray(aPosition);
    glVertexAttribPointer(aPosition, positionSize, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void *>(baseOffset));
    offset += positionSize * 4;
    if (fvf & D3DFVF_NORMAL) offset += 12;  // the shader ignores the normal
    if (fvf & D3DFVF_DIFFUSE) {
      glEnableVertexAttribArray(aColor);
      glVertexAttribPointer(aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, reinterpret_cast<const void *>(baseOffset + offset));
      offset += 4;
    } else {
      glDisableVertexAttribArray(aColor);
      glVertexAttrib4f(aColor, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    if (fvf & D3DFVF_SPECULAR) offset += 4;
    if ((fvf & D3DFVF_TEXCOUNT_MASK) >= D3DFVF_TEX1) {
      glEnableVertexAttribArray(aTexCoord);
      glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void *>(baseOffset + offset));
    } else {
      glDisableVertexAttribArray(aTexCoord);
      glVertexAttrib2f(aTexCoord, 0.0f, 0.0f);
    }
    return D3D_OK;
  }
  //	Shared draw preparation: the FVF shader program, the viewport
  //	uniform, the transform matrices (for the XYZ path) and the texture
  //	binding with stage 0's address/filter states. Returns false when the
  //	context or the viewport is unusable.
  bool PrepareFvfDraw() {
    if (emscripten_webgl_get_current_context() != context) return false;
    ensureShaderProgram();
    if (!shaderProgram) return false;
    GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0 || viewport[3] <= 0) return false;
    glUseProgram(shaderProgram);
    glUniform4f(uViewport, viewport[0], viewport[1], viewport[2], viewport[3]);
    const bool transformed = (currentFVF & D3DFVF_XYZRHW) != 0;
    glUniform1i(uTransformed, transformed ? 1 : 0);
    if (!transformed) {
      // The raw D3D row-major arrays read as GL column-major are the
      // transposes the row-vector convention needs.
      glUniformMatrix4fv(uWorld, 1, GL_FALSE, &transforms[D3DTS_WORLD].m[0][0]);
      glUniformMatrix4fv(uView, 1, GL_FALSE, &transforms[D3DTS_VIEW].m[0][0]);
      glUniformMatrix4fv(uProjection, 1, GL_FALSE, &transforms[D3DTS_PROJECTION].m[0][0]);
    }
    glUniform1i(uTextured, textures[0] ? 1 : 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "u_texture"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0] ? static_cast<PW153Texture *>(textures[0])->texture() : 0);
    if (textures[0]) {
      // Stage 0's addressing and filtering apply to the bound texture.
      const TextureStage &stage = textureStages[0];
      static const GLenum wrapModes[] = {0, GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE};
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                      stage.addressU < 4 ? wrapModes[stage.addressU] : GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                      stage.addressV < 4 ? wrapModes[stage.addressV] : GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                      stage.magFilter == D3DTEXF_LINEAR ? GL_LINEAR : GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      stage.minFilter == D3DTEXF_LINEAR ? GL_LINEAR : GL_NEAREST);
    }
    return true;
  }
  //	Vertex/index counts for a D3D8 primitive count (triangle list = 3
  //	per primitive, strip/fan = count + 2, line list = 2 per primitive).
  static uint64_t CountsForType(DWORD type, UINT count) {
    return type == D3DPT_POINTLIST ? count :
           type == D3DPT_LINELIST ? uint64_t(count) * 2 :
           type == D3DPT_LINESTRIP ? uint64_t(count) + 1 :
           type == D3DPT_TRIANGLELIST ? uint64_t(count) * 3 : uint64_t(count) + 2;
  }
  HRESULT DrawIndexedPrimitive(DWORD type, UINT minIndex, UINT numVertices, UINT startIndex, UINT count) override {
    if (emscripten_webgl_get_current_context() != context || !indexBuffer || !streamSource) return E_FAIL;
    static const GLenum modes[] = {0, GL_POINTS, GL_LINES, GL_LINE_STRIP,
                                   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    if (type < 1 || type > 6) return E_FAIL;
    uint64_t indices = CountsForType(type, count);
    auto *indexData = static_cast<PW153IndexBuffer *>(indexBuffer);
    auto *buffer = static_cast<PW153VertexBuffer *>(streamSource);
    const UINT indexSize = indexData->format() == D3DFMT_INDEX32 ? 4 : 2;
    if (!count || indices > INT32_MAX ||
        uint64_t(startIndex) * indexSize + indices * indexSize > UINT32_MAX) return E_FAIL;
    if (!PrepareFvfDraw()) return E_FAIL;
    //	The D3D8 SetIndices second parameter is BaseVertexIndex: every
    //	index refers to baseVertexIndex + i. WebGL cannot offset index
    //	values, so the vertex base shifts the attribute pointers instead.
    uintptr_t baseOffset = 0;
    if (SetupFvfAttributes(currentFVF, buffer->buffer(), streamOffset, indexOffset, baseOffset) != D3D_OK) return E_FAIL;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexData->buffer());
    glDrawElements(modes[type], static_cast<GLsizei>(indices),
                   indexData->format() == D3DFMT_INDEX32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
                   reinterpret_cast<const void *>(uint64_t(startIndex) * indexSize));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT DrawPrimitiveUP(DWORD type, UINT count, const void *data, UINT stride) override {
    if (emscripten_webgl_get_current_context() != context || !data || !stride) return E_FAIL;
    static const GLenum modes[] = {0, GL_POINTS, GL_LINES, GL_LINE_STRIP,
                                   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    if (type < 1 || type > 6) return E_FAIL;
    const uint64_t vertices = CountsForType(type, count);
    if (!count || vertices > INT32_MAX || vertices * stride > UINT32_MAX) return E_FAIL;
    if (!PrepareFvfDraw()) return E_FAIL;
    // Upload the user-pointer data to a temporary VBO and draw with the
    // current FVF's attribute layout.
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices * stride), data, GL_STREAM_DRAW);
    uintptr_t baseOffset = 0;
    const HRESULT setup = SetupFvfAttributes(currentFVF, vbo, stride, 0, baseOffset);
    if (setup == D3D_OK)
      glDrawArrays(modes[type], 0, static_cast<GLsizei>(vertices));
    glDeleteBuffers(1, &vbo);
    return setup == D3D_OK && glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT DrawIndexedPrimitiveUP(DWORD type, UINT minIndex, UINT numVertices, UINT count,
                                 const void *indices, DWORD indexFormat,
                                 const void *vertexData, UINT stride) override {
    if (emscripten_webgl_get_current_context() != context || !indices || !vertexData || !stride) return E_FAIL;
    static const GLenum modes[] = {0, GL_POINTS, GL_LINES, GL_LINE_STRIP,
                                   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    if (type < 1 || type > 6 || (indexFormat != D3DFMT_INDEX16 && indexFormat != D3DFMT_INDEX32)) return E_FAIL;
    const uint64_t indicesCount = CountsForType(type, count);
    if (!count || !numVertices || indicesCount > INT32_MAX ||
        uint64_t(numVertices) * stride > UINT32_MAX ||
        indicesCount * (indexFormat == D3DFMT_INDEX16 ? 2 : 4) > UINT32_MAX) return E_FAIL;
    if (!PrepareFvfDraw()) return E_FAIL;
    GLuint vbo = 0, ibo = 0;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(uint64_t(numVertices) * stride), vertexData, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indicesCount * (indexFormat == D3DFMT_INDEX16 ? 2 : 4)), indices, GL_STREAM_DRAW);
    uintptr_t baseOffset = 0;
    const HRESULT setup = SetupFvfAttributes(currentFVF, vbo, stride, 0, baseOffset);
    if (setup == D3D_OK)
      glDrawElements(modes[type], static_cast<GLsizei>(indicesCount),
                     indexFormat == D3DFMT_INDEX16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, nullptr);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    return setup == D3D_OK && glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT SetRenderTarget(IDirect3DSurface8 *target, IDirect3DSurface8 *depth) override {
    if (emscripten_webgl_get_current_context() != context) return E_FAIL;
    // The default colour surface (or NULL, the D3D8 "back buffer" request)
    // binds the canvas's framebuffer 0 — no renderbuffer attachments may
    // be made to the default framebuffer.
    if (!target || target == defaultColor) {
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      return D3D_OK;
    }
    auto *surface = static_cast<PW153Surface *>(target);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, surface->renderbuffer());
    if (depth && depth != defaultDepth) {
      auto *depthSurface = static_cast<PW153Surface *>(depth);
      glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_RENDERBUFFER, depthSurface->renderbuffer());
    }
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
};
}
IDirect3DDevice8 *pw153_create_web_device(const D3DPRESENT_PARAMETERS &params) {
  const auto context = emscripten_webgl_get_current_context();
  if (!context || emscripten_is_webgl_context_lost(context) || !params.Windowed ||
      !params.BackBufferWidth || !params.BackBufferHeight || params.BackBufferCount > 1 || params.MultiSampleType != D3DMULTISAMPLE_NONE ||
      (params.BackBufferFormat != D3DFMT_X8R8G8B8 && params.BackBufferFormat != D3DFMT_A8R8G8B8 && params.BackBufferFormat != D3DFMT_R5G6B5)) return nullptr;
  // Render DIRECTLY to the canvas's default framebuffer (framebuffer 0).
  // The offscreen framebuffer + blit approach doesn't produce visible
  // output (the blit fails on multisampled framebuffers even with
  // antialias=false). The canvas's default framebuffer has a depth buffer
  // (the context was created with depth=true), so depth testing works
  // without explicit renderbuffers. Present is a no-op — the rendering
  // is already on the visible canvas.
  constexpr GLuint fb = 0;
  auto *result = new (std::nothrow) Device(nullptr, nullptr, fb, context,
                                           params.BackBufferWidth, params.BackBufferHeight);
  return result;
}
