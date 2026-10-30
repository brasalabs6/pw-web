#include "d3d8_surface.h"
#include <emscripten/html5.h>
#include <new>
#include <cstdlib>

PW153Surface::PW153Surface(GLuint buffer, const D3DSURFACE_DESC &description)
  : buffer_(buffer), description_(description) {}
PW153Surface::~PW153Surface() { glDeleteRenderbuffers(1, &buffer_); }
HRESULT PW153Surface::QueryInterface(const void *, void **out) {
  if (out) *out = nullptr;
  return E_NOINTERFACE;
}
ULONG PW153Surface::AddRef() { return ++references_; }
ULONG PW153Surface::Release() { auto count = --references_; if (!count) delete this; return count; }
HRESULT PW153Surface::LockRect(D3DLOCKED_RECT *, const RECT *, DWORD) { return E_NOTIMPL; }
HRESULT PW153Surface::UnlockRect() { return E_NOTIMPL; }
HRESULT PW153Surface::GetDesc(void *out) {
  if (!out) return E_FAIL;
  *static_cast<D3DSURFACE_DESC *>(out) = description_;
  return D3D_OK;
}
PW153Surface *pw153_create_surface(UINT width, UINT height, D3DFORMAT format) {
  auto context = emscripten_webgl_get_current_context();
  if (!context || emscripten_is_webgl_context_lost(context) || !width || !height) return nullptr;
  GLenum storage;
  UINT bytes;
  switch (format) {
    case D3DFMT_R5G6B5: storage = GL_RGB565; bytes = 2; break;
    case D3DFMT_X8R8G8B8: case D3DFMT_A8R8G8B8: storage = GL_RGBA8; bytes = 4; break;
    case D3DFMT_D16: storage = GL_DEPTH_COMPONENT16; bytes = 2; break;
    case D3DFMT_D24X8: storage = GL_DEPTH_COMPONENT24; bytes = 4; break;
    default: return nullptr;
  }
  GLint limit = 0, previous = 0;
  glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &limit);
  if (limit <= 0 || width > static_cast<UINT>(limit) || height > static_cast<UINT>(limit) ||
      static_cast<uint64_t>(width) * height * bytes > UINT32_MAX) return nullptr;
  glGetIntegerv(GL_RENDERBUFFER_BINDING, &previous);
  GLuint buffer = 0;
  glGenRenderbuffers(1, &buffer);
  if (!buffer) return nullptr;
  glBindRenderbuffer(GL_RENDERBUFFER, buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, storage, width, height);
  GLint actualWidth = 0, actualHeight = 0;
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &actualWidth);
  glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &actualHeight);
  glBindRenderbuffer(GL_RENDERBUFFER, static_cast<GLuint>(previous));
  if (actualWidth != static_cast<GLint>(width) || actualHeight != static_cast<GLint>(height)) {
    glDeleteRenderbuffers(1, &buffer); return nullptr;
  }
  D3DSURFACE_DESC desc{};
  desc.Format = format; desc.Type = D3DRTYPE_SURFACE; desc.Pool = D3DPOOL_DEFAULT;
  desc.Size = width * height * bytes; desc.Width = width; desc.Height = height;
  auto *surface = new (std::nothrow) PW153Surface(buffer, desc);
  if (!surface) glDeleteRenderbuffers(1, &buffer);
  return surface;
}

namespace {
class ImageSurface final : public IDirect3DSurface8 {
  ULONG refs_ = 1;
  D3DSURFACE_DESC desc_{};
  BYTE *pixels_;
  bool locked_ = false;
  unsigned bytes_;
public:
  ImageSurface(UINT width, UINT height, D3DFORMAT format, unsigned bytes, BYTE *pixels)
      : pixels_(pixels), bytes_(bytes) {
    desc_.Format = format; desc_.Type = D3DRTYPE_SURFACE; desc_.Pool = D3DPOOL_SYSTEMMEM;
    desc_.Width = width; desc_.Height = height; desc_.Size = width * height * bytes;
  }
  ~ImageSurface() override { std::free(pixels_); }
  HRESULT QueryInterface(const void *, void **out) override { if (out) *out = nullptr; return E_NOINTERFACE; }
  ULONG AddRef() override { return ++refs_; }
  ULONG Release() override { ULONG refs = --refs_; if (!refs) delete this; return refs; }
  HRESULT GetDesc(void *out) override { if (!out) return E_FAIL; *static_cast<D3DSURFACE_DESC *>(out) = desc_; return D3D_OK; }
  HRESULT LockRect(D3DLOCKED_RECT *out, const RECT *rect, DWORD flags) override {
    if (!out) return E_FAIL;
    out->pBits = nullptr; out->Pitch = 0;
    if (locked_ || (flags & ~D3DLOCK_READONLY)) return E_FAIL;
    LONG x = 0, y = 0;
    if (rect) {
      if (rect->left < 0 || rect->top < 0 || rect->right <= rect->left || rect->bottom <= rect->top ||
          static_cast<UINT>(rect->right) > desc_.Width || static_cast<UINT>(rect->bottom) > desc_.Height) return E_FAIL;
      x = rect->left; y = rect->top;
    }
    locked_ = true;
    out->Pitch = static_cast<INT>(desc_.Width * bytes_);
    out->pBits = pixels_ + (static_cast<size_t>(y) * desc_.Width + x) * bytes_;
    return D3D_OK;
  }
  HRESULT UnlockRect() override { if (!locked_) return E_FAIL; locked_ = false; return D3D_OK; }
};
}

IDirect3DSurface8 *pw153_create_image_surface(UINT width, UINT height, D3DFORMAT format) {
  unsigned bytes = 0;
  switch (format) {
    case D3DFMT_A8R8G8B8: case D3DFMT_X8R8G8B8: bytes = 4; break;
    case D3DFMT_A4R4G4B4: case D3DFMT_R5G6B5: case D3DFMT_A1R5G5B5: bytes = 2; break;
    default: return nullptr;
  }
  const uint64_t size = static_cast<uint64_t>(width) * height * bytes;
  if (!width || !height || size > INT32_MAX || static_cast<uint64_t>(width) * bytes > INT32_MAX) return nullptr;
  auto *pixels = static_cast<BYTE *>(std::calloc(static_cast<size_t>(size), 1));
  if (!pixels) return nullptr;
  auto *surface = new (std::nothrow) ImageSurface(width, height, format, bytes, pixels);
  if (!surface) std::free(pixels);
  return surface;
}

// --- Default framebuffer surfaces (the canvas attachments) -----------------

PW153DefaultSurface::PW153DefaultSurface(UINT width, UINT height, D3DFORMAT format) {
  description_ = {};
  description_.Format = format;
  description_.Type = D3DRTYPE_SURFACE;
  description_.Usage = 0;
  description_.Pool = D3DPOOL_DEFAULT;
  description_.Width = width;
  description_.Height = height;
}
PW153DefaultSurface::~PW153DefaultSurface() = default;
HRESULT PW153DefaultSurface::QueryInterface(const void *, void **out) {
  if (out) *out = nullptr;
  return E_NOINTERFACE;
}
ULONG PW153DefaultSurface::AddRef() { return ++references_; }
ULONG PW153DefaultSurface::Release() {
  const auto count = --references_;
  if (!count) delete this;
  return count;
}
HRESULT PW153DefaultSurface::LockRect(D3DLOCKED_RECT *, const RECT *, DWORD) {
  // The default framebuffer is not CPU-lockable in WebGL without a
  // readback; fail explicitly instead of pretending.
  return E_NOTIMPL;
}
HRESULT PW153DefaultSurface::UnlockRect() { return E_FAIL; }
HRESULT PW153DefaultSurface::GetDesc(void *out) {
  if (!out) return E_FAIL;
  *static_cast<D3DSURFACE_DESC *>(out) = description_;
  return D3D_OK;
}
