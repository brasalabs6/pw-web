#include "d3d8_texture.h"
#include <cstdlib>
#include <new>

PW153Texture::PW153Texture(IDirect3DDevice8 *device, GLuint texture, BYTE *pixels, UINT width, UINT height, D3DFORMAT format, D3DPOOL pool, DWORD usage, EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)
  : device_(device), texture_(texture), pixels_(pixels), width_(width), height_(height), format_(format), pool_(pool), usage_(usage), context_(context) { device_->AddRef(); }
PW153Texture::~PW153Texture() {
  if (emscripten_webgl_get_current_context() == context_) glDeleteTextures(1, &texture_);
  std::free(pixels_); device_->Release();
}
HRESULT PW153Texture::QueryInterface(const void *, void **out) { if (out) *out = nullptr; return E_NOINTERFACE; }
ULONG PW153Texture::AddRef() { return ++references_; }
ULONG PW153Texture::Release() { auto count = --references_; if (!count) delete this; return count; }
HRESULT PW153Texture::GetDevice(IDirect3DDevice8 **out) { if (!out) return E_FAIL; device_->AddRef(); *out = device_; return D3D_OK; }
HRESULT PW153Texture::SetPrivateData(DWORD, const void *, DWORD, DWORD) { return E_NOTIMPL; }
HRESULT PW153Texture::GetPrivateData(DWORD, void *, DWORD *) { return E_NOTIMPL; }
HRESULT PW153Texture::FreePrivateData(DWORD) { return E_NOTIMPL; }
DWORD PW153Texture::SetPriority(DWORD value) { auto old = priority_; priority_ = value; return old; }
DWORD PW153Texture::GetPriority() { return priority_; }
DWORD PW153Texture::SetLOD(DWORD) { return 0; }
DWORD PW153Texture::GetLOD() { return 0; }
DWORD PW153Texture::GetLevelCount() { return 1; }
HRESULT PW153Texture::GetLevelDesc(UINT level, D3DSURFACE_DESC *out) {
  if (level || !out) return E_FAIL;
  *out = {}; out->Format = format_; out->Type = D3DRTYPE_SURFACE;
  out->Pool = pool_; out->Usage = usage_; out->Width = width_; out->Height = height_; out->Size = width_ * height_ * (format_ == D3DFMT_A4R4G4B4 ? 2 : 4);
  return D3D_OK;
}
HRESULT PW153Texture::GetSurfaceLevel(UINT level, IDirect3DSurface8 **out) {
  if (!out) return E_FAIL;
  *out = nullptr;
  if (level) return E_FAIL;
  *out = new (std::nothrow) PW153TextureSurface(this);
  return *out ? D3D_OK : E_FAIL;
}
PW153TextureSurface::PW153TextureSurface(PW153Texture *texture) : texture_(texture) { texture_->AddRef(); }
PW153TextureSurface::~PW153TextureSurface() { texture_->Release(); }
HRESULT PW153TextureSurface::QueryInterface(const void *, void **out) { if (out) *out = nullptr; return E_NOINTERFACE; }
ULONG PW153TextureSurface::AddRef() { return ++references_; }
ULONG PW153TextureSurface::Release() { auto count = --references_; if (!count) delete this; return count; }
HRESULT PW153TextureSurface::LockRect(D3DLOCKED_RECT *out, const RECT *rect, DWORD flags) { return texture_->LockRect(0, out, rect, flags); }
HRESULT PW153TextureSurface::UnlockRect() { return texture_->UnlockRect(0); }
HRESULT PW153TextureSurface::GetDesc(void *out) { return texture_->GetLevelDesc(0, static_cast<D3DSURFACE_DESC *>(out)); }
HRESULT PW153Texture::LockRect(UINT level, D3DLOCKED_RECT *out, const RECT *rect, DWORD flags) {
  if (!out) return E_FAIL;
  out->pBits = nullptr; out->Pitch = 0;
  if (level || locked_ || !pixels_ || (flags & ~D3DLOCK_READONLY)) return E_FAIL;
  LONG x = 0, y = 0;
  if (rect) {
    if (rect->left < 0 || rect->top < 0 || rect->right <= rect->left || rect->bottom <= rect->top ||
        static_cast<UINT>(rect->right) > width_ || static_cast<UINT>(rect->bottom) > height_) return E_FAIL;
    x = rect->left; y = rect->top;
  }
  flags_ = flags; locked_ = true;
  const UINT bytes = format_ == D3DFMT_A4R4G4B4 ? 2 : 4;
  out->Pitch = static_cast<INT>(width_ * bytes);
  out->pBits = pixels_ + (static_cast<size_t>(y) * width_ + x) * bytes;
  return D3D_OK;
}
HRESULT PW153Texture::UnlockRect(UINT level) {
  if (level || !locked_) return E_FAIL;
  locked_ = false;
  if (emscripten_webgl_get_current_context() != context_ || emscripten_is_webgl_context_lost(context_)) return E_FAIL;
  if (flags_ & D3DLOCK_READONLY) return D3D_OK;
  const size_t size = static_cast<size_t>(width_) * height_ * 4;
  auto *rgba = static_cast<BYTE *>(std::malloc(size));
  if (!rgba) return E_FAIL;
  for (size_t i = 0; i < size; i += 4) {
    if (format_ == D3DFMT_A4R4G4B4) {
      const unsigned v = pixels_[i / 2] | (static_cast<unsigned>(pixels_[i / 2 + 1]) << 8);
      rgba[i] = ((v >> 8) & 15) * 17; rgba[i + 1] = ((v >> 4) & 15) * 17;
      rgba[i + 2] = (v & 15) * 17; rgba[i + 3] = ((v >> 12) & 15) * 17;
    } else {
      rgba[i] = pixels_[i + 2]; rgba[i + 1] = pixels_[i + 1]; rgba[i + 2] = pixels_[i]; rgba[i + 3] = pixels_[i + 3];
    }
  }
  GLint previous = 0, alignment = 0, rowLength = 0, skipRows = 0, skipPixels = 0, unpackBuffer = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment); glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
  glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows); glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
  glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0); glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previous));
  glPixelStorei(GL_UNPACK_ALIGNMENT, alignment); glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows); glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, static_cast<GLuint>(unpackBuffer));
  std::free(rgba);
  return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
}
PW153Texture *pw153_create_texture(IDirect3DDevice8 *device, UINT width, UINT height, UINT levels, DWORD usage, D3DFORMAT format, D3DPOOL pool) {
  auto context = emscripten_webgl_get_current_context();
  if (!device || context <= 0 || emscripten_is_webgl_context_lost(context) || !width || !height ||
      levels != 1 || (format != D3DFMT_A8R8G8B8 && format != D3DFMT_A4R4G4B4) ||
      !((usage == 0 && pool == D3DPOOL_MANAGED) || (format == D3DFMT_A8R8G8B8 && usage == D3DUSAGE_RENDERTARGET && pool == D3DPOOL_DEFAULT)) ||
      static_cast<uint64_t>(width) * height > INT32_MAX / 4) return nullptr;
  GLint limit = 0, previous = 0, unpackBuffer = 0; glGetIntegerv(GL_MAX_TEXTURE_SIZE, &limit);
  if (limit <= 0 || width > static_cast<UINT>(limit) || height > static_cast<UINT>(limit)) return nullptr;
  auto *pixels = usage ? nullptr : static_cast<BYTE *>(std::calloc(static_cast<size_t>(width) * height, format == D3DFMT_A4R4G4B4 ? 2 : 4));
  if (!usage && !pixels) return nullptr;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous);
  GLuint name = 0; glGenTextures(1, &name);
  if (!name) { std::free(pixels); return nullptr; }
  glBindTexture(GL_TEXTURE_2D, name);
  glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  const bool ok = glGetError() == GL_NO_ERROR;
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, static_cast<GLuint>(unpackBuffer));
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previous));
  if (!ok) { glDeleteTextures(1, &name); std::free(pixels); return nullptr; }
  auto *result = new (std::nothrow) PW153Texture(device, name, pixels, width, height, format, pool, usage, context);
  if (!result) { glDeleteTextures(1, &name); std::free(pixels); }
  return result;
}
