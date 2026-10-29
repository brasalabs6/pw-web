#include "d3d8.h"
#include <GLES3/gl3.h>
#include <emscripten/html5.h>
#include <new>

namespace {
bool usable_context() {
  const auto context = emscripten_webgl_get_current_context();
  if (context <= 0 || emscripten_is_webgl_context_lost(context)) return false;
  EmscriptenWebGLContextAttributes attributes{};
  return emscripten_webgl_get_context_attributes(context, &attributes) == EMSCRIPTEN_RESULT_SUCCESS &&
         attributes.majorVersion >= 2;
}
// Check actual framebuffer completeness without changing the caller's bindings.
bool renderable_color(DWORD format, DWORD depth = D3DFMT_UNKNOWN) {
  GLenum storage = 0;
  switch (format) {
    case D3DFMT_R5G6B5: storage = GL_RGB565; break;
    case D3DFMT_X8R8G8B8:
    case D3DFMT_A8R8G8B8: storage = GL_RGBA8; break;
    default: return false;
  }
  GLenum depthStorage = 0;
  switch (depth) {
    case D3DFMT_UNKNOWN: break;
    case D3DFMT_D16: depthStorage = GL_DEPTH_COMPONENT16; break;
    case D3DFMT_D24X8: depthStorage = GL_DEPTH_COMPONENT24; break;
    default: return false;
  }
  GLint draw = 0, read = 0, previous = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
  glGetIntegerv(GL_RENDERBUFFER_BINDING, &previous);
  GLuint framebuffer = 0, renderbuffer = 0, depthbuffer = 0;
  glGenFramebuffers(1, &framebuffer);
  glGenRenderbuffers(1, &renderbuffer);
  bool supported = false;
  if (framebuffer && renderbuffer) {
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, storage, 1, 1);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
    if (depthStorage) {
      glGenRenderbuffers(1, &depthbuffer);
      if (depthbuffer) {
        glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, depthStorage, 1, 1);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer);
      }
    }
    supported = (!depthStorage || depthbuffer) && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
  }
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(draw));
  glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(read));
  glBindRenderbuffer(GL_RENDERBUFFER, static_cast<GLuint>(previous));
  glDeleteFramebuffers(1, &framebuffer);
  glDeleteRenderbuffers(1, &renderbuffer);
  glDeleteRenderbuffers(1, &depthbuffer);
  return supported;
}
class Factory final : public IDirect3D8 {
  ULONG references = 1;
public:
  HRESULT QueryInterface(const void *, void **out) override {
    if (out) *out = nullptr;
    return E_NOINTERFACE;
  }
  ULONG AddRef() override { return ++references; }
  ULONG Release() override { auto count = --references; if (!count) delete this; return count; }
  HRESULT CheckDeviceType(UINT adapter, DWORD type, DWORD adapterFormat, DWORD target, BOOL) override {
    if (adapter != 0 || type != D3DDEVTYPE_HAL || !usable_context()) return E_FAIL;
    if (adapterFormat != target && !(adapterFormat == D3DFMT_X8R8G8B8 && target == D3DFMT_A8R8G8B8)) return E_FAIL;
    return renderable_color(target) ? D3D_OK : E_FAIL;
  }
  HRESULT CheckDeviceFormat(UINT adapter, DWORD type, DWORD adapterFormat, DWORD usage, DWORD resource, DWORD format) override {
    if (adapter != 0 || type != D3DDEVTYPE_HAL || adapterFormat != D3DFMT_X8R8G8B8 ||
        usage != D3DUSAGE_DEPTHSTENCIL || resource != D3DRTYPE_SURFACE ||
        format == D3DFMT_UNKNOWN || !usable_context()) return E_FAIL;
    return renderable_color(adapterFormat, format) ? D3D_OK : E_FAIL;
  }
  HRESULT CreateDevice(UINT, DWORD, HWND, DWORD, D3DPRESENT_PARAMETERS *, IDirect3DDevice8 **out) override {
    if (out) *out = nullptr;
    return E_NOTIMPL;
  }
  HRESULT GetAdapterIdentifier(UINT, DWORD, D3DADAPTER_IDENTIFIER8 *) override { return E_NOTIMPL; }
  UINT GetAdapterModeCount(UINT adapter) override { return adapter == 0 && usable_context() ? 1 : 0; }
  HRESULT EnumAdapterModes(UINT adapter, UINT mode, D3DDISPLAYMODE *out) override {
    return mode == 0 ? GetAdapterDisplayMode(adapter, out) : E_FAIL;
  }
  HRESULT GetAdapterDisplayMode(UINT adapter, D3DDISPLAYMODE *out) override {
    if (adapter != 0 || !out || !usable_context()) return E_FAIL;
    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (viewport[2] <= 0 || viewport[3] <= 0 || glGetError() != GL_NO_ERROR) return E_FAIL;
    *out = {static_cast<UINT>(viewport[2]), static_cast<UINT>(viewport[3]), 0, D3DFMT_X8R8G8B8};
    return D3D_OK;
  }
  HRESULT GetDeviceCaps(UINT adapter, DWORD type, D3DCAPS8 *caps) override {
    if (adapter != 0 || type != D3DDEVTYPE_HAL || !caps || !usable_context()) return E_FAIL;
    // Report only implemented adapter capabilities, not the raw WebGL limits.
    // Texture/shader support is deliberately zero until the device implements it.
    *caps = {};
    caps->DeviceType = type;
    caps->AdapterOrdinal = adapter;
    return D3D_OK;
  }
  HRESULT CheckDepthStencilMatch(UINT adapter, DWORD type, DWORD adapterFormat, DWORD target, DWORD depth) override {
    if (adapter != 0 || type != D3DDEVTYPE_HAL || !usable_context() ||
        adapterFormat != D3DFMT_X8R8G8B8 || depth == D3DFMT_UNKNOWN) return E_FAIL;
    return renderable_color(target, depth) ? D3D_OK : E_FAIL;
  }
};
}
IDirect3D8 *Direct3DCreate8(UINT version) {
  return version == D3D_SDK_VERSION && usable_context() ? new (std::nothrow) Factory : nullptr;
}
