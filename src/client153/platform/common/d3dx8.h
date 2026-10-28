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
#define D3DVSDT_FLOAT2 0x01

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
  const float a = p->vNormal.x;
  const float b = p->vNormal.y;
  const float c = p->vNormal.z;
  const float d = p->fDist;
  D3DXPLANE r;
  r.vNormal.x = m->_11 * a + m->_12 * b + m->_13 * c + m->_14 * d;
  r.vNormal.y = m->_21 * a + m->_22 * b + m->_23 * c + m->_24 * d;
  r.vNormal.z = m->_31 * a + m->_32 * b + m->_33 * c + m->_34 * d;
  r.fDist = m->_41 * a + m->_42 * b + m->_43 * c + m->_44 * d;
  *out = r;
  return out;
}
#endif
