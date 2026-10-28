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
    LONG lPitch;
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
  void *lpSurface;
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
struct IDirectDraw2 : IUnknown {
  virtual HRESULT GetAvailableVidMem(DDSCAPS *caps, DWORD *total, DWORD *free) = 0;
};
extern const void *IID_IDirectDraw2;
HRESULT DirectDrawCreate(const void *guid, IDirectDraw **draw, IUnknown *outer);
