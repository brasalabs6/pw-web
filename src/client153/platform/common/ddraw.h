#pragma once
// Compile-time boundary for the original hardware query. This is a partial
// port interface, not a native DirectDraw COM vtable or an implementation.
#include "unknwn.h"

struct DDSCAPS { DWORD dwCaps; };
constexpr DWORD DDSCAPS_TEXTURE = 0x00001000;
constexpr DWORD DDSCAPS_LOCALVIDMEM = 0x10000000;
constexpr HRESULT DD_OK = 0;

struct IDirectDraw : IUnknown {};
struct IDirectDraw2 : IUnknown {
  virtual HRESULT GetAvailableVidMem(DDSCAPS *caps, DWORD *total, DWORD *free) = 0;
};
extern const void *IID_IDirectDraw2;
HRESULT DirectDrawCreate(const void *guid, IDirectDraw **draw, IUnknown *outer);
