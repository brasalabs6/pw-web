#pragma once
#include "d3d8.h"
#include <GLES3/gl3.h>
#include <emscripten/html5.h>

class PW153Texture final : public IDirect3DTexture8 {
  ULONG references_ = 1;
  IDirect3DDevice8 *device_;
  GLuint texture_;
  BYTE *pixels_;
  UINT width_, height_;
  UINT levels_;
  D3DFORMAT format_;
  D3DPOOL pool_;
  DWORD usage_;
  DWORD priority_ = 0, flags_ = 0;
  bool locked_ = false;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_;
public:
  PW153Texture(IDirect3DDevice8 *, GLuint, BYTE *, UINT, UINT, UINT, D3DFORMAT, D3DPOOL, DWORD, EMSCRIPTEN_WEBGL_CONTEXT_HANDLE);
  ~PW153Texture() override;
  GLuint texture() const { return texture_; }
  HRESULT QueryInterface(const void *, void **) override;
  ULONG AddRef() override;
  ULONG Release() override;
  HRESULT GetDevice(IDirect3DDevice8 **) override;
  HRESULT SetPrivateData(DWORD, const void *, DWORD, DWORD) override;
  HRESULT GetPrivateData(DWORD, void *, DWORD *) override;
  HRESULT FreePrivateData(DWORD) override;
  DWORD SetPriority(DWORD) override;
  DWORD GetPriority() override;
  DWORD SetLOD(DWORD) override;
  DWORD GetLOD() override;
  DWORD GetLevelCount() override;
  HRESULT GetLevelDesc(UINT, D3DSURFACE_DESC *) override;
  HRESULT GetSurfaceLevel(UINT, IDirect3DSurface8 **) override;
  HRESULT LockRect(UINT, D3DLOCKED_RECT *, const RECT *, DWORD) override;
  HRESULT UnlockRect(UINT) override;
};
class PW153TextureSurface final : public IDirect3DSurface8 {
  ULONG references_ = 1;
  PW153Texture *texture_;
public:
  explicit PW153TextureSurface(PW153Texture *texture);
  ~PW153TextureSurface() override;
  GLuint texture() const { return texture_->texture(); }
  HRESULT QueryInterface(const void *, void **) override;
  ULONG AddRef() override;
  ULONG Release() override;
  HRESULT LockRect(D3DLOCKED_RECT *, const RECT *, DWORD) override;
  HRESULT UnlockRect() override;
  HRESULT GetDesc(void *) override;
};
PW153Texture *pw153_create_texture(IDirect3DDevice8 *, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL);
