/*
 * FILE: d3d8.h (platform seam)
 *
 * DESCRIPTION: The Direct3D 8 type boundary the original client compiles
 * against. The interfaces carry the real documented Win32 signatures for the
 * methods the client calls; the WebGL2 renderer adapter implements them in
 * the renderer cycle. D3DX math types alias the ported A3D math (the original
 * D3DX headers wrap the same layouts). Sources that call methods not declared
 * here keep failing on those methods - the honest compile-first signal.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_D3D8_H
#define PW153_PLATFORM_D3D8_H

#define D3D_OK 0
#define D3DLOCK_READONLY 0x0010
#define D3DCLEAR_TARGET 0x00000001
#define D3DCLEAR_ZBUFFER 0x00000002
#define D3DFVF_XYZRHW 0x0004
#define D3DFVF_DIFFUSE 0x0040
#define D3DFVF_SPECULAR 0x0080
#define D3DFVF_TEX1 0x0100
#define D3DFVF_TEX2 0x0200

#include "Windows.h"

//	D3DX math aliases: the ported A3D math carries the same layouts and the
//	original D3DX headers wrap them.
#include "A3DVector.h"
#include "A3DMatrix.h"
#include "A3DQuaternion.h"
#include "A3DGeometry.h"	//	A3DPLANE backs D3DXPLANE.
typedef A3DVECTOR3 D3DXVECTOR3;
typedef A3DPLANE D3DXPLANE;
typedef A3DMATRIX4 D3DXMATRIX;
typedef A3DQUATERNION D3DXQUATERNION;

struct D3DRECT {
  LONG x1;
  LONG y1;
  LONG x2;
  LONG y2;
};

struct D3DLOCKED_RECT {
  INT Pitch;
  void *pBits;
};

struct D3DDISPLAYMODE {
  UINT Width;
  UINT Height;
  UINT RefreshRate;
  UINT Format;
};

struct D3DPRESENT_PARAMETERS {
  UINT BackBufferWidth;
  UINT BackBufferHeight;
  UINT BackBufferFormat;
  UINT BackBufferCount;
  DWORD MultiSampleType;
  DWORD SwapEffect;
  HWND hDeviceWindow;
  BOOL Windowed;
  BOOL EnableAutoDepthStencil;
  DWORD AutoDepthStencilFormat;
  DWORD Flags;
  UINT FullScreen_RefreshRateInHz;
  UINT FullScreen_PresentationInterval;
};

//	COM base: the three methods every interface inherits.
struct IUnknownD3D {
  virtual ~IUnknownD3D() = default;
  virtual HRESULT QueryInterface(const void *riid, void **ppvObject) = 0;
  virtual ULONG AddRef() = 0;
  virtual ULONG Release() = 0;
};

struct IDirect3DSurface8 : IUnknownD3D {
  virtual HRESULT LockRect(D3DLOCKED_RECT *pLockedRect, const RECT *pRect,
                           DWORD flags) = 0;
  virtual HRESULT UnlockRect() = 0;
  virtual HRESULT GetDesc(void *pDesc) = 0;
};

struct IDirect3DTexture8 : IUnknownD3D {
  virtual HRESULT GetSurfaceLevel(UINT level, IDirect3DSurface8 **ppSurface) = 0;
  virtual HRESULT LockRect(UINT level, D3DLOCKED_RECT *pLockedRect,
                           const RECT *pRect, DWORD flags) = 0;
  virtual HRESULT UnlockRect(UINT level) = 0;
};

struct IDirect3DVertexBuffer8 : IUnknownD3D {
  virtual HRESULT Lock(UINT offset, UINT size, BYTE **ppbData, DWORD flags) = 0;
  virtual HRESULT Unlock() = 0;
};

struct IDirect3DIndexBuffer8 : IUnknownD3D {
  virtual HRESULT Lock(UINT offset, UINT size, BYTE **ppbData, DWORD flags) = 0;
  virtual HRESULT Unlock() = 0;
};

struct IDirect3DDevice8 : IUnknownD3D {
  virtual HRESULT Clear(DWORD count, const D3DRECT *pRects, DWORD flags,
                        DWORD color, float z, DWORD stencil) = 0;
};

struct IDirect3DSwapChain8 : IUnknownD3D {
  virtual HRESULT Present(const RECT *pSourceRect, const RECT *pDestRect,
                          HWND hDestWindow, const void *pDirtyRegion) = 0;
};

struct D3DCAPS8 {
  UINT DeviceType;
  DWORD AdapterOrdinal;
  DWORD Caps;
  DWORD Caps2;
  DWORD Caps3;
  DWORD PresentationIntervals;
  DWORD CursorCaps;
  DWORD DevCaps;
  DWORD PrimitiveMiscCaps;
  DWORD RasterCaps;
  DWORD ZCmpCaps;
  DWORD SrcBlendCaps;
  DWORD DestBlendCaps;
  DWORD AlphaCmpCaps;
  DWORD ShadeCaps;
  DWORD TextureCaps;
  DWORD TextureFilterCaps;
  DWORD CubeTextureFilterCaps;
  DWORD VolumeTextureFilterCaps;
  DWORD TextureAddressCaps;
  DWORD VolumeTextureAddressCaps;
  DWORD LineCaps;
  DWORD MaxTextureWidth;
  DWORD MaxTextureHeight;
  DWORD MaxVolumeExtent;
  DWORD MaxTextureRepeat;
  DWORD MaxTextureAspectRatio;
  DWORD MaxAnisotropy;
  float MaxVertexW;
  float GuardBandLeft;
  float GuardBandTop;
  float GuardBandRight;
  float GuardBandBottom;
  float ExtentsAdjust;
  DWORD StencilCaps;
  DWORD FVFCaps;
  DWORD TextureOpCaps;
  DWORD MaxTextureBlendStages;
  DWORD MaxSimultaneousTextures;
  DWORD VertexProcessingCaps;
  DWORD MaxActiveLights;
  DWORD MaxUserClipPlanes;
  DWORD MaxVertexBlendMatrices;
  DWORD MaxVertexBlendMatrixIndex;
  float MaxPointSize;
  DWORD MaxPrimitiveCount;
  DWORD MaxVertexIndex;
  DWORD MaxStreams;
  DWORD MaxStreamStride;
  DWORD VertexShaderVersion;
  DWORD MaxVertexShaderConst;
  DWORD PixelShaderVersion;
  float MaxPixelShaderValue;
};

struct ID3DXBuffer : IUnknownD3D {
  virtual void *GetBufferPointer() = 0;
  virtual DWORD GetBufferSize() = 0;
};
typedef ID3DXBuffer *LPD3DXBUFFER;

struct IDirect3D8 : IUnknownD3D {
  virtual HRESULT CheckDeviceType(UINT adapter, DWORD checkType,
                                  DWORD displayFormat, DWORD backBufferFormat,
                                  BOOL windowed) = 0;
  virtual HRESULT CreateDevice(UINT adapter, DWORD deviceType, HWND focusWindow,
                               DWORD behaviorFlags,
                               D3DPRESENT_PARAMETERS *pPresentationParameters,
                               IDirect3DDevice8 **ppDevice) = 0;
};

#endif // PW153_PLATFORM_D3D8_H
