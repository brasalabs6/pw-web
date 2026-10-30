#pragma once
#include "d3d8.h"
#include <GLES3/gl3.h>

// Owns one WebGL renderbuffer; callers own the returned COM reference.
class PW153Surface final : public IDirect3DSurface8 {
  ULONG references_ = 1;
  GLuint buffer_;
  D3DSURFACE_DESC description_;
public:
  PW153Surface(GLuint buffer, const D3DSURFACE_DESC &description);
  ~PW153Surface() override;
  GLuint renderbuffer() const { return buffer_; }
  HRESULT QueryInterface(const void *, void **out) override;
  ULONG AddRef() override;
  ULONG Release() override;
  HRESULT LockRect(D3DLOCKED_RECT *, const RECT *, DWORD) override;
  HRESULT UnlockRect() override;
  HRESULT GetDesc(void *out) override;
};
PW153Surface *pw153_create_surface(UINT width, UINT height, D3DFORMAT format);
IDirect3DSurface8 *pw153_create_image_surface(UINT width, UINT height, D3DFORMAT format);

//	Represents an attachment of the DEFAULT framebuffer (the canvas): the
//	back buffer colour attachment or the context's depth attachment. It
//	owns no GL storage — rendering goes straight to the canvas — so
//	LockRect fails explicitly (the default framebuffer is not CPU-lockable
//	without a readback) and the desc reports the real backend format.
class PW153DefaultSurface final : public IDirect3DSurface8 {
  ULONG references_ = 1;
  D3DSURFACE_DESC description_;
public:
  PW153DefaultSurface(UINT width, UINT height, D3DFORMAT format);
  ~PW153DefaultSurface() override;
  HRESULT QueryInterface(const void *, void **out) override;
  ULONG AddRef() override;
  ULONG Release() override;
  HRESULT LockRect(D3DLOCKED_RECT *, const RECT *, DWORD) override;
  HRESULT UnlockRect() override;
  HRESULT GetDesc(void *out) override;
};
