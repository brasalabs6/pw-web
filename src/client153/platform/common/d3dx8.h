/*
 * FILE: d3dx8.h (platform seam)
 *
 * DESCRIPTION: D3DX math aliases - the ported A3D math carries the same
 * layouts the original D3DX headers wrapped. See d3d8.h. The D3D8 vertex
 * shader declarator tokens, texture stage states and render states the
 * client sources name are the documented d3d8types.h values.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */
#ifndef PW153_PLATFORM_D3DX8_H
#define PW153_PLATFORM_D3DX8_H
#include "d3d8.h"
#include <cmath>

//	D3D8 vertex shader declarator tokens (documented d3d8types.h values).
#define D3DVSD_TOKENTYPESHIFT 29
#define D3DVSD_DATATYPESHIFT 16
#define D3DVSD_TOKEN_STREAM 1
#define D3DVSD_TOKEN_STREAMDATA 2
#define D3DVSD_MAKETOKENTYPE(TokenType) \
  (((unsigned)(TokenType) << D3DVSD_TOKENTYPESHIFT) & \
   (0x7u << D3DVSD_TOKENTYPESHIFT))
#define D3DVSD_STREAM(StreamNumber) \
  (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAM) | (StreamNumber))
#define D3DVSD_REG(VertexRegister, Type) \
  (D3DVSD_MAKETOKENTYPE(D3DVSD_TOKEN_STREAMDATA) | \
   ((Type) << D3DVSD_DATATYPESHIFT) | (VertexRegister))
#define D3DVSD_END() 0xFFFFFFFF
#define D3DVSDT_FLOAT1 0x00
#define D3DVSDT_FLOAT2 0x01
#define D3DVSDE_POSITION 0
#define D3DVSDE_BLENDWEIGHT 1
#define D3DVSDE_BLENDINDICES 2
#define D3DVSDE_NORMAL 3
#define D3DVSDE_DIFFUSE 5
#define D3DVSDE_SPECULAR 6
#define D3DVSDE_TEXCOORD0 7
#define D3DVSDE_TEXCOORD1 8

//	The image header record (the documented D3DXIMAGE_INFO layout; the
//	format fields carry the enum values as unsigned ints until the
//	graphics cycle defines the enums).
typedef struct _D3DXIMAGE_INFO {
  UINT Width;
  UINT Height;
  UINT Depth;
  UINT MipLevels;
  UINT Format;
  UINT ResourceType;
  UINT ImageFileFormat;
} D3DXIMAGE_INFO;

#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL 0x8876086C
#endif

//	The image file formats (the documented D3DXIMAGE_FILEFORMAT values).
#define D3DXIFF_BMP 0
#define D3DXIFF_JPG 1
#define D3DXIFF_DDS 2
#define D3DXIFF_DIB 3
#define D3DXIFF_HDR 4
#define D3DXIFF_PFM 5
#define D3DXIFF_PNG 6
#define D3DXIFF_PPM 7
#define D3DXIFF_TGA 8

//	The texture save: the render-target textures do not exist on the
//	browser target yet (the WebGL backend owns the real surface when it
//	registers); the save fails explicitly and the caller's error path
//	handles it.
inline HRESULT D3DXSaveTextureToFileA(const char *file, DWORD format,
                                     IDirect3DBaseTexture8 *texture,
                                     const PALETTEENTRY *palette) {
  (void)file;
  (void)format;
  (void)texture;
  (void)palette;
  return D3DERR_INVALIDCALL;
}

//	The surface save: the same explicit failure (no real surfaces on
//	the browser target yet).
inline HRESULT D3DXSaveSurfaceToFileA(const char *file, DWORD format,
                                      IDirect3DSurface8 *surface,
                                      const PALETTEENTRY *palette,
                                      const void *reserved = nullptr) {
  (void)file;
  (void)format;
  (void)surface;
  (void)palette;
  (void)reserved;
  return D3DERR_INVALIDCALL;
}
inline HRESULT D3DXSaveSurfaceToFileW(const wchar_t *file, DWORD format,
                                      IDirect3DSurface8 *surface,
                                      const PALETTEENTRY *palette,
                                      const void *reserved = nullptr) {
  (void)file;
  (void)format;
  (void)surface;
  (void)palette;
  (void)reserved;
  return D3DERR_INVALIDCALL;
}
inline HRESULT D3DXSaveSurfaceToFile(const wchar_t *file, DWORD format,
                                     IDirect3DSurface8 *surface,
                                     const PALETTEENTRY *palette,
                                     const void *reserved = nullptr) {
  return D3DXSaveSurfaceToFileW(file, format, surface, palette, reserved);
}

//	The surface copy/filter: the same explicit failure (no real
//	surfaces on the browser target yet).
inline HRESULT D3DXLoadSurfaceFromSurface(IDirect3DSurface8 *destination,
                                          const PALETTEENTRY *destinationPalette,
                                          const RECT *destinationRect,
                                          IDirect3DSurface8 *source,
                                          const PALETTEENTRY *sourcePalette,
                                          const RECT *sourceRect,
                                          DWORD filter, DWORD colorKey) {
  (void)destination;
  (void)destinationPalette;
  (void)destinationRect;
  (void)source;
  (void)sourcePalette;
  (void)sourceRect;
  (void)filter;
  (void)colorKey;
  return D3DERR_INVALIDCALL;
}

//	The D3DX default sentinel (the documented d3dx8 value).
#define D3DX_DEFAULT 0xFFFFFFFF

//	The D3DX texture filters (the documented d3dx8tex values).
#define D3DX_FILTER_NONE (1 << 0)
#define D3DX_FILTER_POINT (2 << 0)
#define D3DX_FILTER_LINEAR (3 << 0)
#define D3DX_FILTER_TRIANGLE (4 << 0)
#define D3DX_FILTER_BOX (5 << 0)

//	The image header read: the BBS download path has no files on the
//	browser target yet (the download seam fails explicitly), so the
//	read fails with D3DERR_INVALIDCALL and the caller's documented
//	fallback (the placeholder size) applies. The real header parse
//	lands with the graphics cycle when the download path delivers
//	real files.
inline HRESULT D3DXGetImageInfoFromFileA(const char *file,
                                         D3DXIMAGE_INFO *info) {
  (void)file;
  if (info) {
    info->Width = 0;
    info->Height = 0;
    info->Depth = 0;
    info->MipLevels = 0;
    info->Format = 0;
    info->ResourceType = 0;
    info->ImageFileFormat = 0;
  }
  return D3DERR_INVALIDCALL;
}
#define D3DVSDT_FLOAT3 0x02
#define D3DVSDT_FLOAT4 0x03
#define D3DVSDT_D3DCOLOR 0x04

//	D3D8 texture stage states (documented d3d8types.h values).
#define D3DTSS_TCI_CAMERASPACEPOSITION 0x20000
#define D3DTSS_MIPMAPLODBIAS 19

//	D3D8 fixed-function states the grass render reaches (documented
//	d3d8types.h values).
#define D3DFVF_NORMAL 0x0010
#define D3DRS_CLIPPLANEENABLE 152
#define D3DCLIPPLANE0 (1 << 0)

inline D3DXMATRIX *D3DXMatrixOrthoOffCenterLH(D3DXMATRIX *out, float left,
                                              float right, float bottom,
                                              float top, float nearZ, float farZ) {
  *out = D3DXMATRIX(2.f / (right - left), 0.f, 0.f, 0.f,
                    0.f, 2.f / (top - bottom), 0.f, 0.f,
                    0.f, 0.f, 1.f / (farZ - nearZ), 0.f,
                    (left + right) / (left - right),
                    (top + bottom) / (bottom - top),
                    nearZ / (nearZ - farZ), 1.f);
  return out;
}

//	The D3DX 4-component vector (the ported A3DVECTOR4 carries the same
//	layout the original D3DX header wrapped).
typedef A3DVECTOR4 D3DXVECTOR4;

//	D3DXMatrixShadow: the flattening matrix per the published Microsoft
//	formula (https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixshadow):
//	P = normalize(Plane); L = Light; d = -dot(P, L);
//	M[row][col] = P[row] * L[col] + (row == col ? d : 0).
inline D3DXMATRIX *D3DXMatrixShadow(D3DXMATRIX *out, const D3DXVECTOR4 *light,
                                    const D3DXPLANE *plane) {
  const float len = std::sqrt(plane->a * plane->a + plane->b * plane->b +
                              plane->c * plane->c);
  const float p[4] = {plane->a / len, plane->b / len, plane->c / len,
                      plane->d / len};
  const float l[4] = {light->x, light->y, light->z, light->w};
  const float d = -(p[0] * l[0] + p[1] * l[1] + p[2] * l[2] + p[3] * l[3]);
  for (int row = 0; row < 4; ++row)
    for (int col = 0; col < 4; ++col)
      out->m[row][col] = p[row] * l[col] + (row == col ? d : 0.0f);
  return out;
}

//	D3DXVec3TransformCoord: the row-vector transform of (x, y, z, 1)
//	with the perspective divide back into w = 1 (the documented
//	D3DXVec3TransformCoord contract).
inline D3DXVECTOR3 *D3DXVec3TransformCoord(D3DXVECTOR3 *out,
                                           const D3DXVECTOR3 *v,
                                           const D3DXMATRIX *m) {
  const float x = v->x, y = v->y, z = v->z;
  const float tx = x * m->_11 + y * m->_21 + z * m->_31 + m->_41;
  const float ty = x * m->_12 + y * m->_22 + z * m->_32 + m->_42;
  const float tz = x * m->_13 + y * m->_23 + z * m->_33 + m->_43;
  const float tw = x * m->_14 + y * m->_24 + z * m->_34 + m->_44;
  const float invW = 1.0f / tw;
  out->x = tx * invW;
  out->y = ty * invW;
  out->z = tz * invW;
  return out;
}

inline D3DXMATRIX *D3DXMatrixPerspectiveFovLH(D3DXMATRIX *out, float fovy,
                                               float aspect, float zn, float zf) {
  const float yscale = 1.0f / std::tan(fovy * 0.5f);
  for (int row = 0; row < 4; ++row)
    for (int col = 0; col < 4; ++col)
      out->m[row][col] = 0.0f;
  out->_11 = yscale / aspect;
  out->_22 = yscale;
  out->_33 = zf / (zf - zn);
  out->_34 = 1.0f;
  out->_43 = -zn * zf / (zf - zn);
  return out;
}

//	D3DXPlaneTransform: the plane transforms as a row vector by the
//	transposed matrix (the call sites pass the inverse-transposed VP to
//	move a world-space plane into clip space).
inline D3DXPLANE *D3DXPlaneTransform(D3DXPLANE *out, const D3DXPLANE *p,
                                     const D3DXMATRIX *m) {
  const float a = p->a;
  const float b = p->b;
  const float c = p->c;
  const float d = p->d;
  D3DXPLANE r;
  r.a = m->_11 * a + m->_12 * b + m->_13 * c + m->_14 * d;
  r.b = m->_21 * a + m->_22 * b + m->_23 * c + m->_24 * d;
  r.c = m->_31 * a + m->_32 * b + m->_33 * c + m->_34 * d;
  r.d = m->_41 * a + m->_42 * b + m->_43 * c + m->_44 * d;
  *out = r;
  return out;
}
#endif
