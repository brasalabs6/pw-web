#include "d3d8.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

int main() {
  if (Direct3DCreate8(D3D_SDK_VERSION)) return 1;
  EmscriptenWebGLContextAttributes attributes;
  emscripten_webgl_init_context_attributes(&attributes);
  attributes.majorVersion = 2;
  auto context = emscripten_webgl_create_context("#canvas", &attributes);
  if (context <= 0 || emscripten_webgl_make_context_current(context)) return 2;
  if (Direct3DCreate8(D3D_SDK_VERSION + 1)) return 3;
  IDirect3D8 *d3d = Direct3DCreate8(D3D_SDK_VERSION);
  if (!d3d) { EM_ASM({ document.body.dataset.result = 'missing-factory'; }); return 4; }
  GLuint framebuffer, renderbuffer;
  glGenFramebuffers(1, &framebuffer); glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glGenRenderbuffers(1, &renderbuffer); glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
  if (d3d->CheckDeviceType(0, D3DDEVTYPE_HAL, D3DFMT_R5G6B5, D3DFMT_R5G6B5, FALSE) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'missing-rgb565'; }); return 8;
  }
  GLint boundFramebuffer, boundRenderbuffer;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &boundFramebuffer);
  glGetIntegerv(GL_RENDERBUFFER_BINDING, &boundRenderbuffer);
  if (boundFramebuffer != static_cast<GLint>(framebuffer) || boundRenderbuffer != static_cast<GLint>(renderbuffer)) return 11;
  glBindFramebuffer(GL_FRAMEBUFFER, 0); glDeleteFramebuffers(1, &framebuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, 0); glDeleteRenderbuffers(1, &renderbuffer);
  if (d3d->CheckDeviceType(1, D3DDEVTYPE_HAL, D3DFMT_R5G6B5, D3DFMT_R5G6B5, FALSE) == D3D_OK) return 9;
  if (d3d->CheckDeviceType(0, D3DDEVTYPE_HAL, D3DFMT_R5G6B5, D3DFMT_DXT1, FALSE) == D3D_OK) return 10;
  if (d3d->CheckDepthStencilMatch(0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_D16) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'missing-depth-match'; }); return 16;
  }
  if (d3d->CheckDepthStencilMatch(1, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_D16) == D3D_OK) return 17;
  if (d3d->CheckDepthStencilMatch(0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_DXT1) == D3D_OK) return 18;
  if (d3d->CheckDeviceFormat(0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'missing-depth-format'; }); return 19;
  }
  D3DCAPS8 caps{};
  if (d3d->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps) != D3D_OK || caps.DeviceType != D3DDEVTYPE_HAL || caps.MaxTextureWidth != 0 || caps.MaxSimultaneousTextures != 0) {
    EM_ASM({ document.body.dataset.result = 'missing-caps'; }); return 12;
  }
  if (d3d->GetDeviceCaps(1, D3DDEVTYPE_HAL, &caps) == D3D_OK) return 13;
  D3DDISPLAYMODE mode{};
  if (d3d->GetAdapterDisplayMode(0, &mode) != D3D_OK || !mode.Width || !mode.Height) return 5;
  if (d3d->GetAdapterDisplayMode(1, &mode) == D3D_OK) return 6;
  IDirect3DDevice8 *device = reinterpret_cast<IDirect3DDevice8 *>(1);
  D3DPRESENT_PARAMETERS params{};
  if (d3d->CreateDevice(0, D3DDEVTYPE_HAL, nullptr, 0, &params, &device) == D3D_OK || device) return 7;
  d3d->Release();
  emscripten_webgl_destroy_context(context);
  EM_ASM({ document.body.dataset.result = 'pass'; });
}
