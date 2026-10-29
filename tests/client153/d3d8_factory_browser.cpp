#include "d3d8.h"
#include "d3d8_surface.h"
#include "d3d8_buffer.h"
#include "d3d8_texture.h"
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
  IDirect3DVertexBuffer8 *vertex = nullptr;
  if (device->CreateVertexBuffer(16, D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &vertex) != D3D_OK || !vertex) {
    EM_ASM({ document.body.dataset.result = 'missing-vertex-buffer'; }); return 35;
  }
  BYTE *mapped = nullptr;
  if (vertex->Lock(4, 4, &mapped, 0) != D3D_OK || !mapped) return 36;
  mapped[0] = 7; mapped[1] = 8; mapped[2] = 9; mapped[3] = 10;
  if (vertex->Unlock() != D3D_OK || vertex->Unlock() == D3D_OK) return 37;
  if (vertex->Lock(15, 2, &mapped, 0) == D3D_OK || mapped) return 38;
  const GLuint vertexName = static_cast<PW153VertexBuffer *>(vertex)->buffer();
  glBindBuffer(GL_ARRAY_BUFFER, vertexName);
  if (!EM_ASM_INT({ const gpu = new Uint8Array(4); GLctx.getBufferSubData(GLctx.ARRAY_BUFFER, 4, gpu); return gpu[0] === 7 && gpu[1] === 8 && gpu[2] === 9 && gpu[3] === 10; })) return 39;
  glBindBuffer(GL_ARRAY_BUFFER, 0); vertex->Release();
  if (glIsBuffer(vertexName)) return 40;
  IDirect3DIndexBuffer8 *indices = nullptr;
  if (device->CreateIndexBuffer(8, D3DUSAGE_DYNAMIC, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &indices) != D3D_OK || !indices) {
    EM_ASM({ document.body.dataset.result = 'missing-index-buffer'; }); return 41;
  }
  if (indices->Lock(2, 2, &mapped, 0) != D3D_OK) return 42;
  mapped[0] = 0x34; mapped[1] = 0x12;
  if (indices->Unlock() != D3D_OK) return 43;
  const GLuint indexName = static_cast<PW153IndexBuffer *>(indices)->buffer();
  glBindBuffer(GL_ARRAY_BUFFER, indexName);
  if (!EM_ASM_INT({ const gpu = new Uint8Array(2); GLctx.getBufferSubData(GLctx.ARRAY_BUFFER, 2, gpu); return gpu[0] === 0x34 && gpu[1] === 0x12; })) return 44;
  glBindBuffer(GL_ARRAY_BUFFER, 0); indices->Release();
  if (glIsBuffer(indexName)) return 45;
  GLuint unpack; glGenBuffers(1, &unpack); glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpack);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, 4, nullptr, GL_STATIC_DRAW);
  IDirect3DTexture8 *texture = reinterpret_cast<IDirect3DTexture8 *>(1);
  if (device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture) != D3D_OK || !texture) {
    EM_ASM({ document.body.dataset.result = 'missing-texture'; }); return 31;
  }
  D3DSURFACE_DESC textureDesc{};
  if (texture->GetLevelDesc(0, &textureDesc) != D3D_OK || textureDesc.Type != D3DRTYPE_SURFACE || textureDesc.Width != 1 || textureDesc.Height != 1) return 53;
  D3DLOCKED_RECT locked{};
  if (texture->LockRect(0, &locked, nullptr, 0) != D3D_OK || locked.Pitch != 4) return 46;
  auto *texels = static_cast<unsigned char *>(locked.pBits);
  texels[0] = 0; texels[1] = 0; texels[2] = 255; texels[3] = 255;
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 3);
  if (texture->UnlockRect(0) != D3D_OK || texture->UnlockRect(0) == D3D_OK) return 47;
  GLint unpackRowLength = 0; glGetIntegerv(GL_UNPACK_ROW_LENGTH, &unpackRowLength);
  if (unpackRowLength != 3) return 50;
  GLint unpackBinding = 0; glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBinding);
  if (unpackBinding != static_cast<GLint>(unpack)) return 52;
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0); glDeleteBuffers(1, &unpack);
  GLuint textureFrame; glGenFramebuffers(1, &textureFrame); glBindFramebuffer(GL_FRAMEBUFFER, textureFrame);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, static_cast<PW153Texture *>(texture)->texture(), 0);
  unsigned char texturePixel[4]{}; glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, texturePixel);
  if (texturePixel[0] < 250 || texturePixel[1] > 2 || texturePixel[2] > 2 || texturePixel[3] < 250) return 48;
  glBindFramebuffer(GL_FRAMEBUFFER, 0); glDeleteFramebuffers(1, &textureFrame);
  const GLuint textureName = static_cast<PW153Texture *>(texture)->texture(); texture->Release();
  if (glIsTexture(textureName)) return 49;
  if (device->CreateTexture(1, 1, 1, 0, D3DFMT_DXT1, D3DPOOL_MANAGED, &texture) == D3D_OK || texture) return 51;
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
