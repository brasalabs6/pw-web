#include "d3d8.h"
#include <cstddef>

// D3D8 SDK layout: verified against Wine's d3d8caps.h.
static_assert(sizeof(D3DCAPS8) == 212, "D3DCAPS8 size");
static_assert(D3D_OK == 0, "D3D success code");
static_assert(D3DLOCK_READONLY == 0x0010, "D3D read-only lock flag");
static_assert(offsetof(D3DCAPS8, ExtentsAdjust) == 132, "D3DCAPS8 extents");
static_assert(offsetof(D3DCAPS8, MaxVertexIndex) == 184, "D3DCAPS8 vertex index");
static_assert(offsetof(D3DCAPS8, MaxVertexShaderConst) == 200, "D3DCAPS8 shader constants");
static_assert(offsetof(D3DCAPS8, MaxPixelShaderValue) == 208, "D3DCAPS8 pixel shader value");
int main() {}
