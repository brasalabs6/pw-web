/*
 * FILE: d3dx8.h (platform seam)
 *
 * DESCRIPTION: D3DX math aliases - the ported A3D math carries the same
 * layouts the original D3DX headers wrapped. See d3d8.h.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */
#ifndef PW153_PLATFORM_D3DX8_H
#define PW153_PLATFORM_D3DX8_H
#include "d3d8.h"
#include <cmath>

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
#endif
