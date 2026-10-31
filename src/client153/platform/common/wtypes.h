/*
 * FILE: wtypes.h (platform seam)
 *
 * DESCRIPTION: The OLE base type surface. The client's wtypes includers use
 * none of the COM machinery (verified: no VARIANT/BSTR/IDispatch usage in
 * the including translation units); the header exists because the original
 * include chain reached it. The types carry the documented Win32 layouts;
 * the COM object model (Home automation) extends this seam in its own cycle.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_WTYPES_H
#define PW153_PLATFORM_WTYPES_H

#include "Windows.h"

struct IUnknown;

typedef wchar_t OLECHAR;
typedef OLECHAR *BSTR;
typedef const OLECHAR *LPCOLESTR;
typedef OLECHAR *LPOLESTR;
typedef unsigned short VARIANT_BOOL;

#define VARIANT_TRUE static_cast<VARIANT_BOOL>(0xFFFF)
#define VARIANT_FALSE static_cast<VARIANT_BOOL>(0)
typedef double DATE;
typedef WORD VARTYPE;
typedef DWORD TYMED;
typedef std::int64_t LONGLONG;
typedef std::int64_t LARGE_INTEGER_TYPE_UNUSED;
typedef std::uint64_t ULONGLONG_;

typedef struct tagDEC {
  WORD wReserved;
  BYTE scale;
  BYTE sign;
  DWORD Hi32;
  ULONGLONG_ Lo64;
} DECIMAL;

typedef struct tagVARIANT VARIANT;

struct tagVARIANT {
  VARTYPE vt;
  WORD wReserved1;
  WORD wReserved2;
  WORD wReserved3;
  union {
    LONGLONG llVal;
    LONG lVal;
    int intVal;
    BYTE bVal;
    short iVal;
    VARIANT_BOOL boolVal;
    float fltVal;
    double dblVal;
    DATE date;
    BSTR bstrVal;
  };
};

#define VT_EMPTY 0

//	VariantClear: the real reset (frees a BSTR value, empties the rest);
//	defined in atlcom_registry.cpp over the real SysFreeString.
HRESULT VariantClear(VARIANT *variant);

//	The OLE automation string surface (the real length-prefixed BSTR
//	allocation).
inline BSTR SysAllocString(const wchar_t *text) {
  if (!text)
    return nullptr;
  std::size_t length = 0;
  while (text[length])
    ++length;
  auto *header = static_cast<std::uint32_t *>(std::malloc(sizeof(std::uint32_t) + (length + 1) * sizeof(wchar_t)));
  if (!header)
    return nullptr;
  *header = static_cast<std::uint32_t>(length);
  auto *data = reinterpret_cast<wchar_t *>(header + 1);
  std::memcpy(data, text, (length + 1) * sizeof(wchar_t));
  return data;
}

inline void SysFreeString(BSTR text) {
  if (text)
    std::free(reinterpret_cast<std::uint32_t *>(text) - 1);
}

inline std::uint32_t SysStringLen(BSTR text) {
  if (!text)
    return 0;
  return *(reinterpret_cast<std::uint32_t *>(text) - 1);
}

#define VT_NULL 1
#define VT_I2 2
#define VT_I4 3
#define VT_R4 4
#define VT_R8 5
#define VT_BSTR 8
#define VT_BOOL 11
#define VT_I8 20

struct FORMATETC {
  unsigned short clipFormat;
  void *ptd;
  DWORD aspect;
  LONG index;
  TYMED medium;
};

//	The exception record the automation Invoke contract fills (the
//	documented EXCEPINFO layout).
typedef struct tagEXCEPINFO {
  WORD wCode;
  WORD wReserved1;
  WORD wReserved2;
  WORD wReserved3;
  BSTR bstrSource;
  BSTR bstrDescription;
  BSTR bstrHelpFile;
  DWORD dwHelpContext;
  void *pvReserved;
  void (*pfnDeferredFillIn)(tagEXCEPINFO *);
  DWORD scode;
} EXCEPINFO;

struct STGMEDIUM {
  TYMED type;
  void *unionPointer;
  IUnknown *unknown;
};

#endif // PW153_PLATFORM_WTYPES_H
