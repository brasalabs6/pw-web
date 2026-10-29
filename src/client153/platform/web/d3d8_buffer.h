#pragma once
#include "d3d8.h"
#include <GLES3/gl3.h>
#include <emscripten/html5.h>

class PW153VertexBuffer final : public IDirect3DVertexBuffer8 {
  ULONG references_ = 1;
  GLuint buffer_;
  BYTE *data_;
  UINT size_, offset_ = 0, length_ = 0;
  DWORD usage_, lockFlags_ = 0;
  bool locked_ = false;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_;
public:
  PW153VertexBuffer(GLuint buffer, BYTE *data, UINT size, DWORD usage, EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context);
  ~PW153VertexBuffer() override;
  GLuint buffer() const { return buffer_; }
  HRESULT QueryInterface(const void *, void **out) override;
  ULONG AddRef() override;
  ULONG Release() override;
  HRESULT Lock(UINT offset, UINT size, BYTE **out, DWORD flags) override;
  HRESULT Unlock() override;
};
PW153VertexBuffer *pw153_create_vertex_buffer(UINT size, DWORD usage, D3DPOOL pool);

class PW153IndexBuffer final : public IDirect3DIndexBuffer8 {
  ULONG references_ = 1;
  PW153VertexBuffer *storage_;
  D3DFORMAT format_;
public:
  PW153IndexBuffer(PW153VertexBuffer *storage, D3DFORMAT format);
  ~PW153IndexBuffer() override;
  GLuint buffer() const { return storage_->buffer(); }
  D3DFORMAT format() const { return format_; }
  HRESULT QueryInterface(const void *, void **out) override;
  ULONG AddRef() override;
  ULONG Release() override;
  HRESULT Lock(UINT offset, UINT size, BYTE **out, DWORD flags) override;
  HRESULT Unlock() override;
};
PW153IndexBuffer *pw153_create_index_buffer(UINT size, DWORD usage, D3DFORMAT format, D3DPOOL pool);
