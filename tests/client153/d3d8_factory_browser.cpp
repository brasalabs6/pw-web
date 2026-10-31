#include "d3d8.h"
#include "d3dx8.h"
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
  attributes.antialias = false;
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
  if (d3d->CheckDeviceFormat(0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_TEXTURE, D3DFMT_A4R4G4B4) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'font-format-unavailable'; }); return 67;
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
  IDirect3DSurface8 *image = nullptr;
  D3DLOCKED_RECT imageLock{};
  if (device->CreateImageSurface(2, 2, D3DFMT_A8R8G8B8, &image) != D3D_OK || !image ||
      image->LockRect(&imageLock, nullptr, 0) != D3D_OK || imageLock.Pitch != 8) {
    EM_ASM({ document.body.dataset.result = 'image-surface-not-lockable'; }); return 68;
  }
  static_cast<unsigned char *>(imageLock.pBits)[0] = 42;
  if (image->UnlockRect() != D3D_OK || image->LockRect(&imageLock, nullptr, D3DLOCK_READONLY) != D3D_OK ||
      static_cast<unsigned char *>(imageLock.pBits)[0] != 42 || image->UnlockRect() != D3D_OK) return 69;
  const unsigned char tga[] = {0,0,2,0,0,0,0,0,0,0,0,0,2,0,2,0,32,0x28,
                               0,0,255,255, 0,255,0,255, 255,0,0,255, 255,255,255,128};
  if (D3DXLoadSurfaceFromFileInMemory(image, nullptr, nullptr, tga, sizeof(tga) - 1,
                                      nullptr, 0, 0, nullptr) == D3D_OK) return 77;
  if (D3DXLoadSurfaceFromFileInMemory(image, nullptr, nullptr, tga, sizeof(tga),
                                      nullptr, 0, 0, nullptr) != D3D_OK ||
      image->LockRect(&imageLock, nullptr, D3DLOCK_READONLY) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'image-decode-failed'; }); return 70;
  }
  const auto *imagePixel = static_cast<const unsigned char *>(imageLock.pBits);
  if (imagePixel[0] != 0 || imagePixel[1] != 0 || imagePixel[2] != 255 || imagePixel[3] != 255 ||
      imagePixel[8] != 255 || imagePixel[9] != 0 || imagePixel[10] != 0 || imagePixel[11] != 255) return 71;
  if (image->UnlockRect() != D3D_OK) return 72;
  IDirect3DTexture8 *imageTexture = nullptr;
  IDirect3DSurface8 *textureSurface = nullptr;
  if (device->CreateTexture(2, 2, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &imageTexture) != D3D_OK ||
      imageTexture->GetSurfaceLevel(0, &textureSurface) != D3D_OK ||
      device->CopyRects(image, nullptr, 0, textureSurface, nullptr) != D3D_OK ||
      textureSurface->LockRect(&imageLock, nullptr, D3DLOCK_READONLY) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'image-copy-failed'; }); return 73;
  }
  imagePixel = static_cast<const unsigned char *>(imageLock.pBits);
  if (imagePixel[2] != 255 || imagePixel[3] != 255 || imagePixel[8] != 255) return 74;
  if (textureSurface->UnlockRect() != D3D_OK) return 75;
  GLuint imageFramebuffer = 0;
  glGenFramebuffers(1, &imageFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, imageFramebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         static_cast<PW153Texture *>(imageTexture)->texture(), 0);
  unsigned char gpuImagePixel[4]{};
  glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, gpuImagePixel);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &imageFramebuffer);
  if (gpuImagePixel[0] != 255 || gpuImagePixel[1] != 0 || gpuImagePixel[2] != 0 || gpuImagePixel[3] != 255) {
    EM_ASM({ document.body.dataset.result = 'image-gpu-copy-failed'; }); return 76;
  }
  struct UiVertex { float x, y, z, rhw; DWORD diffuse, specular; float u, v; };
  const UiVertex vertices[] = {{0, 0, 0, 1, 0xff800000, 0, .25f, .25f},
                               {32, 0, 0, 1, 0xff800000, 0, .25f, .25f},
                               {0, 16, 0, 1, 0xff800000, 0, .25f, .25f}};
  IDirect3DVertexBuffer8 *uiVertices = nullptr;
  BYTE *vertexData = nullptr;
  if (device->CreateVertexBuffer(sizeof(vertices), 0, 0, D3DPOOL_MANAGED, &uiVertices) != D3D_OK ||
      uiVertices->Lock(0, sizeof(vertices), &vertexData, 0) != D3D_OK) return 83;
  std::memcpy(vertexData, vertices, sizeof(vertices));
  if (uiVertices->Unlock() != D3D_OK ||
      device->SetStreamSource(0, uiVertices, sizeof(UiVertex)) != D3D_OK ||
      device->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1) != D3D_OK ||
      device->SetTexture(0, imageTexture) != D3D_OK) return 84;
  glViewport(0, 0, 32, 16);
  glDisable(GL_DEPTH_TEST); glDisable(GL_BLEND); glDisable(GL_CULL_FACE);
  glClearColor(0, 0, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
  if (device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'ui-draw-failed'; }); return 85;
  }
  unsigned char uiPixel[4]{};
  glReadPixels(4, 12, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, uiPixel);
  if (uiPixel[0] < 126 || uiPixel[0] > 130 || uiPixel[1] > 2 || uiPixel[2] > 2) {
    EM_ASM({ document.body.dataset.result = 'ui-pixel-failed'; }); return 86;
  }
  const UiVertex alphaVertices[] = {{0, 0, 0, 1, 0xffffffff, 0, .75f, .75f},
                                    {32, 0, 0, 1, 0xffffffff, 0, .75f, .75f},
                                    {0, 16, 0, 1, 0xffffffff, 0, .75f, .75f}};
  if (uiVertices->Lock(0, sizeof(alphaVertices), &vertexData, 0) != D3D_OK) return 87;
  std::memcpy(vertexData, alphaVertices, sizeof(alphaVertices));
  if (uiVertices->Unlock() != D3D_OK || device->SetRenderState(D3DRS_SRCBLEND, 5) != D3D_OK ||
      device->SetRenderState(D3DRS_DESTBLEND, 6) != D3D_OK ||
      device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'ui-blend-state-failed'; }); return 88;
  }
  glClearColor(0, 0, 1, 1); glClear(GL_COLOR_BUFFER_BIT);
  if (device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1) != D3D_OK) return 89;
  glReadPixels(4, 12, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, uiPixel);
  if (uiPixel[0] < 125 || uiPixel[0] > 130 || uiPixel[1] < 125 || uiPixel[1] > 130 || uiPixel[2] < 250) {
    EM_ASM({ document.body.dataset.result = 'ui-blend-pixel-failed'; }); return 90;
  }
  device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  device->SetTexture(0, nullptr); device->SetStreamSource(0, nullptr, 0); uiVertices->Release();
  textureSurface->Release(); imageTexture->Release();
  image->Release();
  unsigned char dds[144]{};
  auto dds32 = [&](int offset, unsigned value) {
    for (int i = 0; i < 4; ++i) dds[offset + i] = (value >> (i * 8)) & 255;
  };
  dds[0] = 'D'; dds[1] = 'D'; dds[2] = 'S'; dds[3] = ' ';
  dds32(4, 124); dds32(12, 4); dds32(16, 4); dds32(76, 32); dds32(80, 4); dds32(84, D3DFMT_DXT3);
  for (int i = 128; i < 136; ++i) dds[i] = 0xff;
  dds[136] = 0; dds[137] = 0xf8;
  IDirect3DSurface8 *ddsSurface = nullptr;
  if (device->CreateImageSurface(4, 4, D3DFMT_A8R8G8B8, &ddsSurface) != D3D_OK ||
      D3DXLoadSurfaceFromFileInMemory(ddsSurface, nullptr, nullptr, dds, sizeof(dds), nullptr, 0, 0, nullptr) != D3D_OK ||
      ddsSurface->LockRect(&imageLock, nullptr, D3DLOCK_READONLY) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'dds-decode-failed'; }); return 78;
  }
  imagePixel = static_cast<const unsigned char *>(imageLock.pBits);
  if (imagePixel[0] != 0 || imagePixel[1] != 0 || imagePixel[2] != 255 || imagePixel[3] != 255) return 79;
  if (ddsSurface->UnlockRect() != D3D_OK) return 80;
  ddsSurface->Release();
  IDirect3DTexture8 *loadedTexture = nullptr;
  D3DXIMAGE_INFO loadedInfo{};
  if (D3DXCreateTextureFromFileInMemoryEx(device, tga, sizeof(tga), D3DX_DEFAULT, D3DX_DEFAULT,
      1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, 0, 0, 0, &loadedInfo, nullptr, &loadedTexture) != D3D_OK ||
      !loadedTexture || loadedInfo.Width != 2 || loadedTexture->LockRect(0, &imageLock, nullptr, D3DLOCK_READONLY) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'texture-decode-failed'; }); return 81;
  }
  imagePixel = static_cast<const unsigned char *>(imageLock.pBits);
  if (imagePixel[2] != 255 || imagePixel[3] != 255 || loadedTexture->UnlockRect(0) != D3D_OK) return 82;
  loadedTexture->Release();
#ifdef PW153_IMAGE_SURFACE_ONLY
  EM_ASM({ document.body.dataset.result = 'pass'; });
  return 0;
#endif
  IDirect3DSurface8 *target = nullptr, *depth = nullptr, *back = nullptr;
  if (device->GetRenderTarget(&target) != D3D_OK || device->GetDepthStencilSurface(&depth) != D3D_OK ||
      device->GetBackBuffer(0, 0, &back) != D3D_OK || !target || !depth || !back) return 27;
  if (target != back) return 28;
  if (device->Clear(0, nullptr, D3DCLEAR_TARGET, 0xffff0000, 1.0f, 0) != D3D_OK) return 29;
  if (device->Present(nullptr, nullptr, nullptr, nullptr) != D3D_OK) {
    EM_ASM({ document.body.dataset.result = 'present-failed'; }); return 60;
  }
  unsigned char presented[4]{};
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, presented);
  if (glGetError() != GL_NO_ERROR || presented[0] < 250 || presented[1] > 2 || presented[2] > 2) {
    EM_ASM({ document.body.dataset.result = 'present-wrong-pixel'; }); return 61;
  }
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
  IDirect3DTexture8 *fontTexture = nullptr;
  if (device->CreateTexture(2, 1, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &fontTexture) != D3D_OK || !fontTexture) {
    EM_ASM({ document.body.dataset.result = 'missing-font-texture'; }); return 62;
  }
  D3DSURFACE_DESC fontDesc{};
  if (fontTexture->GetLevelDesc(0, &fontDesc) != D3D_OK || fontDesc.Format != D3DFMT_A4R4G4B4 || fontDesc.Size != 4) return 63;
  D3DLOCKED_RECT fontLock{};
  if (fontTexture->LockRect(0, &fontLock, nullptr, 0) != D3D_OK || fontLock.Pitch != 4) return 64;
  auto *fontPixels = static_cast<unsigned short *>(fontLock.pBits);
  fontPixels[0] = 0x8fff; fontPixels[1] = 0xff00;
  if (fontTexture->UnlockRect(0) != D3D_OK) return 65;
  RECT fontRect{1, 0, 2, 1};
  if (fontTexture->LockRect(0, &fontLock, &fontRect, D3DLOCK_READONLY) != D3D_OK ||
      *static_cast<unsigned short *>(fontLock.pBits) != 0xff00 || fontLock.Pitch != 4 ||
      fontTexture->UnlockRect(0) != D3D_OK) return 68;
  GLuint fontFrame; glGenFramebuffers(1, &fontFrame); glBindFramebuffer(GL_FRAMEBUFFER, fontFrame);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, static_cast<PW153Texture *>(fontTexture)->texture(), 0);
  unsigned char fontRead[8]{}; glReadPixels(0, 0, 2, 1, GL_RGBA, GL_UNSIGNED_BYTE, fontRead);
  if (fontRead[0] != 255 || fontRead[1] != 255 || fontRead[2] != 255 || fontRead[3] != 136 ||
      fontRead[4] != 255 || fontRead[5] != 0 || fontRead[6] != 0 || fontRead[7] != 255) return 66;
  glBindFramebuffer(GL_FRAMEBUFFER, 0); glDeleteFramebuffers(1, &fontFrame); fontTexture->Release();
  IDirect3DTexture8 *renderTexture = nullptr;
  if (device->CreateTexture(4, 4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &renderTexture) != D3D_OK || !renderTexture) {
    EM_ASM({ document.body.dataset.result = 'missing-render-texture'; }); return 54;
  }
  IDirect3DSurface8 *renderSurface = nullptr;
  if (renderTexture->GetSurfaceLevel(0, &renderSurface) != D3D_OK || !renderSurface) return 55;
  const GLuint renderTextureName = static_cast<PW153TextureSurface *>(renderSurface)->texture();
  renderTexture->Release();
  if (!glIsTexture(renderTextureName)) return 56;
  GLuint renderFrame; glGenFramebuffers(1, &renderFrame); glBindFramebuffer(GL_FRAMEBUFFER, renderFrame);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTextureName, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return 57;
  glClearColor(0, 1, 0, 1); glClear(GL_COLOR_BUFFER_BIT);
  unsigned char renderPixel[4]{}; glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, renderPixel);
  if (renderPixel[0] > 2 || renderPixel[1] < 250) return 58;
  glBindFramebuffer(GL_FRAMEBUFFER, 0); glDeleteFramebuffers(1, &renderFrame);
  renderSurface->Release(); if (glIsTexture(renderTextureName)) return 59;
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
