#pragma once
// Compile-time boundary for the original hardware query. This is a partial
// port interface, not a native DirectDraw COM vtable or an implementation.
#include "unknwn.h"
#include <cstddef>	//	offsetof (the layout asserts below).

struct DDSCAPS { DWORD dwCaps; };
constexpr DWORD DDSCAPS_TEXTURE = 0x00001000;
constexpr DWORD DDSCAPS_LOCALVIDMEM = 0x10000000;
constexpr HRESULT DD_OK = 0;

//	The pixel format flags the DDS readers name (the documented
//	ddraw.h values).
#define DDPF_ALPHAPIXELS 0x00000001
#define DDPF_ALPHA 0x00000002
#define DDPF_FOURCC 0x00000004
#define DDPF_RGB 0x00000040
#define DDPF_YUV 0x00000200
#define DDPF_ZBUFFER 0x00000800

//	The color key record (the documented DDCOLORKEY layout).
struct DDCOLORKEY {
  DWORD dwColorSpaceLowValue;
  DWORD dwColorSpaceHighValue;
};

//	The surface capability record (the documented DDSCAPS2 layout).
struct DDSCAPS2 {
  DWORD dwCaps;
  DWORD dwCaps2;
  DWORD dwCaps3;
  union {
    DWORD dwCaps4;
    DWORD dwVolumeDepth;
  };
};

//	The pixel format record (the documented DDPIXELFORMAT layout with
//	its documented unions; 8 DWORDs = 32 bytes).
struct DDPIXELFORMAT {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwFourCC;
  union {
    DWORD dwRGBBitCount;
    DWORD dwYUVBitCount;
    DWORD dwZBufferBitDepth;
    DWORD dwAlphaBitDepth;
    DWORD dwLuminanceBitCount;
    DWORD dwBumpBitCount;
    DWORD dwPrivateBitCount;
  };
  union {
    DWORD dwRBitMask;
    DWORD dwYBitMask;
    DWORD dwStencilBitDepth;
    DWORD dwLuminanceBitMask;
    DWORD dwBumpDuBitMask;
    DWORD dwOperations;
  };
  union {
    DWORD dwGBitMask;
    DWORD dwUBitMask;
    DWORD dwZBitMask;
    DWORD dwBumpDvBitMask;
  };
  union {
    DWORD dwBBitMask;
    DWORD dwVBitMask;
    DWORD dwStencilBitMask;
    DWORD dwBumpLuminanceBitMask;
  };
  union {
    DWORD dwRGBAlphaBitMask;
    DWORD dwYUVAlphaBitMask;
    DWORD dwLuminanceAlphaBitMask;
    DWORD dwRGBZBitMask;
    DWORD dwYUVZBitMask;
  };
};

//	The surface description (the documented DDSURFACEDESC2 layout with
//	its documented unions; 124 bytes so the raw DDS file header at
//	file offset 4 casts onto it, as the original 32-bit build did).
struct DDSURFACEDESC2 {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwHeight;
  DWORD dwWidth;
  union {
    //	The documented Win32 LONG pitch is 32-bit; the C `long` is 64-bit
    //	on the native LP64 test host and would break the pinned 124-byte
    //	DDSURFACEDESC2 layout below (it is 4 bytes on the wasm32 target).
    int lPitch;
    DWORD dwLinearSize;
  };
  union {
    DWORD dwBackBufferCount;
    DWORD dwDepth;
  };
  union {
    DWORD dwMipMapCount;
    DWORD dwRefreshRate;
    DWORD dwSrcVBHandle;
  };
  DWORD dwAlphaBitDepth;
  DWORD dwReserved;
  //	The documented layout's lpSurface slot: the raw-DDS cast path only
  //	ever sees the stored 32-bit file value (never a live pointer), and a
  //	real pointer is 8 bytes on the native LP64 test host, which would
  //	break the pinned 124-byte layout below.
  DWORD dwSurfaceSlot;
  union {
    DDCOLORKEY ddckCKDestOverlay;
    DWORD dwEmptyFaceColor;
  };
  DDCOLORKEY ddckCKDestBlt;
  DDCOLORKEY ddckCKSrcOverlay;
  DDCOLORKEY ddckCKSrcBlt;
  union {
    DDPIXELFORMAT ddpfPixelFormat;
    DWORD dwFVF;
  };
  DDSCAPS2 ddsCaps;
  DWORD dwTextureStage;
};

//	The raw DDS file layout is magic (4 bytes) + this description; the
//	casts in A3DTexture.cpp require the documented offsets to hold.
static_assert(sizeof(DDSURFACEDESC2) == 124,
              "DDSURFACEDESC2 must match the documented 32-bit layout");
static_assert(sizeof(DDPIXELFORMAT) == 32,
              "DDPIXELFORMAT must match the documented 32-bit layout");
static_assert(offsetof(DDSURFACEDESC2, dwWidth) == 12 &&
                  offsetof(DDSURFACEDESC2, dwLinearSize) == 16 &&
                  offsetof(DDSURFACEDESC2, dwMipMapCount) == 24 &&
                  offsetof(DDSURFACEDESC2, ddpfPixelFormat) == 72,
              "DDSURFACEDESC2 field offsets must match the documented layout");

struct IDirectDraw : IUnknown {};

//	The DirectDraw 7 declaration surface the vendored DirectShow SDK
//	headers (videoctl.h) reference. The browser has no DirectDraw; these
//	exist so the SDK declarations parse — the runtime path fails through
//	DirectDrawCreate's explicit failure below.
struct IDirectDrawSurface : IUnknown {};
struct IDirectDrawClipper : IUnknown {};
struct IDirectDrawPalette : IUnknown {};

typedef IDirectDraw *LPDIRECTDRAW;
typedef IDirectDrawSurface *LPDIRECTDRAWSURFACE;
typedef IDirectDrawClipper *LPDIRECTDRAWCLIPPER;
typedef IDirectDrawPalette *LPDIRECTDRAWPALETTE;
typedef PALETTEENTRY *LPPALETTEENTRY;
typedef DDCOLORKEY *LPDDCOLORKEY;
typedef DDPIXELFORMAT *LPDDPIXELFORMAT;

//	The DirectDraw 3-era surface description (the documented layout: the
//	DDSURFACEDESC2 fields without the extended unions).
struct DDSURFACEDESC {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwHeight;
  DWORD dwWidth;
  union {
    int lPitch;
    DWORD dwLinearSize;
  };
  DWORD dwBackBufferCount;
  DWORD dwMipMapCount;
  DWORD dwAlphaBitDepth;
  DWORD dwReserved;
  DWORD lpSurface;
  DDCOLORKEY ddckCKDestOverlay;
  DDCOLORKEY ddckCKDestBlt;
  DDCOLORKEY ddckCKSrcOverlay;
  DDCOLORKEY ddckCKSrcBlt;
  DDPIXELFORMAT ddpfPixelFormat;
  DDSCAPS2 ddsCaps;
};
typedef DDSURFACEDESC *LPDDSURFACEDESC;

typedef HRESULT (*LPDDENUMMODESCALLBACK)(LPDDSURFACEDESC, void *);

//	The DirectDraw caps record pointer (the struct is defined above).
typedef DDSCAPS *LPDDSCAPS;

//	The ROP capability-array length (the documented ddraw.h constant;
//	the DDCAPS record below carries the arrays).
#define DD_ROP_SPACE 8

//	The DirectDraw driver-caps record (the documented DDCAPS DirectX 7
//	layout; videoctl.h's aggregated DirectDraw passes the pointers only —
//	the browser has no DirectDraw, so nothing dereferences the fields).
struct DDCAPS {
  DWORD dwSize;
  DWORD dwCaps;
  DWORD dwCaps2;
  DWORD dwCKeyCaps;
  DWORD dwFXCaps;
  DWORD dwFXAlphaCaps;
  DWORD dwPalCaps;
  DWORD dwSVCaps;
  DWORD dwAlphaBltConstBitDepths;
  DWORD dwAlphaBltPixelBitDepths;
  DWORD dwAlphaBltSurfaceBitDepths;
  DWORD dwAlphaOverlayConstBitDepths;
  DWORD dwAlphaOverlayPixelBitDepths;
  DWORD dwAlphaOverlaySurfaceBitDepths;
  DWORD dwZBufferBitDepths;
  DWORD dwVidMemTotal;
  DWORD dwVidMemFree;
  DWORD dwMaxVisibleOverlays;
  DWORD dwCurrVisibleOverlays;
  DWORD dwNumFourCCCodes;
  DWORD dwAlignBoundarySrc;
  DWORD dwAlignSizeSrc;
  DWORD dwAlignBoundaryDest;
  DWORD dwAlignSizeDest;
  DWORD dwAlignStrideAlign;
  DWORD dwRops[DD_ROP_SPACE];
  DDSCAPS ddsOldCaps;
  DWORD dwMinOverlayStretch;
  DWORD dwMaxOverlayStretch;
  DWORD dwMinLiveVideoStretch;
  DWORD dwMaxLiveVideoStretch;
  DWORD dwMinHwCodecStretch;
  DWORD dwMaxHwCodecStretch;
  DWORD dwReserved1;
  DWORD dwReserved2;
  DWORD dwReserved3;
  DWORD dwSVBCaps;
  DWORD dwSVBCKeyCaps;
  DWORD dwSVBFXCaps;
  DWORD dwSVBRops[DD_ROP_SPACE];
  DWORD dwVSBCaps;
  DWORD dwVSBCKeyCaps;
  DWORD dwVSBFXCaps;
  DWORD dwVSBRops[DD_ROP_SPACE];
  DWORD dwSSBCaps;
  DWORD dwSSBCKeyCaps;
  DWORD dwSSBFXCaps;
  DWORD dwSSBRops[DD_ROP_SPACE];
  DWORD dwMaxVideoPorts;
  DWORD dwCurrVideoPorts;
  DWORD dwSVBCaps2;
  DWORD dwNLVBCaps;
  DWORD dwNLVBCaps2;
  DWORD dwNLVBCKeyCaps;
  DWORD dwNLVBFXCaps;
  DWORD dwNLVBRops[DD_ROP_SPACE];
  DDSCAPS2 ddsCaps;
};
typedef DDCAPS *LPDDCAPS;

//	The blit-effects record (the documented DDBLTFX layout).
struct DDBLTFX {
  DWORD dwSize;
  DWORD dwDDFX;
  DWORD dwROP;
  DWORD dwDDROP;
  DWORD dwRotationAngle;
  DWORD dwZBufferOpCode;
  DWORD dwZBufferLow;
  DWORD dwZBufferHigh;
  DWORD dwZBufferBaseDest;
  DWORD dwZDestConstBitDepth;
  IDirectDrawSurface *lpDDSZBufferDest;
  DWORD dwZSrcConstBitDepth;
  IDirectDrawSurface *lpDDSZBufferSrc;
  DWORD dwAlphaEdgeBlendBitDepth;
  DWORD dwAlphaEdgeBlend;
  DWORD dwReserved;
  DWORD dwAlphaDestConstBitDepth;
  IDirectDrawSurface *lpDDSAlphaDest;
  DWORD dwAlphaSrcConstBitDepth;
  IDirectDrawSurface *lpDDSAlphaSrc;
  IDirectDrawSurface *lpDDSPattern;
  DDCOLORKEY ddckDestColorkey;
  DDCOLORKEY ddckSrcColorkey;
};
typedef DDBLTFX *LPDDBLTFX;

//	The overlay-effects record (the documented DDOVERLAYFX layout; the
//	constant/surface slots are the documented unions).
struct DDOVERLAYFX {
  DWORD dwSize;
  DWORD dwAlphaEdgeBlendBitDepth;
  DWORD dwAlphaEdgeBlend;
  DWORD dwReserved;
  DWORD dwAlphaDestConstBitDepth;
  union {
    DWORD dwAlphaDestConst;
    IDirectDrawSurface *lpDDSAlphaDest;
  };
  DWORD dwAlphaSrcConstBitDepth;
  union {
    DWORD dwAlphaSrcConst;
    IDirectDrawSurface *lpDDSAlphaSrc;
  };
  DDCOLORKEY dckDestColorkey;
  DDCOLORKEY dckSrcColorkey;
  DWORD dwDDFX;
  DWORD dwFlags;
};
typedef DDOVERLAYFX *LPDDOVERLAYFX;

//	The blit-batch record (the documented DDBLTBATCH layout).
struct DDBLTBATCH {
  RECT *lprDest;
  IDirectDrawSurface *lpDDSSrcSurface;
  RECT *lprSrc;
  DWORD dwFlags;
  DDBLTFX *lpDDBltFx;
};
typedef DDBLTBATCH *LPDDBLTBATCH;

typedef HRESULT (*LPDDENUMSURFACESCALLBACK)(LPDIRECTDRAWSURFACE,
                                            LPDDSURFACEDESC, void *);
struct IDirectDraw2 : IUnknown {
  virtual HRESULT GetAvailableVidMem(DDSCAPS *caps, DWORD *total, DWORD *free) = 0;
};
extern const void *IID_IDirectDraw2;
HRESULT DirectDrawCreate(const void *guid, IDirectDraw **draw, IUnknown *outer);
