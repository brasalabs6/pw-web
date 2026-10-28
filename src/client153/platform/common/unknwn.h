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
//	STDAPI/STDAPI_: the free-function forms (the seam's __stdcall is
//	empty; the linkage is the caller's, like STDMETHODIMP above).
#define STDAPI HRESULT
#define STDAPI_(type) type
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
//	reference; the byte-array accessors yield const void*.
inline bool IsEqualIID(const void *riid, const GUID &iid) {
  return riid && IsEqualGUID(*static_cast<const GUID *>(riid), iid);
}
inline bool IsEqualIID(const void *a, const void *b) {
  return a && b && std::memcmp(a, b, 16) == 0;
}
inline bool IsEqualIID(const GUID &a, const GUID &b) {
  return IsEqualGUID(a, b);
}
inline bool operator==(const GUID &a, const GUID &b) {
  return IsEqualGUID(a, b);
}
inline bool operator!=(const GUID &a, const GUID &b) {
  return !IsEqualGUID(a, b);
}

//	The null GUID (the documented all-zero id).
inline const GUID GUID_NULL = {};

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

//	The COM reference types (the seam's REFIID/REFCLSID are the opaque
//	pointer forms; the interface ids are byte layouts behind them).
typedef const void *REFIID;
typedef const void *REFCLSID;
typedef const GUID &REFGUID;

//	The MIDL interface declaration macro (the uuid-carrying struct
//	declaration the generated COM headers use).
#define MIDL_INTERFACE(uuidText) struct PW153_COM_UUID(uuidText)
#define DECLSPEC_UUID(uuidText) PW153_COM_UUID(uuidText)

//	The COM identifier types (the documented forms: CLSID/IID are GUIDs).
typedef GUID CLSID;
typedef GUID IID;

//	IUnknown's interface id (the documented bytes).
inline const void *IID_IUnknown() {
  static const unsigned char id[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0xc0, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x46};
  return id;
}

#endif // PW153_PLATFORM_UNKNWN_H
