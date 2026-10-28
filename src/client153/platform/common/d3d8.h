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
#define D3DLOCK_NOOVERWRITE 0x1000
#define D3DCLEAR_TARGET 0x00000001
#define D3DCLEAR_ZBUFFER 0x00000002
#define D3DFVF_XYZ 0x002
#define D3DFVF_XYZRHW 0x0004
#define D3DFVF_DIFFUSE 0x0040
#define D3DFVF_SPECULAR 0x0080
#define D3DFVF_TEX1 0x0100
#define D3DFVF_TEX2 0x0200
#define D3DFVF_TEX3 0x0300
#define D3DFVF_TEX5 0x0500
#define D3DTSS_BORDERCOLOR 15
#define MAX_FVF_DECL_SIZE 20

constexpr unsigned int D3DCOLOR_RGBA(unsigned int r, unsigned int g, unsigned int b, unsigned int a) {
  return ((a & 0xffu) << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu);
}

#include "Windows.h"
#include "unknwn.h"	//	GUID (D3DADAPTER_IDENTIFIER8's DeviceIdentifier).

//	D3DX math aliases: the ported A3D math carries the same layouts and the
//	original D3DX headers wrap them.
#include "A3DVector.h"
#include "A3DMatrix.h"
#include "A3DQuaternion.h"
#include "A3DGeometry.h"	//	A3DPLANE backs D3DXPLANE.
typedef A3DVECTOR3 D3DXVECTOR3;
//	The D3DX plane (the documented d3dx8math.h layout with its
//	constructors).
typedef struct D3DXPLANE {
#ifdef __cplusplus
  D3DXPLANE() {}
  D3DXPLANE(const float *pf) {
    a = pf[0];
    b = pf[1];
    c = pf[2];
    d = pf[3];
  }
  D3DXPLANE(float fa, float fb, float fc, float fd)
      : a(fa), b(fb), c(fc), d(fd) {}
  //	cast operators (the documented d3dx8math.h surface).
  operator float *() { return &a; }
  operator const float *() const { return &a; }
#endif
  float a, b, c, d;
} D3DXPLANE;
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

struct IDirect3DVertexBuffer8 : IUnknownD3D {
  virtual HRESULT Lock(UINT offset, UINT size, BYTE **ppbData, DWORD flags) = 0;
  virtual HRESULT Unlock() = 0;
};

struct IDirect3DIndexBuffer8 : IUnknownD3D {
  virtual HRESULT Lock(UINT offset, UINT size, BYTE **ppbData, DWORD flags) = 0;
  virtual HRESULT Unlock() = 0;
};

constexpr DWORD D3DRS_ZWRITEENABLE = 14;
//	The render states the UI/media/engine surfaces name (the documented
//	d3d8types.h values).
constexpr DWORD D3DRS_ZENABLE = 8;
constexpr DWORD D3DRS_ZFUNC = 23;
constexpr DWORD D3DRS_ALPHAFUNC = 25;
constexpr DWORD D3DRS_ALPHAREF = 24;
constexpr DWORD D3DRS_ALPHATESTENABLE = 15;
constexpr DWORD D3DRS_ALPHABLENDENABLE = 27;
constexpr DWORD D3DRS_SRCBLEND = 19;
constexpr DWORD D3DRS_DESTBLEND = 20;
constexpr DWORD D3DRS_COLORWRITEENABLE = 168;
constexpr DWORD D3DCOLORWRITEENABLE_RED = 1;
constexpr DWORD D3DCOLORWRITEENABLE_GREEN = 2;
constexpr DWORD D3DCOLORWRITEENABLE_BLUE = 4;
constexpr DWORD D3DCOLORWRITEENABLE_ALPHA = 8;

//	The surface format (the documented d3d8types.h values for the
//	formats the client names).
enum D3DFORMAT : DWORD {
  D3DFMT_UNKNOWN = 0,
  D3DFMT_R5G6B5 = 23,
  D3DFMT_X1R5G5B5 = 24,
  D3DFMT_X8R8G8B8 = 22,
  D3DFMT_DXT1 = 0x31545844,	//	FOURCC 'DXT1'.
  D3DFMT_DXT3 = 0x33545844,	//	FOURCC 'DXT3'.
};

enum D3DMULTISAMPLE_TYPE : DWORD {
  D3DMULTISAMPLE_NONE = 0,
  D3DMULTISAMPLE_2_SAMPLES = 2,
  D3DMULTISAMPLE_4_SAMPLES = 4,
};

//	The surface description (the documented D3DSURFACE_DESC layout).
typedef struct _D3DSURFACE_DESC {
  D3DFORMAT Format;
  D3DMULTISAMPLE_TYPE MultiSampleType;
  DWORD MultiSampleQuality;
  UINT Width;
  UINT Height;
} D3DSURFACE_DESC;

//	The SDK version token (the documented DX8.1 value); the direct
//	factory returns none until the graphics backend registers (the
//	callers' null-device paths handle it).
#define D3D_SDK_VERSION 220

//	The shader version tokens (the documented d3d8types macros).
#define D3DPS_VERSION(major, minor) (0xFFFF0000 | ((major) << 8) | (minor))
#define D3DVS_VERSION(major, minor) (0xFFFE0000 | ((major) << 8) | (minor))

struct IDirect3DBaseTexture8;

// Port interface only; this subset is not a native Direct3D COM vtable.
struct IDirect3DDevice8 : IUnknownD3D {
  virtual HRESULT CopyRects(IDirect3DSurface8 *source,
                            const RECT *sourceRects, UINT rectCount,
                            IDirect3DSurface8 *destination,
                            const POINT *destinationPoints) = 0;
  virtual HRESULT GetRenderState(DWORD state, DWORD *value) = 0;
  virtual HRESULT Clear(DWORD count, const D3DRECT *pRects, DWORD flags,
                        DWORD color, float z, DWORD stencil) = 0;
  //	Texture stage binding (the wire collector unbinds stage 0).
  virtual HRESULT SetTexture(DWORD stage, IDirect3DBaseTexture8 *pTexture) = 0;
  //	User clip planes (the grass land render reads and rewrites them).
  virtual HRESULT GetClipPlane(DWORD index, float *pPlane) = 0;
  virtual HRESULT SetClipPlane(DWORD index, const float *pPlane) = 0;
  //	Render-target surface creation (the portrait renderer builds its
  //	lockable system-memory target; the WebGL backend owns the real
  //	surface when it registers).
  virtual HRESULT CreateRenderTarget(UINT width, UINT height, D3DFORMAT format,
                                     D3DMULTISAMPLE_TYPE multiSample,
                                     BOOL lockable,
                                     IDirect3DSurface8 **surface) = 0;
};

//	The texture resource hierarchy (the documented d3d8 interfaces:
//	resource -> base texture -> texture).
struct IDirect3DResource8 : IUnknownD3D {
  virtual HRESULT GetDevice(IDirect3DDevice8 **ppDevice) = 0;
  virtual HRESULT SetPrivateData(DWORD guid, const void *data,
                                 DWORD size, DWORD flags) = 0;
  virtual HRESULT GetPrivateData(DWORD guid, void *data, DWORD *size) = 0;
  virtual HRESULT FreePrivateData(DWORD guid) = 0;
  virtual DWORD SetPriority(DWORD priorityNew) = 0;
  virtual DWORD GetPriority() = 0;
};

struct IDirect3DBaseTexture8 : IDirect3DResource8 {
  virtual DWORD SetLOD(DWORD lodNew) = 0;
  virtual DWORD GetLOD() = 0;
  virtual DWORD GetLevelCount() = 0;
};

//	The palette entry lives in Windows.h (the GDI layout; the seam's
//	umbrella owns it).

struct IDirect3DTexture8 : IDirect3DBaseTexture8 {
  virtual HRESULT GetSurfaceLevel(UINT level, IDirect3DSurface8 **ppSurface) = 0;
  virtual HRESULT LockRect(UINT level, D3DLOCKED_RECT *pLockedRect,
                           const RECT *pRect, DWORD flags) = 0;
  virtual HRESULT UnlockRect(UINT level) = 0;
};

//	The cube texture (the documented d3d8 interface).
typedef struct _D3DLOCKED_BOX {
  int RowPitch;
  int SlicePitch;
  void *pBits;
} D3DLOCKED_BOX;

//	The cube map faces (the documented d3d8types enumeration).
typedef enum _D3DCUBEMAP_FACES {
  D3DCUBEMAP_FACE_POSITIVE_X = 0,
  D3DCUBEMAP_FACE_NEGATIVE_X = 1,
  D3DCUBEMAP_FACE_POSITIVE_Y = 2,
  D3DCUBEMAP_FACE_NEGATIVE_Y = 3,
  D3DCUBEMAP_FACE_POSITIVE_Z = 4,
  D3DCUBEMAP_FACE_NEGATIVE_Z = 5
} D3DCUBEMAP_FACES;

typedef struct _D3DSURFACE_DESC_CUBE {
  D3DFORMAT Format;
  D3DMULTISAMPLE_TYPE MultiSampleType;
  DWORD MultiSampleQuality;
  UINT Width;
  UINT Height;
} D3DSURFACE_DESC_CUBE;

struct IDirect3DCubeTexture8 : IDirect3DBaseTexture8 {
  virtual HRESULT GetLevelDesc(UINT level,
                               D3DSURFACE_DESC_CUBE *description) = 0;
  virtual HRESULT GetCubeMapSurface(D3DCUBEMAP_FACES faceType, UINT level,
                                    IDirect3DSurface8 **cubeMapSurface) = 0;
  virtual HRESULT LockBox(D3DCUBEMAP_FACES faceType, UINT level,
                          D3DLOCKED_BOX *lockedBox, const RECT *box,
                          DWORD flags) = 0;
  virtual HRESULT UnlockBox(D3DCUBEMAP_FACES faceType, UINT level) = 0;
  virtual HRESULT AddDirtyRect(D3DCUBEMAP_FACES faceType,
                               const RECT *dirtyRect) = 0;
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

//	The adapter identification (the documented D3DADAPTER_IDENTIFIER8
//	layout; the driver strings feed the defence collector's description).
#define MAX_DEVICE_IDENTIFIER_STRING 512

typedef struct _D3DADAPTER_IDENTIFIER8 {
  char Driver[MAX_DEVICE_IDENTIFIER_STRING];
  char Description[MAX_DEVICE_IDENTIFIER_STRING];
  char DeviceName[32];
  DWORD DriverVersionHighPart;
  DWORD DriverVersionLowPart;
  DWORD VendorId;
  DWORD DeviceId;
  DWORD SubSysId;
  DWORD Revision;
  GUID DeviceIdentifier;
  DWORD WHQLLevel;
} D3DADAPTER_IDENTIFIER8;

#define D3DADAPTER_DEFAULT 0
#define D3DENUM_NO_WHQL_LEVEL 0x00000002L

struct IDirect3D8 : IUnknownD3D {
  virtual HRESULT CheckDeviceType(UINT adapter, DWORD checkType,
                                  DWORD displayFormat, DWORD backBufferFormat,
                                  BOOL windowed) = 0;
  virtual HRESULT CheckDeviceFormat(UINT adapter, DWORD deviceType,
                                    DWORD adapterFormat, DWORD usage,
                                    DWORD resourceType,
                                    DWORD checkFormat) = 0;
  virtual HRESULT CreateDevice(UINT adapter, DWORD deviceType, HWND focusWindow,
                               DWORD behaviorFlags,
                               D3DPRESENT_PARAMETERS *pPresentationParameters,
                               IDirect3DDevice8 **ppDevice) = 0;
  virtual HRESULT GetAdapterIdentifier(UINT adapter, DWORD flags,
                                       D3DADAPTER_IDENTIFIER8 *identifier) = 0;
};

//	The device and resource type constants (the documented d3d8types
//	values).
#define D3DDEVTYPE_HAL 1
#define D3DDEVTYPE_REF 2
#define D3DRTYPE_SURFACE 1
#define D3DRTYPE_VOLUME 2
#define D3DRTYPE_TEXTURE 3
#define D3DRTYPE_VOLUMETEXTURE 4
#define D3DRTYPE_CUBETEXTURE 5
#define D3DRTYPE_VERTEXBUFFER 6
#define D3DRTYPE_INDEXBUFFER 7

//	The direct factory: returns none until the graphics backend
//	registers (the callers' null-device paths handle it).
inline IDirect3D8 *Direct3DCreate8(UINT sdkVersion) {
  (void)sdkVersion;
  return nullptr;
}

#endif // PW153_PLATFORM_D3D8_H
