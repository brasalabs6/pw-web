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
  D3DMATRIX transforms[32] = {};
  // The FVF shader program (created lazily on the first draw call).
  GLuint shaderProgram = 0;
  GLint aPosition = -1, aColor = -1, aTexCoord = -1, uViewport = -1, uTextured = -1;
public:
  Device(PW153Surface *c, PW153Surface *d, GLuint f, EMSCRIPTEN_WEBGL_CONTEXT_HANDLE owner,
         UINT w, UINT h)
    : color(c), depth(d), framebuffer(f), context(owner), backWidth(w), backHeight(h) {}
  ~Device() override { }
  HRESULT QueryInterface(const void *, void **out) override { if (out) *out = nullptr; return E_NOINTERFACE; }
  ULONG AddRef() override { return ++refs; }
  ULONG Release() override { auto count = --refs; if (!count) delete this; return count; }
  HRESULT GetRenderTarget(IDirect3DSurface8 **out) override { if (!out) return E_FAIL; *out = nullptr; return D3D_OK; }
  HRESULT GetDepthStencilSurface(IDirect3DSurface8 **out) override { if (!out) return E_FAIL; *out = nullptr; return D3D_OK; }
  HRESULT GetBackBuffer(UINT index, DWORD type, IDirect3DSurface8 **out) override {
    if (index || type) { if (out) *out = nullptr; return E_FAIL; } return GetRenderTarget(out);
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
    if (state >= 0 && state < 32) {
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
    } else return E_NOTIMPL;
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT SetTextureStageState(DWORD, DWORD, DWORD) override { return E_NOTIMPL; }
  HRESULT GetTextureStageState(DWORD, DWORD, DWORD *) override { return E_NOTIMPL; }
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
  HRESULT SetIndices(IDirect3DIndexBuffer8 *buffer, UINT offset) override {
    // Store the index buffer for the next indexed draw (the D3D8 contract:
    // SetIndices persists until the next SetIndices, and the base offset
    // shifts every index read).
    if (buffer) buffer->AddRef();
    if (indexBuffer) indexBuffer->Release();
    indexBuffer = buffer;
    indexOffset = offset;
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
  // The sprite FVF contains pretransformed screen coordinates and one texture.
  void ensureShaderProgram() {
    if (shaderProgram) return;
    static const char *vsSource =
        "#version 300 es\n"
        "layout(location=0) in vec4 a_position;\n"
        "layout(location=1) in vec4 a_color;\n"
        "layout(location=2) in vec2 a_texcoord;\n"
        "uniform vec4 u_viewport;\n"
        "out vec4 v_color;\n"
        "out vec2 v_texcoord;\n"
        "void main() { float w=1.0/a_position.w; vec2 p=(a_position.xy-u_viewport.xy)/u_viewport.zw; "
        "gl_Position=vec4((p.x*2.0-1.0)*w,(1.0-p.y*2.0)*w,(a_position.z*2.0-1.0)*w,w); "
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
  }

  HRESULT DrawPrimitive(DWORD type, UINT startVertex, UINT count) override {
    if (emscripten_webgl_get_current_context() != context || !streamSource ||
        currentFVF != (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1) ||
        streamOffset < 32) return E_FAIL;
    static const GLenum modes[] = {0, GL_POINTS, GL_LINES, GL_LINE_STRIP,
                                   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    if (type < 1 || type > 6) return E_FAIL;
    uint64_t vertices = type == D3DPT_POINTLIST ? count :
                        type == D3DPT_LINELIST ? uint64_t(count) * 2 :
                        type == D3DPT_LINESTRIP ? uint64_t(count) + 1 :
                        type == D3DPT_TRIANGLELIST ? uint64_t(count) * 3 : uint64_t(count) + 2;
    auto *buffer = static_cast<PW153VertexBuffer *>(streamSource);
    if (!count || vertices > INT32_MAX || (uint64_t(startVertex) + vertices) * streamOffset > buffer->size()) return E_FAIL;
    ensureShaderProgram();
    if (!shaderProgram) return E_FAIL;
    GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0 || viewport[3] <= 0) return E_FAIL;
    glUseProgram(shaderProgram);
    glUniform4f(uViewport, viewport[0], viewport[1], viewport[2], viewport[3]);
    glUniform1i(uTextured, textures[0] ? 1 : 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "u_texture"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0] ? static_cast<PW153Texture *>(textures[0])->texture() : 0);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->buffer());
    glEnableVertexAttribArray(aPosition); glEnableVertexAttribArray(aColor); glEnableVertexAttribArray(aTexCoord);
    glVertexAttribPointer(aPosition, 4, GL_FLOAT, GL_FALSE, streamOffset, reinterpret_cast<const void *>(0));
    glVertexAttribPointer(aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, streamOffset, reinterpret_cast<const void *>(16));
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, streamOffset, reinterpret_cast<const void *>(24));
    glDrawArrays(modes[type], startVertex, static_cast<GLsizei>(vertices));
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT DrawIndexedPrimitive(DWORD type, UINT minIndex, UINT numVertices, UINT startIndex, UINT count) override {
    if (emscripten_webgl_get_current_context() != context || !indexBuffer || !streamSource) return E_FAIL;
    static const GLenum modes[] = {0, GL_POINTS, GL_LINES, GL_LINE_STRIP,
                                   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    if (type < 1 || type > 6) return E_FAIL;
    // D3D8 count is a primitive count: triangle list = 3 indices per
    // primitive, strip/fan = count + 2, line list = 2 per primitive.
    uint64_t indices = type == D3DPT_POINTLIST ? count :
                       type == D3DPT_LINELIST ? uint64_t(count) * 2 :
                       type == D3DPT_LINESTRIP ? uint64_t(count) + 1 :
                       type == D3DPT_TRIANGLELIST ? uint64_t(count) * 3 : uint64_t(count) + 2;
    auto *indexData = static_cast<PW153IndexBuffer *>(indexBuffer);
    auto *buffer = static_cast<PW153VertexBuffer *>(streamSource);
    const UINT indexSize = indexData->format() == D3DFMT_INDEX32 ? 4 : 2;
    if (!count || indices > INT32_MAX ||
        uint64_t(indexOffset) + uint64_t(startIndex) * indexSize + indices * indexSize > UINT32_MAX) return E_FAIL;
    ensureShaderProgram();
    if (!shaderProgram) return E_FAIL;
    GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0 || viewport[3] <= 0) return E_FAIL;
    glUseProgram(shaderProgram);
    glUniform4f(uViewport, viewport[0], viewport[1], viewport[2], viewport[3]);
    glUniform1i(uTextured, textures[0] ? 1 : 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "u_texture"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0] ? static_cast<PW153Texture *>(textures[0])->texture() : 0);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->buffer());
    glEnableVertexAttribArray(aPosition); glEnableVertexAttribArray(aColor); glEnableVertexAttribArray(aTexCoord);
    glVertexAttribPointer(aPosition, 4, GL_FLOAT, GL_FALSE, streamOffset, reinterpret_cast<const void *>(0));
    glVertexAttribPointer(aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, streamOffset, reinterpret_cast<const void *>(16));
    glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, streamOffset, reinterpret_cast<const void *>(24));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexData->buffer());
    glDrawElements(modes[type], static_cast<GLsizei>(indices),
                   indexData->format() == D3DFMT_INDEX32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT,
                   reinterpret_cast<const void *>(uint64_t(indexOffset) + uint64_t(startIndex) * indexSize));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT DrawPrimitiveUP(DWORD type, UINT count, const void *data, UINT stride) override {
    if (emscripten_webgl_get_current_context() != context || !data) return E_FAIL;
    static const GLenum modes[] = {0, GL_POINTS, GL_LINES, GL_LINE_STRIP,
                                   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    if (type < 1 || type > 6) return E_FAIL;
    // Upload the user-pointer data to a temporary VBO and draw.
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, count * stride, data, GL_STREAM_DRAW);
    glDrawArrays(modes[type], 0, count);
    glDeleteBuffers(1, &vbo);
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT DrawIndexedPrimitiveUP(DWORD type, UINT minIndex, UINT numVertices, UINT count,
                                 const void *indices, DWORD indexFormat,
                                 const void *vertexData, UINT stride) override {
    if (emscripten_webgl_get_current_context() != context || !indices || !vertexData) return E_FAIL;
    static const GLenum modes[] = {0, GL_POINTS, GL_LINES, GL_LINE_STRIP,
                                   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    if (type < 1 || type > 6) return E_FAIL;
    GLuint vbo = 0, ibo = 0;
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, numVertices * stride, vertexData, GL_STREAM_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * 3 * (indexFormat == D3DFMT_INDEX16 ? 2 : 4), indices, GL_STREAM_DRAW);
    glDrawElements(modes[type], count * 3, indexFormat == D3DFMT_INDEX16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, nullptr);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT SetRenderTarget(IDirect3DSurface8 *target, IDirect3DSurface8 *depth) override {
    if (emscripten_webgl_get_current_context() != context) return E_FAIL;
    if (!target) {
      // Binding the default framebuffer (the canvas).
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      return D3D_OK;
    }
    auto *surface = static_cast<PW153Surface *>(target);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, surface->renderbuffer());
    if (depth) {
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
