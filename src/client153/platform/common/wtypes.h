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

struct STGMEDIUM {
  TYMED type;
  void *unionPointer;
  IUnknown *unknown;
};

#endif // PW153_PLATFORM_WTYPES_H
