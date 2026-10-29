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
public:
  Device(PW153Surface *c, PW153Surface *d, GLuint f, EMSCRIPTEN_WEBGL_CONTEXT_HANDLE owner)
    : color(c), depth(d), framebuffer(f), context(owner) {}
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
  HRESULT SetTexture(DWORD, IDirect3DBaseTexture8 *) override { return E_NOTIMPL; }
  HRESULT GetClipPlane(DWORD, float *) override { return E_NOTIMPL; }
  HRESULT SetClipPlane(DWORD, const float *) override { return E_NOTIMPL; }
  HRESULT CreateRenderTarget(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, BOOL, IDirect3DSurface8 **surface) override { if (surface) *surface = nullptr; return E_NOTIMPL; }
  HRESULT CreateDepthStencilSurface(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, IDirect3DSurface8 **surface) override { if (surface) *surface = nullptr; return E_NOTIMPL; }
  HRESULT CreateTexture(UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool, IDirect3DTexture8 **out) override {
    if (!out) return E_FAIL;
    *out = nullptr;
    if (emscripten_webgl_get_current_context() != context || emscripten_is_webgl_context_lost(context)) return E_FAIL;
    *out = pw153_create_texture(this, width, height, levels, usage, format, pool);
    return *out ? D3D_OK : E_FAIL;
  }
  HRESULT CreateCubeTexture(UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DCubeTexture8 **cubeTexture) override { if (cubeTexture) *cubeTexture = nullptr; return E_NOTIMPL; }
  HRESULT CreateImageSurface(UINT, UINT, D3DFORMAT, IDirect3DSurface8 **surface) override { if (surface) *surface = nullptr; return E_NOTIMPL; }
  HRESULT TestCooperativeLevel() override { return E_NOTIMPL; }
  HRESULT BeginScene() override { return E_NOTIMPL; }
  HRESULT EndScene() override { return E_NOTIMPL; }
  HRESULT Present(const RECT *, const RECT *, HWND, const void *) override { return E_NOTIMPL; }
  HRESULT Reset(D3DPRESENT_PARAMETERS *) override { return E_NOTIMPL; }
  HRESULT SetTransform(D3DTRANSFORMSTATETYPE, const D3DMATRIX *) override { return E_NOTIMPL; }
  HRESULT SetRenderState(DWORD, DWORD) override { return E_NOTIMPL; }
  HRESULT SetTextureStageState(DWORD, DWORD, DWORD) override { return E_NOTIMPL; }
  HRESULT GetTextureStageState(DWORD, DWORD, DWORD *) override { return E_NOTIMPL; }
  HRESULT SetLight(DWORD, const D3DLIGHT8 *) override { return E_NOTIMPL; }
  HRESULT LightEnable(DWORD, BOOL) override { return E_NOTIMPL; }
  HRESULT SetMaterial(const D3DMATERIAL8 *) override { return E_NOTIMPL; }
  HRESULT SetViewport(const D3DVIEWPORT8 *) override { return E_NOTIMPL; }
  HRESULT SetStreamSource(UINT, IDirect3DVertexBuffer8 *, UINT) override { return E_NOTIMPL; }
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
  HRESULT SetVertexShader(DWORD) override { return E_NOTIMPL; }
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
  HRESULT DrawIndexedPrimitive(DWORD, UINT, UINT, UINT, UINT) override { return E_NOTIMPL; }
  HRESULT DrawPrimitive(DWORD, UINT, UINT) override { return E_NOTIMPL; }
  HRESULT DrawIndexedPrimitiveUP(DWORD, UINT, UINT, UINT, const void *, DWORD, const void *, UINT) override { return E_NOTIMPL; }
  HRESULT DrawPrimitiveUP(DWORD, UINT, const void *, UINT) override { return E_NOTIMPL; }
  HRESULT SetRenderTarget(IDirect3DSurface8 *, IDirect3DSurface8 *) override { return E_NOTIMPL; }
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
  auto *result = new (std::nothrow) Device(color, depth, fb, context);
  if (!result) { glDeleteFramebuffers(1, &fb); color->Release(); if (depth) depth->Release(); }
  return result;
}
