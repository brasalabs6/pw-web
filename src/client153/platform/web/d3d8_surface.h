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
