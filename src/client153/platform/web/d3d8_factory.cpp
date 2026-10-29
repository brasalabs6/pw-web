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
class Factory final : public IDirect3D8 {
  ULONG references = 1;
public:
  HRESULT QueryInterface(const void *, void **out) override {
    if (out) *out = nullptr;
    return E_NOINTERFACE;
  }
  ULONG AddRef() override { return ++references; }
  ULONG Release() override { auto count = --references; if (!count) delete this; return count; }
  HRESULT CheckDeviceType(UINT, DWORD, DWORD, DWORD, BOOL) override { return E_NOTIMPL; }
  HRESULT CheckDeviceFormat(UINT, DWORD, DWORD, DWORD, DWORD, DWORD) override { return E_NOTIMPL; }
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
  HRESULT GetDeviceCaps(UINT, DWORD, D3DCAPS8 *) override { return E_NOTIMPL; }
  HRESULT CheckDepthStencilMatch(UINT, DWORD, DWORD, DWORD, DWORD) override { return E_NOTIMPL; }
};
}
IDirect3D8 *Direct3DCreate8(UINT version) {
  return version == D3D_SDK_VERSION && usable_context() ? new (std::nothrow) Factory : nullptr;
}
