#include "d3d8.h"
#include <cstddef>

// D3D8 SDK layout: verified against Wine's d3d8caps.h.
static_assert(sizeof(D3DCAPS8) == 212, "D3DCAPS8 size");
static_assert(D3DFVF_TEX3 == 0x0300 && D3DFVF_TEX5 == 0x0500, "D3D texture coordinate counts");
static_assert(D3DFVF_XYZ == 0x002, "D3D position vertex format");
static_assert(D3DRS_ZWRITEENABLE == 14, "D3D depth write state");
static_assert(D3DCOLOR_RGBA(0x12, 0x34, 0x56, 0x78) == 0x78123456u, "D3D RGBA packing");
static_assert(D3D_OK == 0, "D3D success code");
static_assert(D3DLOCK_NOOVERWRITE == 0x1000, "D3D streaming lock flag");
static_assert(D3DLOCK_READONLY == 0x0010, "D3D read-only lock flag");
static_assert(D3DTSS_BORDERCOLOR == 15, "D3D border color state");
static_assert(D3DCLEAR_TARGET == 1 && D3DCLEAR_ZBUFFER == 2, "D3D clear flags");
static_assert((D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1) == 0x01c4,
              "transformed textured vertex format");
static_assert(FAILED(static_cast<HRESULT>(0x80004005u)), "failure HRESULT");
static_assert(SUCCEEDED(static_cast<HRESULT>(1)), "success HRESULT");
static_assert(offsetof(D3DCAPS8, ExtentsAdjust) == 132, "D3DCAPS8 extents");
static_assert(offsetof(D3DCAPS8, MaxVertexIndex) == 184, "D3DCAPS8 vertex index");
static_assert(offsetof(D3DCAPS8, MaxVertexShaderConst) == 200, "D3DCAPS8 shader constants");
static_assert(offsetof(D3DCAPS8, MaxPixelShaderValue) == 208, "D3DCAPS8 pixel shader value");
int main() {}
