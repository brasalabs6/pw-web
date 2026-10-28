/*
 * FILE: unknwn.h (platform seam)
 *
 * DESCRIPTION: The COM base interface. The SDK headers include it for
 * IUnknown; the port's interface seams already derive the same three-method
 * surface, so this declares the canonical base.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_UNKNWN_H
#define PW153_PLATFORM_UNKNWN_H

#include "Windows.h"

#define __RPC_FAR
#define STDMETHODCALLTYPE

#define STDMETHOD(method) virtual HRESULT method
#define STDMETHOD_(type, method) virtual type method
#define STDMETHODIMP HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type) type STDMETHODCALLTYPE
#define PURE = 0
#define __stdcall
#define STDMETHODCALLTYPE

//	The COM identifier layout (the documented GUID byte layout; the
//	__uuidof operator over __declspec(uuid) types yields references
//	to it).
typedef struct _GUID {
  unsigned long Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;

inline bool IsEqualGUID(const GUID &a, const GUID &b) {
  return a.Data1 == b.Data1 && a.Data2 == b.Data2 && a.Data3 == b.Data3 &&
         std::memcmp(a.Data4, b.Data4, sizeof(a.Data4)) == 0;
}

//	The interface comparison: riid arrives as the seam's REFIID (const
//	void* to the GUID bytes); the __uuidof operand yields a GUID
//	reference.
inline bool IsEqualIID(const void *riid, const GUID &iid) {
  return riid && IsEqualGUID(*static_cast<const GUID *>(riid), iid);
}

//	The COM uuid attribute: the __uuidof operator needs it; it exists
//	only under MSVC or clang's -fms-extensions (the game probe builds);
//	plain GCC and the plain-clang test builds never call __uuidof.
//	(The clang check is nested so GCC's preprocessor never lexes
//	__is_identifier.)
#if defined(_MSC_VER)
#define PW153_COM_UUID(uuidText) __declspec(uuid(uuidText))
#else
#if defined(__clang__)
#if !__is_identifier(__declspec)
#define PW153_COM_UUID(uuidText) __declspec(uuid(uuidText))
#else
#define PW153_COM_UUID(uuidText)
#endif
#else
#define PW153_COM_UUID(uuidText)
#endif
#endif

struct PW153_COM_UUID("{00000000-0000-0000-C000-000000000046}") IUnknown {
  virtual ~IUnknown() = default;
  virtual HRESULT QueryInterface(const void *riid, void **ppvObject) = 0;
  virtual ULONG AddRef() = 0;
  virtual ULONG Release() = 0;
};

#endif // PW153_PLATFORM_UNKNWN_H
