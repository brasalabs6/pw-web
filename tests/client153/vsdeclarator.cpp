//	D3DXDeclaratorFromFVF contract test: the declarator the seam builds
//	from an FVF must follow the documented DX8 D3DX mapping (stream token
//	first, one D3DVSD_REG per FVF component in vertex-data order bound to
//	the fixed-function registers, D3DVSD_END terminator, documented
//	texture-size-bit decoding). The engine's own FVFs (A3DVertex.h) pin
//	the exercised surface.
#include "A3DVertex.h"
#include "d3dx8.h"
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
int main() {
  DWORD decl[MAX_FVF_DECL_SIZE];

  //	A3DFVF_A3DVERTEX = XYZ|NORMAL|TEX1: position, normal, tex0.
  CHECK(D3DXDeclaratorFromFVF(A3DFVF_A3DVERTEX, decl) == D3D_OK);
  CHECK(decl[0] == D3DVSD_STREAM(0));
  CHECK(decl[1] == D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));
  CHECK(decl[2] == D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3));
  CHECK(decl[3] == D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2));
  CHECK(decl[4] == D3DVSD_END());
  CHECK(decl[5] == 0);

  //	A3DFVF_A3DLVERTEX = XYZ|DIFFUSE|SPECULAR|TEX1.
  CHECK(D3DXDeclaratorFromFVF(A3DFVF_A3DLVERTEX, decl) == D3D_OK);
  CHECK(decl[1] == D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));
  CHECK(decl[2] == D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR));
  CHECK(decl[3] == D3DVSD_REG(D3DVSDE_SPECULAR, D3DVSDT_D3DCOLOR));
  CHECK(decl[4] == D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2));
  CHECK(decl[5] == D3DVSD_END());

  //	A3DFVF_A3DTLVERTEX = XYZRHW|DIFFUSE|SPECULAR|TEX1: pre-transformed
  //	position is FLOAT4.
  CHECK(D3DXDeclaratorFromFVF(A3DFVF_A3DTLVERTEX, decl) == D3D_OK);
  CHECK(decl[1] == D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT4));
  CHECK(decl[4] == D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2));
  CHECK(decl[5] == D3DVSD_END());

  //	A3DFVF_A3DBVERTEX1 = XYZB2|LASTBETA_UBYTE4|NORMAL|TEX1: one blend
  //	weight then UBYTE4 blend indices.
  CHECK(D3DXDeclaratorFromFVF(A3DFVF_A3DBVERTEX1, decl) == D3D_OK);
  CHECK(decl[1] == D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3));
  CHECK(decl[2] == D3DVSD_REG(D3DVSDE_BLENDWEIGHT, D3DVSDT_FLOAT1));
  CHECK(decl[3] == D3DVSD_REG(D3DVSDE_BLENDINDICES, D3DVSDT_UBYTE4));
  CHECK(decl[4] == D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3));
  CHECK(decl[5] == D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2));
  CHECK(decl[6] == D3DVSD_END());

  //	Plain XYZB3 (an alternative to the XYZ bit): three blend
//	weights, no indices.
  CHECK(D3DXDeclaratorFromFVF(D3DFVF_XYZB3, decl) == D3D_OK);
  CHECK(decl[2] == D3DVSD_REG(D3DVSDE_BLENDWEIGHT, D3DVSDT_FLOAT3));
  CHECK(decl[3] == D3DVSD_END());

  //	Plain XYZB5: four weights then UBYTE4 blend indices.
  CHECK(D3DXDeclaratorFromFVF(D3DFVF_XYZB5, decl) == D3D_OK);
  CHECK(decl[2] == D3DVSD_REG(D3DVSDE_BLENDWEIGHT, D3DVSDT_FLOAT4));
  CHECK(decl[3] == D3DVSD_REG(D3DVSDE_BLENDINDICES, D3DVSDT_UBYTE4));
  CHECK(decl[4] == D3DVSD_END());

  //	The documented texture-size bits: TEX2 with set 0 at size 3 and set
  //	1 at size 4 (2 bits per set at bit 16; 1=FLOAT3, 2=FLOAT4).
  const DWORD fvfTex34 =
      D3DFVF_XYZ | D3DFVF_TEX2 | (1 << 16) | (2 << 18);
  CHECK(D3DXDeclaratorFromFVF(fvfTex34, decl) == D3D_OK);
  CHECK(decl[2] == D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT3));
  CHECK(decl[3] == D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT4));
  CHECK(decl[4] == D3DVSD_END());

  //	An FVF without a position component fails explicitly (non-zero
  //	HRESULT; the seam returns E_FAIL), as does a null declarator.
  CHECK(D3DXDeclaratorFromFVF(D3DFVF_NORMAL | D3DFVF_TEX1, decl) != D3D_OK);
  CHECK(D3DXDeclaratorFromFVF(A3DFVF_A3DVERTEX, NULL) != D3D_OK);

  std::cout << "client153_vsdeclarator: all checks passed\n";
  return 0;
}
