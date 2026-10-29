#include "d3d8.h"
#include "d3d8_surface.h"
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
  auto *surface = pw153_create_surface(4, 3, D3DFMT_R5G6B5);
  if (!surface) { EM_ASM({ document.body.dataset.result = 'missing-surface'; }); return 20; }
  D3DSURFACE_DESC description{};
  if (surface->GetDesc(&description) != D3D_OK || description.Width != 4 || description.Height != 3 || description.Format != D3DFMT_R5G6B5) return 21;
  const GLuint buffer = surface->renderbuffer();
  if (!glIsRenderbuffer(buffer)) return 22;
  if (surface->AddRef() != 2 || surface->Release() != 1 || !glIsRenderbuffer(buffer)) return 25;
  if (surface->Release() != 0) return 26;
  if (glIsRenderbuffer(buffer)) return 23;
  if (pw153_create_surface(0, 3, D3DFMT_R5G6B5) || pw153_create_surface(4, 3, D3DFMT_DXT1)) return 24;
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
  params.BackBufferWidth = 32; params.BackBufferHeight = 16;
  params.BackBufferFormat = D3DFMT_X8R8G8B8; params.Windowed = TRUE;
  params.EnableAutoDepthStencil = TRUE; params.AutoDepthStencilFormat = D3DFMT_D24X8;
  auto invalid = params; invalid.Windowed = FALSE;
  if (d3d->CreateDevice(0, D3DDEVTYPE_HAL, nullptr, 0, &invalid, &device) == D3D_OK || device) return 33;
  if (d3d->CreateDevice(0, D3DDEVTYPE_HAL, nullptr, 0, &params, &device) != D3D_OK || !device) {
    EM_ASM({ document.body.dataset.result = 'missing-device'; }); return 7;
  }
  IDirect3DSurface8 *target = nullptr, *depth = nullptr, *back = nullptr;
  if (device->GetRenderTarget(&target) != D3D_OK || device->GetDepthStencilSurface(&depth) != D3D_OK ||
      device->GetBackBuffer(0, 0, &back) != D3D_OK || !target || !depth || !back) return 27;
  if (target != back) return 28;
  if (device->Clear(0, nullptr, D3DCLEAR_TARGET, 0xffff0000, 1.0f, 0) != D3D_OK) return 29;
  IDirect3DTexture8 *texture = reinterpret_cast<IDirect3DTexture8 *>(1);
  if (device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture) == D3D_OK || texture) return 31;
  if (device->Clear(0, nullptr, 0x4, 0, 1.0f, 0) == D3D_OK) return 32;
  GLuint frame;
  glGenFramebuffers(1, &frame); glBindFramebuffer(GL_FRAMEBUFFER, frame);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                            static_cast<PW153Surface *>(target)->renderbuffer());
  unsigned char pixel[4]{}; glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
  if (pixel[0] < 250 || pixel[1] > 2 || pixel[2] > 2) return 30;
  glBindFramebuffer(GL_FRAMEBUFFER, 0); glDeleteFramebuffers(1, &frame);
  auto colorBuffer = static_cast<PW153Surface *>(target)->renderbuffer();
  auto depthBuffer = static_cast<PW153Surface *>(depth)->renderbuffer();
  target->Release(); depth->Release(); back->Release(); device->Release();
  if (glIsRenderbuffer(colorBuffer) || glIsRenderbuffer(depthBuffer)) return 34;
  d3d->Release();
  emscripten_webgl_destroy_context(context);
  EM_ASM({ document.body.dataset.result = 'pass'; });
}
