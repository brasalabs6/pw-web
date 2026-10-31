#include "d3d8.h"
#include "d3d8_surface.h"
#include <GLES3/gl3.h>
#include <emscripten/html5.h>
#include <new>

IDirect3DDevice8 *pw153_create_web_device(const D3DPRESENT_PARAMETERS &params);

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
  auto *color = pw153_create_surface(1, 1, static_cast<D3DFORMAT>(format));
  if (!color || format == D3DFMT_D16 || format == D3DFMT_D24X8) {
    if (color) color->Release();
    return false;
  }
  PW153Surface *depthSurface = nullptr;
  if (depth != D3DFMT_UNKNOWN) {
    if (depth != D3DFMT_D16 && depth != D3DFMT_D24X8) { color->Release(); return false; }
    depthSurface = pw153_create_surface(1, 1, static_cast<D3DFORMAT>(depth));
    if (!depthSurface) { color->Release(); return false; }
  }
  GLint draw = 0, read = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read);
  GLuint framebuffer = 0;
  glGenFramebuffers(1, &framebuffer);
  bool supported = false;
  if (framebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color->renderbuffer());
    if (depthSurface) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthSurface->renderbuffer());
    supported = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
  }
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(draw));
  glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(read));
  glDeleteFramebuffers(1, &framebuffer);
  if (depthSurface) depthSurface->Release();
  color->Release();
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
    if (adapter != 0 || type != D3DDEVTYPE_HAL || adapterFormat != D3DFMT_X8R8G8B8 || !usable_context()) return E_FAIL;
    if (usage == 0 && resource == D3DRTYPE_TEXTURE &&
        (format == D3DFMT_A4R4G4B4 || format == D3DFMT_A8R8G8B8)) return D3D_OK;
    if (usage == D3DUSAGE_DEPTHSTENCIL && resource == D3DRTYPE_SURFACE && format != D3DFMT_UNKNOWN)
      return renderable_color(adapterFormat, format) ? D3D_OK : E_FAIL;
    return E_FAIL;
  }
  HRESULT CreateDevice(UINT adapter, DWORD type, HWND, DWORD, D3DPRESENT_PARAMETERS *params, IDirect3DDevice8 **out) override {
    if (!out) return E_FAIL;
    *out = nullptr;
    if (adapter != 0 || type != D3DDEVTYPE_HAL || !params || !usable_context()) return E_FAIL;
    *out = pw153_create_web_device(*params);
    return *out ? D3D_OK : E_FAIL;
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
