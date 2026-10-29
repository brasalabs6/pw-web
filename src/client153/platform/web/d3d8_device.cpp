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
  DWORD currentFVF = 0;
  IDirect3DBaseTexture8 *textures[8] = {};
  D3DMATRIX transforms[32] = {};
  // The FVF shader program (created lazily on the first draw call).
  GLuint shaderProgram = 0;
  GLint aPosition = -1, aColor = -1;
public:
  Device(PW153Surface *c, PW153Surface *d, GLuint f, EMSCRIPTEN_WEBGL_CONTEXT_HANDLE owner,
         UINT w, UINT h)
    : color(c), depth(d), framebuffer(f), context(owner), backWidth(w), backHeight(h) {}
  ~Device() override { glDeleteFramebuffers(1, &framebuffer); color->Release(); if (depth) depth->Release(); }
  HRESULT QueryInterface(const void *, void **out) override { if (out) *out = nullptr; return E_NOINTERFACE; }
  ULONG AddRef() override { return ++refs; }
  ULONG Release() override { auto count = --refs; if (!count) delete this; return count; }
  HRESULT GetRenderTarget(IDirect3DSurface8 **out) override { if (!out) return E_FAIL; color->AddRef(); *out = color; return D3D_OK; }
  HRESULT GetDepthStencilSurface(IDirect3DSurface8 **out) override { if (!out) return E_FAIL; *out = nullptr; if (!depth) return E_FAIL; depth->AddRef(); *out = depth; return D3D_OK; }
  HRESULT GetBackBuffer(UINT index, DWORD type, IDirect3DSurface8 **out) override {
    if (index || type) { if (out) *out = nullptr; return E_FAIL; } return GetRenderTarget(out);
  }
  HRESULT Clear(DWORD count, const D3DRECT *rects, DWORD flags, DWORD value, float z, DWORD) override {
    if (count || rects || !flags || flags & ~(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER) ||
        ((flags & D3DCLEAR_ZBUFFER) && !depth) || emscripten_webgl_get_current_context() != context ||
        emscripten_is_webgl_context_lost(context)) return E_FAIL;
    GLint previous = 0; GLfloat oldColor[4], oldDepth; GLboolean oldMask[4], oldDepthMask;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previous); glGetFloatv(GL_COLOR_CLEAR_VALUE, oldColor);
    glGetFloatv(GL_DEPTH_CLEAR_VALUE, &oldDepth); glGetBooleanv(GL_COLOR_WRITEMASK, oldMask);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &oldDepthMask);
    const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer); glDisable(GL_SCISSOR_TEST);
    GLbitfield mask = 0;
    if (flags & D3DCLEAR_TARGET) { glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glClearColor(((value >> 16) & 255) / 255.0f, ((value >> 8) & 255) / 255.0f,
                   (value & 255) / 255.0f, ((value >> 24) & 255) / 255.0f); mask |= GL_COLOR_BUFFER_BIT; }
    if (flags & D3DCLEAR_ZBUFFER) { glDepthMask(GL_TRUE); glClearDepthf(z); mask |= GL_DEPTH_BUFFER_BIT; }
    glClear(mask);
    glClearColor(oldColor[0], oldColor[1], oldColor[2], oldColor[3]); glClearDepthf(oldDepth);
    glColorMask(oldMask[0], oldMask[1], oldMask[2], oldMask[3]); glDepthMask(oldDepthMask);
    if (scissor) glEnable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(previous));
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT CopyRects(IDirect3DSurface8 *, const RECT *, UINT, IDirect3DSurface8 *, const POINT *) override { return E_NOTIMPL; }
  HRESULT GetRenderState(DWORD, DWORD *) override { return E_NOTIMPL; }
  HRESULT SetTexture(DWORD stage, IDirect3DBaseTexture8 *texture) override {
    if (stage >= 8) return E_FAIL;
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
    *surface = pw153_create_surface(width, height, format);
    return *surface ? D3D_OK : E_FAIL;
  }
  HRESULT TestCooperativeLevel() override { return E_NOTIMPL; }
  HRESULT BeginScene() override { return D3D_OK; }
  HRESULT EndScene() override { return D3D_OK; }
  HRESULT Present(const RECT *, const RECT *, HWND, const void *) override {
    static int presentCount = 0;
    if (presentCount < 3) {
      printf("pw153_browser_probe: Present #%d, blitting %dx%d framebuffer to canvas\n",
             ++presentCount, backWidth, backHeight);
    }
    // Blit the offscreen framebuffer to the canvas (the default
    // framebuffer). This is the WebGL2 equivalent of the D3D8 swap chain
    // operation — the client renders to an offscreen back buffer and
    // Present copies it to the visible canvas.
    GLint previousRead = 0, previousDraw = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousRead);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousDraw);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, backWidth, backHeight,
                      0, 0, backWidth, backHeight,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    //\tDiagnostic: fill the canvas with dark blue so we can see the Present
    //\tis connected. This proves the pipeline works; the actual framebuffer
    //\tcontent depends on the client's Clear and Draw calls.
    glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(previousRead));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(previousDraw));
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
  HRESULT SetRenderState(DWORD, DWORD) override { return E_NOTIMPL; }
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
    if (stream != 0) return D3D_OK;
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
  HRESULT SetIndices(IDirect3DIndexBuffer8 *, UINT) override { return E_NOTIMPL; }
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
  HRESULT GetDeviceCaps(D3DCAPS8 *) override { return E_NOTIMPL; }
  // Create the FVF shader program lazily. The simplest FVF is
  // D3DFVF_XYZRHW | D3DFVF_DIFFUSE: position (4 floats) + color (4 bytes).
  // The vertex shader passes the position through (pre-transformed) and
  // the fragment shader outputs the vertex color.
  void ensureShaderProgram() {
    if (shaderProgram) return;
    static const char *vsSource =
        "#version 300 es\n"
        "in vec4 a_position;\n"
        "in vec4 a_color;\n"
        "out vec4 v_color;\n"
        "void main() { gl_Position = a_position; v_color = a_color; }\n";
    static const char *fsSource =
        "#version 300 es\n"
        "in vec4 v_color;\n"
        "out vec4 fragColor;\n"
        "void main() { fragColor = v_color; }\n";
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
  }

  HRESULT DrawPrimitive(DWORD type, UINT startVertex, UINT count) override {
    if (emscripten_webgl_get_current_context() != context) return E_FAIL;
    static const GLenum modes[] = {0, GL_POINTS, GL_LINES, GL_LINE_STRIP,
                                   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    if (type < 1 || type > 6) return E_FAIL;
    ensureShaderProgram();
    if (shaderProgram) glUseProgram(shaderProgram);
    glDrawArrays(modes[type], startVertex, count);
    return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
  }
  HRESULT DrawIndexedPrimitive(DWORD type, UINT minIndex, UINT numVertices, UINT startIndex, UINT count) override {
    if (emscripten_webgl_get_current_context() != context) return E_FAIL;
    static const GLenum modes[] = {0, GL_POINTS, GL_LINES, GL_LINE_STRIP,
                                   GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    if (type < 1 || type > 6) return E_FAIL;
    glDrawElements(modes[type], count * 3, GL_UNSIGNED_SHORT, nullptr);
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
  auto *color = pw153_create_surface(params.BackBufferWidth, params.BackBufferHeight, static_cast<D3DFORMAT>(params.BackBufferFormat));
  if (!color) return nullptr;
  PW153Surface *depth = nullptr;
  if (params.EnableAutoDepthStencil) {
    if (params.AutoDepthStencilFormat != D3DFMT_D16 && params.AutoDepthStencilFormat != D3DFMT_D24X8) { color->Release(); return nullptr; }
    depth = pw153_create_surface(params.BackBufferWidth, params.BackBufferHeight, static_cast<D3DFORMAT>(params.AutoDepthStencilFormat));
    if (!depth) { color->Release(); return nullptr; }
  }
  GLint draw = 0, read = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw); glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
  GLuint fb = 0; glGenFramebuffers(1, &fb);
  bool complete = false;
  if (fb) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color->renderbuffer());
    if (depth) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth->renderbuffer());
    complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
  }
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(draw)); glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(read));
  if (!complete) { glDeleteFramebuffers(1, &fb); color->Release(); if (depth) depth->Release(); return nullptr; }
  auto *result = new (std::nothrow) Device(color, depth, fb, context,
                                           params.BackBufferWidth, params.BackBufferHeight);
  if (!result) { glDeleteFramebuffers(1, &fb); color->Release(); if (depth) depth->Release(); }
  return result;
}
