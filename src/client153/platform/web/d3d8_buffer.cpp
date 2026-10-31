#include "d3d8_buffer.h"
#include <cstdlib>
#include <cstring>
#include <new>

PW153VertexBuffer::PW153VertexBuffer(GLuint buffer, BYTE *data, UINT size, DWORD usage, EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)
  : buffer_(buffer), data_(data), size_(size), usage_(usage), context_(context) {}
PW153VertexBuffer::~PW153VertexBuffer() {
  if (emscripten_webgl_get_current_context() == context_) glDeleteBuffers(1, &buffer_);
  std::free(data_);
}
HRESULT PW153VertexBuffer::QueryInterface(const void *, void **out) { if (out) *out = nullptr; return E_NOINTERFACE; }
ULONG PW153VertexBuffer::AddRef() { return ++references_; }
ULONG PW153VertexBuffer::Release() { auto count = --references_; if (!count) delete this; return count; }
HRESULT PW153VertexBuffer::Lock(UINT offset, UINT size, BYTE **out, DWORD flags) {
  if (!out) return E_FAIL;
  *out = nullptr;
  constexpr DWORD supported = D3DLOCK_READONLY | D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE;
  if (locked_ || offset > size_ || flags & ~supported ||
      ((flags & D3DLOCK_READONLY) && (usage_ & D3DUSAGE_WRITEONLY)) ||
      ((flags & D3DLOCK_READONLY) && (flags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE))) ||
      ((flags & D3DLOCK_DISCARD) && (!(usage_ & D3DUSAGE_DYNAMIC) || (flags & D3DLOCK_NOOVERWRITE)))) return E_FAIL;
  const UINT length = size ? size : size_ - offset;
  if (!length || length > size_ - offset) return E_FAIL;
  if (flags & D3DLOCK_DISCARD) std::memset(data_, 0, size_);
  offset_ = offset; length_ = length; lockFlags_ = flags; locked_ = true;
  *out = data_ + offset;
  return D3D_OK;
}
HRESULT PW153VertexBuffer::Unlock() {
  if (!locked_) return E_FAIL;
  locked_ = false;
  if (emscripten_webgl_get_current_context() != context_ || emscripten_is_webgl_context_lost(context_)) return E_FAIL;
  if (lockFlags_ & D3DLOCK_READONLY) return D3D_OK;
  GLint previous = 0; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previous);
  glBindBuffer(GL_ARRAY_BUFFER, buffer_);
  if (lockFlags_ & D3DLOCK_DISCARD) glBufferSubData(GL_ARRAY_BUFFER, 0, size_, data_);
  else glBufferSubData(GL_ARRAY_BUFFER, offset_, length_, data_ + offset_);
  glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previous));
  return glGetError() == GL_NO_ERROR ? D3D_OK : E_FAIL;
}
PW153VertexBuffer *pw153_create_vertex_buffer(UINT size, DWORD usage, D3DPOOL pool) {
  const auto context = emscripten_webgl_get_current_context();
  if (context <= 0 || emscripten_is_webgl_context_lost(context) || !size ||
      size > static_cast<UINT>(INT32_MAX) || (pool != D3DPOOL_DEFAULT && pool != D3DPOOL_MANAGED) ||
      (usage & ~(D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY | D3DUSAGE_SOFTWAREPROCESSING))) return nullptr;
  auto *data = static_cast<BYTE *>(std::calloc(1, size));
  if (!data) return nullptr;
  GLint previous = 0; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previous);
  GLuint buffer = 0; glGenBuffers(1, &buffer);
  if (!buffer) { std::free(data); return nullptr; }
  glBindBuffer(GL_ARRAY_BUFFER, buffer);
  glBufferData(GL_ARRAY_BUFFER, size, data, usage & D3DUSAGE_DYNAMIC ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
  GLint actual = 0; glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &actual);
  glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(previous));
  if (actual != static_cast<GLint>(size)) { glDeleteBuffers(1, &buffer); std::free(data); return nullptr; }
  auto *result = new (std::nothrow) PW153VertexBuffer(buffer, data, size, usage, context);
  if (!result) { glDeleteBuffers(1, &buffer); std::free(data); }
  return result;
}

PW153IndexBuffer::PW153IndexBuffer(PW153VertexBuffer *storage, D3DFORMAT format) : storage_(storage), format_(format) {}
PW153IndexBuffer::~PW153IndexBuffer() { storage_->Release(); }
HRESULT PW153IndexBuffer::QueryInterface(const void *, void **out) { if (out) *out = nullptr; return E_NOINTERFACE; }
ULONG PW153IndexBuffer::AddRef() { return ++references_; }
ULONG PW153IndexBuffer::Release() { auto count = --references_; if (!count) delete this; return count; }
HRESULT PW153IndexBuffer::Lock(UINT offset, UINT size, BYTE **out, DWORD flags) { return storage_->Lock(offset, size, out, flags); }
HRESULT PW153IndexBuffer::Unlock() { return storage_->Unlock(); }
PW153IndexBuffer *pw153_create_index_buffer(UINT size, DWORD usage, D3DFORMAT format, D3DPOOL pool) {
  const UINT stride = format == D3DFMT_INDEX16 ? 2 : format == D3DFMT_INDEX32 ? 4 : 0;
  if (!stride || size % stride) return nullptr;
  auto *storage = pw153_create_vertex_buffer(size, usage, pool);
  if (!storage) return nullptr;
  auto *result = new (std::nothrow) PW153IndexBuffer(storage, format);
  if (!result) storage->Release();
  return result;
}
