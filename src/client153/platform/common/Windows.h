/*
 * FILE: Windows.h (platform seam)
 *
 * DESCRIPTION: The Win32 type and primitive boundary the original client
 * compiles against, mapped to real portable implementations. This is the
 * owning boundary for the Win32 dependency: types keep the Windows widths,
 * CRITICAL_SECTION is the existing re-entrant std::recursive_mutex seam,
 * Interlocked operations are std::atomics, time functions use the platform
 * clock and MessageBox reports through the diagnostic sink like the original
 * debug window did. Windowing, GDI, D3D, sockets and dynamic loading are NOT
 * part of this seam: sources using them keep failing on those specific APIs
 * until their own browser adapters land.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_WINDOWS_H
#define PW153_PLATFORM_WINDOWS_H

#include "ABaseDef.h"
#include "criticalsection.h"
#include "clock.h"
#include "widestr.h"	//	the 16-bit wide CRT (the system wchar_t CRT is wrong for the port width).
#include "inifile.h"	//	the Win32 private-profile (INI) reader.
#include "utf16.h"

#include <algorithm>
#include <unistd.h>	//	getcwd/chdir/mkdir.
#include <sys/stat.h>	//	stat/mkdir.
#include <sys/statvfs.h>	//	statvfs (GetDiskFreeSpaceA).
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>	//	exit (ExitProcess).
#include <cstring>
#include <strings.h>
#include <string>
#include <string_view>
#include <thread>

inline int stricmp(const char *lhs, const char *rhs) { return ::strcasecmp(lhs, rhs); }
inline int strcmpi(const char *lhs, const char *rhs) { return stricmp(lhs, rhs); }
inline int _stricmp(const char *lhs, const char *rhs) { return stricmp(lhs, rhs); }
inline int strnicmp(const char *lhs, const char *rhs, std::size_t count) { return ::strncasecmp(lhs, rhs, count); }
inline int _strcmpi(const char *lhs, const char *rhs) { return stricmp(lhs, rhs); }

//	Last-error state: a real per-thread slot the callers can read back.
inline DWORD *ThreadLastErrorSlot() {
  thread_local DWORD lastError = 0;
  return &lastError;
}
inline DWORD GetLastError() { return *ThreadLastErrorSlot(); }
inline void SetLastError(DWORD error) { *ThreadLastErrorSlot() = error; }

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

//	The MSVC CRT name of the same path-length constant (stdlib.h).
#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

#ifndef MAXBYTE
#define MAXBYTE 255
#endif
#ifndef MAXWORD
#define MAXWORD 65535
#endif

//	Win32 base types with the Windows widths (the ported ABaseDef already
//	owns DWORD/WORD/BYTE/BOOL; guard everything to coexist).
#ifndef PW153_WIN32_TYPES_DEFINED
#define PW153_WIN32_TYPES_DEFINED
typedef unsigned int UINT;
typedef int INT;
typedef unsigned long ULONG;
typedef unsigned short USHORT;
typedef std::uint16_t ATOM;
typedef unsigned char UCHAR;
//	The Win32 BOOLEAN (the documented 1-byte boolean of winnt.h; the HID
//	callback typedefs in UsbHidKey.h use it).
typedef unsigned char BOOLEAN;
typedef std::uint64_t UINT64;
typedef std::int64_t INT64;
typedef wchar_t WCHAR;
typedef wchar_t TCHAR;
typedef void *PVOID;
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef wchar_t *LPWSTR;
typedef const wchar_t *LPCWSTR;
struct _EXCEPTION_POINTERS;
typedef _EXCEPTION_POINTERS *LPEXCEPTION_POINTERS;
typedef void *HANDLE;
//	The pointer-to-handle form (the token-query seam functions take it;
//	declared with the base types so every later user sees it — the
//	duplicate typedef further down stays legal).
typedef HANDLE *PHANDLE;
typedef BYTE *LPBYTE;
typedef BYTE *PBYTE;
typedef DWORD *LPDWORD;
typedef DWORD *PDWORD;
typedef LONG *PLONG;
typedef const char *PSZ;

typedef const char *PCSZ;
//	The Win32 PSTR family (the setupapi callback typedefs use PCSTR).
typedef char *PSTR;
typedef const char *PCSTR;
typedef void *HFONT;
typedef void *HBITMAP;
typedef void *HBRUSH;
typedef void *HPALETTE;
typedef void *HPEN;
typedef void *HRGN;
typedef void *HGDIOBJ;
typedef void *HCURSOR;
typedef void *HICON;
typedef void *HMENU;
typedef void *HACCEL;
typedef void *HKL;
typedef void *HWND;
typedef void *HDC;
typedef void *HINSTANCE;
typedef void *HMODULE;
typedef std::int32_t HRESULT;
#ifndef E_FAIL
#define E_FAIL 0x80004005
#ifndef E_NOTIMPL
#define E_NOTIMPL 0x80004001
#endif
#ifndef E_NOINTERFACE
#define E_NOINTERFACE 0x80004002
#endif
#ifndef E_POINTER
#define E_POINTER 0x80004003
#endif
#ifndef E_ABORT
#define E_ABORT 0x80004004
#endif
#ifndef E_INVALIDARG
#define E_INVALIDARG 0x80070057
#endif
#ifndef E_OUTOFMEMORY
#define E_OUTOFMEMORY 0x8007000E
#endif
#ifndef S_OK
#define S_OK 0
#endif
#ifndef S_FALSE
#define S_FALSE 1
#endif
#ifndef ERROR_CALL_NOT_IMPLEMENTED
#define ERROR_CALL_NOT_IMPLEMENTED 120
#endif
#endif
typedef struct _CONTEXT *PCONTEXT;
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
typedef std::size_t SIZE_T;
typedef std::uintptr_t UINT_PTR;
typedef std::intptr_t INT_PTR;
typedef std::intptr_t LONG_PTR;
typedef std::uintptr_t ULONG_PTR;
typedef ULONG *PULONG;
typedef std::uintptr_t DWORD_PTR;
typedef std::uintptr_t WPARAM;
typedef std::intptr_t LPARAM;
typedef std::uint32_t UINT32;
typedef std::int32_t INT32;
#endif

#ifndef LOWORD
#define LOWORD(value) ((WORD)(((std::uintptr_t)(value)) & 0xffff))
#define HIWORD(value) ((WORD)((((std::uintptr_t)(value)) >> 16) & 0xffff))
#define LOBYTE(value) ((BYTE)(((std::uintptr_t)(value)) & 0xff))
#define HIBYTE(value) ((BYTE)((((std::uintptr_t)(value)) >> 8) & 0xff))
#define MAKEWORD(low, high) ((WORD)((((BYTE)(low)) & 0xff) | (((WORD)((BYTE)(high))) << 8)))
#define MAKELONG(low, high) ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))
#endif

struct POINT {
  LONG x;
  LONG y;
};

struct RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
};

typedef RECT *LPRECT;
typedef RECT *PRECT;
typedef const RECT *LPCRECT;
//	The rectangle setter (the documented user32 contract: assigns the
//	fields and returns TRUE).
inline BOOL SetRect(LPRECT rect, int left, int top, int right, int bottom) {
  rect->left = left;
  rect->top = top;
  rect->right = right;
  rect->bottom = bottom;
  return TRUE;
}
inline BOOL IntersectRect(LPRECT out, LPCRECT a, LPCRECT b) {
  const RECT overlap{std::max(a->left, b->left), std::max(a->top, b->top),
                     std::min(a->right, b->right), std::min(a->bottom, b->bottom)};
  if (overlap.left >= overlap.right || overlap.top >= overlap.bottom) {
    *out = RECT{0, 0, 0, 0};
    return FALSE;
  }
  *out = overlap;
  return TRUE;
}
typedef DWORD COLORREF;
typedef BOOL (*WNDENUMPROC)(HWND, LPARAM);
struct GUITHREADINFO {
  DWORD cbSize;
  DWORD flags;
  HWND hwndActive;
  HWND hwndFocus;
  HWND hwndCapture;
  HWND hwndMenuOwner;
  HWND hwndMoveSize;
  HWND hwndCaret;
  RECT rcCaret;
};

struct SIZE {
  LONG cx;
  LONG cy;
};

struct SYSTEMTIME {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
};

struct FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
};
//	The system information and version records (the documented layouts).
//	The documented ABI layout (the anonymous struct in the union - see
//	the 64-bit unions above; the pragma waives the pedantic diagnostic
//	for the deliberate layout).
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
struct SYSTEM_INFO {
  union {
    DWORD dwOemId;
    struct {
      WORD wProcessorArchitecture;
      WORD wReserved;
    };
  };
  DWORD dwPageSize;
  void *lpMinimumApplicationAddress;
  void *lpMaximumApplicationAddress;
  DWORD dwActiveProcessorMask;
  DWORD dwNumberOfProcessors;
  DWORD dwProcessorType;
  DWORD dwAllocationGranularity;
  WORD wProcessorLevel;
  WORD wProcessorRevision;
};
#if defined(__clang__)
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif

//	The OS version records: the narrow A variant (ImeUi reads
//	szCSDVersion with lstrlenA) and the wide default the UNICODE build
//	uses (the documented OSVERSIONINFOW layout).
struct OSVERSIONINFOA {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  char szCSDVersion[128];
};
typedef OSVERSIONINFOA *LPOSVERSIONINFOA;

struct OSVERSIONINFO {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  wchar_t szCSDVersion[128];
};

typedef OSVERSIONINFO *LPOSVERSIONINFO;

//	The extended OS version record (the documented OSVERSIONINFOEXW
//	layout; the UNICODE build reads szCSDVersion as wide).
struct OSVERSIONINFOEX {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  wchar_t szCSDVersion[128];
  WORD wServicePackMajor;
  WORD wServicePackMinor;
  WORD wSuiteMask;
  BYTE wProductType;
  BYTE wReserved;
};
typedef OSVERSIONINFOEX *LPOSVERSIONINFOEX;

struct MEMORYSTATUS {
  DWORD dwLength;
  DWORD dwMemoryLoad;
  std::size_t dwTotalPhys;
  std::size_t dwAvailPhys;
  std::size_t dwTotalPageFile;
  std::size_t dwAvailPageFile;
  std::size_t dwTotalVirtual;
  std::size_t dwAvailVirtual;
  std::size_t dwAvailExtendedVirtual;
};

inline void GlobalMemoryStatus(MEMORYSTATUS *status) {
  if (status) {
    status->dwLength = sizeof(MEMORYSTATUS);
    //	The browser runtime's memory is dynamic; report the WASM module's
    //	current limits honestly (the adapter updates them).
    status->dwMemoryLoad = 0;
    status->dwTotalPhys = 0;
    status->dwAvailPhys = 0;
    status->dwTotalPageFile = 0;
    status->dwAvailPageFile = 0;
    status->dwTotalVirtual = 0;
    status->dwAvailVirtual = 0;
  }
}

#define VER_PLATFORM_WIN32_WINDOWS 1
#define VER_PLATFORM_WIN32_NT 2

inline void GetSystemInfo(SYSTEM_INFO *info) {
  if (info) {
    info->dwNumberOfProcessors = 1;
    info->dwPageSize = 4096;
    info->dwProcessorType = 586;
    info->dwAllocationGranularity = 65536;
  }
}

inline BOOL GetVersionExA(OSVERSIONINFOA *info) {
  if (!info || info->dwOSVersionInfoSize < sizeof(OSVERSIONINFOA))
    return FALSE;
  //	The browser target reports the port baseline: a 32-bit Windows-like
  //	environment the original code expects.
  info->dwMajorVersion = 5;
  info->dwMinorVersion = 1;
  info->dwBuildNumber = 2600;
  info->dwPlatformId = 2;
  info->szCSDVersion[0] = '\0';
  return TRUE;
}

inline BOOL GetVersionExW(OSVERSIONINFO *info) {
  if (!info || info->dwOSVersionInfoSize < sizeof(OSVERSIONINFO))
    return FALSE;
  info->dwMajorVersion = 5;
  info->dwMinorVersion = 1;
  info->dwBuildNumber = 2600;
  info->dwPlatformId = 2;
  info->szCSDVersion[0] = L'\0';
  return TRUE;
}

inline BOOL GetVersionEx(OSVERSIONINFO *info) {
  return GetVersionExW(info);
}

typedef FILETIME *LPFILETIME;
typedef const FILETIME *LPCFILETIME;

struct MSG {
  HWND hwnd;
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
  DWORD time;
  POINT pt;
};

typedef MSG *LPMSG;

struct NOTIFYICONDATA {
  DWORD cbSize;
  HWND hWnd;
  UINT uID;
  UINT uFlags;
  UINT uCallbackMessage;
  HICON hIcon;
  wchar_t szTip[128];
  DWORD dwState;
  DWORD dwStateMask;
  wchar_t szInfo[256];
  UINT uTimeout;
  wchar_t szInfoTitle[64];
  DWORD dwInfoFlags;
};

//	The tray icon flags and messages (the documented shellapi values).
#define NIM_ADD 0x00
#define NIM_MODIFY 0x01
#define NIM_DELETE 0x02
#define NIF_MESSAGE 0x01
#define NIF_ICON 0x02
#define NIF_TIP 0x04

//	The tray icon update: no system tray exists on the browser target;
//	the call fails (the callers' error paths handle it).
inline BOOL Shell_NotifyIcon(DWORD message, NOTIFYICONDATA *data) {
  (void)message;
  (void)data;
  return FALSE;
}

//	The display settings change: the browser host owns the display
//	mode; a reset of an unchanged display is vacuously successful (the
//	documented DISP_CHANGE result).
#define CDS_RESET 0x40000000
#define CDS_TEST 0x00000002
#define DISP_CHANGE_SUCCESSFUL 0
#define DISP_CHANGE_FAILED 1
inline LONG ChangeDisplaySettings(void *deviceMode, DWORD flags) {
  (void)deviceMode;
  (void)flags;
  return DISP_CHANGE_SUCCESSFUL;
}

//	The display-mode record (the documented DEVMODE core; the client
//	reads the graphics-mode tail only - the printer ICM fields stay
//	out until a consumer names them).
typedef struct _devicemodeW {
  wchar_t dmDeviceName[32];
  WORD dmSpecVersion;
  WORD dmDriverVersion;
  WORD dmSize;
  WORD dmDriverExtra;
  DWORD dmFields;
  short dmOrientation;
  short dmPaperSize;
  short dmPaperLength;
  short dmPaperWidth;
  short dmScale;
  short dmCopies;
  short dmDefaultSource;
  short dmPrintQuality;
  short dmColor;
  short dmDuplex;
  short dmYResolution;
  short dmTTOption;
  short dmCollate;
  wchar_t dmFormName[32];
  WORD dmLogPixels;
  DWORD dmBitsPerPel;
  DWORD dmPelsWidth;
  DWORD dmPelsHeight;
  DWORD dmDisplayFlags;
  DWORD dmDisplayFrequency;
} DEVMODE, *LPDEVMODE;

//	The display-mode enumeration: the browser host owns the display
//	and publishes no mode list; the call reports none (the frequency
//	probe keeps its zero default, the documented fallback).
inline BOOL EnumDisplaySettings(const wchar_t *deviceName, DWORD modeIndex,
                                DEVMODE *deviceMode) {
  (void)deviceName;
  (void)modeIndex;
  (void)deviceMode;
  return FALSE;
}

struct PALETTEENTRY {
  BYTE peRed;
  BYTE peGreen;
  BYTE peBlue;
  BYTE peFlags;
};

//	Palette-entry flags: the documented wingdi values.
#define PC_RESERVED 0x01
#define PC_EXPLICIT 0x02
#define PC_NOCOLLAPSE 0x04

//	The logical font (the documented wingdi layouts; the browser IME
//	adapter owns the real font selection).
typedef struct tagLOGFONTA {
  LONG lfHeight;
  LONG lfWidth;
  LONG lfEscapement;
  LONG lfOrientation;
  LONG lfWeight;
  BYTE lfItalic;
  BYTE lfUnderline;
  BYTE lfStrikeOut;
  BYTE lfCharSet;
  BYTE lfOutPrecision;
  BYTE lfClipPrecision;
  BYTE lfQuality;
  BYTE lfPitchAndFamily;
  char lfFaceName[32];
} LOGFONTA, *LPLOGFONTA;

typedef struct tagLOGFONTW {
  LONG lfHeight;
  LONG lfWidth;
  LONG lfEscapement;
  LONG lfOrientation;
  LONG lfWeight;
  BYTE lfItalic;
  BYTE lfUnderline;
  BYTE lfStrikeOut;
  BYTE lfCharSet;
  BYTE lfOutPrecision;
  BYTE lfClipPrecision;
  BYTE lfQuality;
  BYTE lfPitchAndFamily;
  wchar_t lfFaceName[32];
} LOGFONTW, *LPLOGFONTW;

//	The text metric and font-enumeration records (the documented wingdi
//	layouts; the browser has no GDI font enumeration - the FWFontEnum
//	callback is never invoked, the Init path fails on the NULL DC).
#define LF_FACESIZE 32
#define LF_FULLFACESIZE 64

typedef struct tagTEXTMETRICA {
  LONG tmHeight;
  LONG tmAscent;
  LONG tmDescent;
  LONG tmInternalLeading;
  LONG tmExternalLeading;
  LONG tmAveCharWidth;
  LONG tmMaxCharWidth;
  LONG tmWeight;
  LONG tmOverhang;
  LONG tmDigitizedAspectX;
  LONG tmDigitizedAspectY;
  BYTE tmFirstChar;
  BYTE tmLastChar;
  BYTE tmDefaultChar;
  BYTE tmBreakChar;
  BYTE tmItalic;
  BYTE tmUnderlined;
  BYTE tmStruckOut;
  BYTE tmPitchAndFamily;
  BYTE tmCharSet;
} TEXTMETRICA, *PTEXTMETRICA;

typedef struct tagTEXTMETRICW {
  LONG tmHeight;
  LONG tmAscent;
  LONG tmDescent;
  LONG tmInternalLeading;
  LONG tmExternalLeading;
  LONG tmAveCharWidth;
  LONG tmMaxCharWidth;
  LONG tmWeight;
  LONG tmOverhang;
  LONG tmDigitizedAspectX;
  LONG tmDigitizedAspectY;
  WCHAR tmFirstChar;
  WCHAR tmLastChar;
  WCHAR tmDefaultChar;
  WCHAR tmBreakChar;
  BYTE tmItalic;
  BYTE tmUnderlined;
  BYTE tmStruckOut;
  BYTE tmPitchAndFamily;
  BYTE tmCharSet;
} TEXTMETRICW, *PTEXTMETRICW;

typedef struct tagNEWTEXTMETRICA {
  LONG ntmHeight;
  LONG ntmAscent;
  LONG ntmDescent;
  LONG ntmCellHeight;
  LONG ntmAvgWidth;
  TEXTMETRICA tmA;
} NEWTEXTMETRICA, *PNEWTEXTMETRICA;

typedef struct tagNEWTEXTMETRICW {
  LONG ntmHeight;
  LONG ntmAscent;
  LONG ntmDescent;
  LONG ntmCellHeight;
  LONG ntmAvgWidth;
  TEXTMETRICW tmW;
} NEWTEXTMETRICW, *PNEWTEXTMETRICW;

typedef struct tagNEWTEXTMETRICEXA {
  NEWTEXTMETRICA ntmTm;
  DWORD ntmFontSig;
  DWORD ntmSysSig;
} NEWTEXTMETRICEXA, *PNEWTEXTMETRICEXA;

typedef struct tagNEWTEXTMETRICEXW {
  NEWTEXTMETRICW ntmTm;
  DWORD ntmFontSig;
  DWORD ntmSysSig;
} NEWTEXTMETRICEXW, *PNEWTEXTMETRICEXW;

typedef struct tagENUMLOGFONTEXA {
  LOGFONTA elfLogFont;
  BYTE elfFullName[LF_FULLFACESIZE];
  BYTE elfStyle[LF_FACESIZE];
  BYTE elfScript[LF_FACESIZE];
} ENUMLOGFONTEXA, *LPENUMLOGFONTEXA;

typedef struct tagENUMLOGFONTEXW {
  LOGFONTW elfLogFont;
  WCHAR elfFullName[LF_FULLFACESIZE];
  WCHAR elfStyle[LF_FACESIZE];
  WCHAR elfScript[LF_FACESIZE];
} ENUMLOGFONTEXW, *LPENUMLOGFONTEXW;

#ifdef UNICODE
typedef ENUMLOGFONTEXW ENUMLOGFONTEX;
typedef LPENUMLOGFONTEXW LPENUMLOGFONTEX;
typedef NEWTEXTMETRICEXW NEWTEXTMETRICEX;
typedef PNEWTEXTMETRICEXW PNEWTEXTMETRICEX;
#else
typedef ENUMLOGFONTEXA ENUMLOGFONTEX;
typedef LPENUMLOGFONTEXA LPENUMLOGFONTEX;
typedef NEWTEXTMETRICEXA NEWTEXTMETRICEX;
typedef PNEWTEXTMETRICEXA PNEWTEXTMETRICEX;
#endif

//	The high-resolution performance counter: the real monotonic clock
//	(the documented Win32 contract; the functions live below with the
//	LARGE_INTEGER layout they reference).

//	The color composition macro (the documented wingdi form; the seam's
//	COLORREF layout is 0x00BBGGRR - the extraction macros live below).
#ifndef RGB
#define RGB(r, g, b) \
  ((COLORREF)((BYTE)(r) | ((BYTE)(g) << 8) | ((BYTE)(b) << 16)))
#endif

//	The IME composition/candidate context handle (the documented imm
//	handle; the browser IME belongs to the input adapter).
typedef HANDLE HIMCC;

//	The legacy pointer near/far qualifiers (empty on 32-bit Windows -
//	the documented values).
#ifndef NEAR
#define NEAR
#endif
#ifndef FAR
#define FAR
#endif

//	The MSVC integer pointer typedefs.
typedef int *PINT, *LPINT;
typedef unsigned int *PUINT, *LPUINT;

//	The language identifier (the documented Win32 typedef).
typedef unsigned short LANGID;

//	The language extraction macros and the language constants (the
//	documented WinNT values).
#define PRIMARYLANGID(langId) ((unsigned short)(langId) & 0x3ff)
#define SUBLANGID(langId) ((unsigned short)(langId) >> 10)
#define MAKELANGID(primary, sublang) \
  ((((unsigned short)(sublang)) << 10) | (unsigned short)(primary))
#define LANG_NEUTRAL 0x00
#define LANG_ENGLISH 0x09
#define LANG_CHINESE 0x04
#define LANG_JAPANESE 0x11
#define LANG_KOREAN 0x12
#define SUBLANG_NEUTRAL 0x00
#define SUBLANG_DEFAULT 0x01
#define SUBLANG_ENGLISH_US 0x01
#define SUBLANG_CHINESE_SIMPLIFIED 0x01
#define SUBLANG_CHINESE_TRADITIONAL 0x02
#define SUBLANG_JAPANESE 0x01
#define SUBLANG_KOREAN 0x01

//	The IME composition string attributes (the documented imm.h values).
#define ATTR_INPUT 0x00
#define ATTR_TARGET_CONVERTED 0x01
#define ATTR_CONVERTED 0x02
#define ATTR_TARGET_NOTCONVERTED 0x03
#define ATTR_INPUT_ERROR 0x04
#define ATTR_FIXEDCONVERTED 0x05

//	The IME conversion mode flags (the documented imm.h values).
#define IME_CMODE_ALPHANUMERIC 0x0000
#define IME_CMODE_NATIVE 0x0001
#define IME_CMODE_KATAKANA 0x0002
#define IME_CMODE_LANGUAGE 0x0003
#define IME_CMODE_FULLSHAPE 0x0008
#define IME_CMODE_ROMAN 0x0010
#define IME_CMODE_CHARCODE 0x0020
#define IME_CMODE_HANJACONVERT 0x0040
#define IME_CMODE_SOFTKBD 0x0080
#define IME_CMODE_NOCONVERSION 0x0100
#define IME_CMODE_EUDC 0x0200
#define IME_CMODE_SYMBOL 0x0400
#define IME_CMODE_FIXED 0x0800

//	The IME notifications (the documented imm.h values).
#define IMN_CLOSESTATUSWINDOW 0x0001
#define IMN_OPENSTATUSWINDOW 0x0002
#define IMN_CHANGECANDIDATEWINDOW 0x0003
#define IMN_CLOSECANDIDATEWINDOW 0x0004
#define IMN_OPENCANDIDATEWINDOW 0x0005
#define IMN_SETCONVERSIONMODE 0x0006
#define IMN_SETSENTENCEMODE 0x0007
#define IMN_SETOPENSTATUS 0x0008
#define IMN_SETCANDIDATEPOS 0x0009
#define IMN_SETCOMPOSITIONFONT 0x000A
#define IMN_SETCOMPOSITIONWINDOW 0x000B
#define IMN_GUIDELINE 0x000C
#define IMN_SETSTATUSWINDOWPOS 0x000D
#define IMN_PRIVATE 0x000E

//	The IME hotkey identifiers (the documented imm.h values).
#define IME_JHOTKEY_CLOSE_OPEN 0x30
#define IME_KHOTKEY_SHAPE_TOGGLE 0x50
#define IME_KHOTKEY_HANJACONVERT 0x51
#define IME_KHOTKEY_ENGLISH 0x52
#define IME_THOTKEY_IME_NONIME_TOGGLE 0x70
#define IME_THOTKEY_SHAPE_TOGGLE 0x71
#define IME_THOTKEY_SYMBOL_TOGGLE 0x72

//	The Win32 64-bit unions carry the documented ABI layout: an
//	anonymous struct in a union (the MS extension the real WinNT.h
//	uses). The pedantic test builds need the diagnostic waived for
//	this deliberate layout; the field access (LowPart/HighPart/
//	QuadPart/wProcessorArchitecture) matches the original code.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
struct LARGE_INTEGER {
  union {
    struct {
      DWORD LowPart;
      DWORD HighPart;
    };
    std::int64_t QuadPart;
  };
};

struct ULARGE_INTEGER {
  union {
    struct {
      DWORD LowPart;
      DWORD HighPart;
    };
    std::uint64_t QuadPart;
  };
};
#if defined(__clang__)
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
#endif

typedef ULARGE_INTEGER *PULARGE_INTEGER;

//	The high-resolution performance counter: the real monotonic clock
//	(the documented Win32 contract: the frequency is ticks/second,
//	the counter ticks; the port's tick is the nanosecond clock).
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER *frequency) {
  if (!frequency)
    return FALSE;
  frequency->QuadPart = 1000000000;	//	nanoseconds.
  return TRUE;
}
inline BOOL QueryPerformanceCounter(LARGE_INTEGER *counter) {
  if (!counter)
    return FALSE;
  counter->QuadPart = static_cast<std::int64_t>(
      std::chrono::steady_clock::now().time_since_epoch().count());
  return TRUE;
}
typedef LARGE_INTEGER *PLARGE_INTEGER;

#define WINAPI
#define CALLBACK
#define APIENTRY
#define TRUE 1
#define FALSE 0

#define INVALID_HANDLE_VALUE ((HANDLE)(std::intptr_t)-1)
#define WAIT_OBJECT_0 0u
#define MAXIMUM_WAIT_OBJECTS 64
#define WAIT_ABANDONED 0x80u
#define WAIT_ABANDONED_0 0x80u
//	The GDI ignore value (the documented wingdi.h constant).
#define IGNORE 0
#define WAIT_TIMEOUT 258u
#define WAIT_FAILED 0xFFFFFFFFu
#define INFINITE 0xFFFFFFFFu
#define GENERIC_READ 0x80000000u

//	The file seek origins (the documented Win32 values).
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
#define GENERIC_WRITE 0x40000000u
#define FILE_SHARE_READ 1u
#define FILE_SHARE_WRITE 2u
#define CREATE_NEW 1u
#define CREATE_ALWAYS 2u
#define OPEN_EXISTING 3u
#define OPEN_ALWAYS 4u
#define TRUNCATE_EXISTING 5u
#define FILE_ATTRIBUTE_READONLY 1u
#define FILE_ATTRIBUTE_HIDDEN 2u
#define FILE_ATTRIBUTE_SYSTEM 4u
#define FILE_ATTRIBUTE_DIRECTORY 0x10u
#define FILE_FLAG_WRITE_THROUGH 0x80000000u
#define FILE_FLAG_NO_BUFFERING 0x20000000u

//	The SDK 'byte' type some headers use.
typedef BYTE byte;
#define FILE_ATTRIBUTE_NORMAL 0x80u
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFFu
#define FORMAT_MESSAGE_FROM_SYSTEM 0x1000u
#define MB_OK 0
#define MB_OKCANCEL 1
#define MB_ABORTRETRYIGNORE 2
#define MB_YESNOCANCEL 3
#define MB_RETRYCANCEL 5
#define MB_ICONERROR 0x10
#define MB_ICONSTOP MB_ICONERROR
#define MB_ICONWARNING 0x30
#define MB_ICONINFORMATION 0x40
#define MB_YESNO 4
#define IDYES 6
#define IDNO 7
#define IDOK 1
#define IDCANCEL 2
//	The DllMain reason codes (the documented WinBase values; the original
//	DllMain switches on all four, so they must stay distinct).
#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_ATTACH 2
#define DLL_THREAD_DETACH 3
#define DLL_PROCESS_DETACH 0

//	Critical section init/enter/leave come from criticalsection.h; the
//	try-variant completes the Win32 surface.
inline BOOL TryEnterCriticalSection(CRITICAL_SECTION *pcs) {
  return pcs->mutex.try_lock() ? TRUE : FALSE;
}

//	Interlocked operations: real std::atomics on the target storage.
inline LONG InterlockedIncrement(volatile LONG *value) {
  return std::atomic_fetch_add_explicit(
      reinterpret_cast<volatile std::atomic<LONG> *>(value), 1,
      std::memory_order_seq_cst) + 1;
}
inline LONG InterlockedDecrement(volatile LONG *value) {
  return std::atomic_fetch_sub_explicit(
      reinterpret_cast<volatile std::atomic<LONG> *>(value), 1,
      std::memory_order_seq_cst) - 1;
}
inline LONG InterlockedExchange(volatile LONG *value, LONG exchange) {
  return std::atomic_exchange_explicit(
      reinterpret_cast<volatile std::atomic<LONG> *>(value), exchange,
      std::memory_order_seq_cst);
}
inline LONG InterlockedCompareExchange(volatile LONG *value, LONG exchange,
                                       LONG compare) {
  std::atomic_compare_exchange_strong_explicit(
      reinterpret_cast<volatile std::atomic<LONG> *>(value), &compare,
      exchange, std::memory_order_seq_cst, std::memory_order_seq_cst);
  return compare;
}

//	The `long`-spelled variants: the game code casts member addresses to
//	`volatile long *` (the Win32 LONG spelling); `long` is a distinct
//	32-bit type on this target, so the exchange needs its own overload.
inline long InterlockedExchange(volatile long *value, long exchange) {
  return std::atomic_exchange_explicit(
      reinterpret_cast<volatile std::atomic<long> *>(value), exchange,
      std::memory_order_seq_cst);
}

//	Time: the platform clock backs the tick counters (same source as ATime).
//	The message loop's quit signal: the browser loop adapter consumes the
//	quit flag; the seam records it.
inline void PostQuitMessage(int exitCode) {
  extern int pw153_quit_code;
  pw153_quit_code = exitCode;
}

#ifndef LRESULT
#define LRESULT LONG_PTR
#endif
#ifndef LPRESULT
typedef LRESULT *LPRESULT;
#endif
#ifndef LPCTSTR
#define LPCTSTR const wchar_t *
#endif
#ifndef LPTSTR
#define LPTSTR wchar_t *
#endif
#ifndef PTSTR
#define PTSTR wchar_t *
#endif
#ifndef PTCHAR
#define PTCHAR wchar_t *
#endif
#ifndef PCTSTR
#define PCTSTR const wchar_t *
#endif

#define GWL_WNDPROC (-4)
#define GWLP_WNDPROC (-4)
#define GWLP_USERDATA (-21)
#define GWL_STYLE (-16)
#define GWL_HWNDPARENT (-8)
#define GWL_EXSTYLE (-20)
#define GWL_USERDATA (-21)

inline int lstrlenW(const wchar_t *text) {
  if (!text) return 0;
  int length = 0;
  while (text[length]) ++length;
  return length;
}
inline int lstrlen(const wchar_t *text) {
  return lstrlenW(text);
}
inline int lstrlenA(const char *text) {
  if (!text) return 0;
  int length = 0;
  while (text[length]) ++length;
  return length;
}

//	lstrcpy: the kernel32 copy without a length check (the callers size
//	the buffer themselves, as the Win32 contract requires).
inline wchar_t *lstrcpyW(wchar_t *dst, const wchar_t *src) {
  wchar_t *out = dst;
  while ((*dst++ = *src++) != L'\0') {}
  return out;
}
inline wchar_t *lstrcpy(wchar_t *dst, const wchar_t *src) {
  return lstrcpyW(dst, src);
}
inline char *lstrcpyA(char *dst, const char *src) {
  char *out = dst;
  while ((*dst++ = *src++) != '\0') {}
  return out;
}

//	The character advance (the documented Win32 signature: the next
//	character, or the terminator's own address at the end; the C form
//	returns a mutable pointer from the const input).
inline wchar_t *CharNextW(const wchar_t *text) {
  return const_cast<wchar_t *>(*text ? text + 1 : text);
}
inline char *CharNextA(const char *text) {
  return const_cast<char *>(*text ? text + 1 : text);
}
inline wchar_t *CharNext(const wchar_t *text) {
  return CharNextW(text);
}

inline int _access(const char *path, int mode) {
  if (!path || (mode != 0 && mode != 2 && mode != 4 && mode != 6)) {
    errno = EINVAL;
    return -1;
  }
  return access(path, mode);
}

// The browser host supplies screen coordinates from pointer events.
extern "C" void pw153_set_cursor_position(LONG x, LONG y);
BOOL GetCursorPos(POINT *point);
//	SetCursorPos: the browser cursor itself is user-owned; the warp
//	updates the adapter's recorded position (the game-visible cursor
//	state GetCursorPos reports) and reports success.
inline BOOL SetCursorPos(int x, int y) {
  pw153_set_cursor_position(x, y);
  return TRUE;
}
// Coordinates refer to the browser host's single canvas, in CSS pixels.
extern "C" void pw153_set_canvas_origin(LONG x, LONG y);
BOOL ScreenToClient(HWND hWnd, POINT *point);
BOOL ClientToScreen(HWND hWnd, POINT *point);
BOOL CreateCaret(HWND hWnd, HBITMAP bitmap, int width, int height);
//	Mouse capture tracking for the single-window browser host: SetCapture
//	records the capturing window, GetCapture returns it, ReleaseCapture
//	clears it. The JS host owns the real pointer capture; this tracks the
//	Win32 capture contract the client queries. The shared state lives in a
//	single accessor so every TU sees the same window.
inline HWND &pw153_capture_window() {
  static HWND s_captured = nullptr;
  return s_captured;
}
inline HWND SetCapture(HWND hWnd) {
  pw153_capture_window() = hWnd;
  return hWnd;
}
inline HWND GetCapture() {
  return pw153_capture_window();
}
inline BOOL ReleaseCapture() {
  pw153_capture_window() = nullptr;
  return TRUE;
}

//	The GetAncestor constants.
#define GA_PARENT 1
#define GA_ROOT 2
#define GA_ROOTOWNER 3

inline HWND GetAncestor(HWND hWnd, UINT flags) {
  (void)hWnd;
  (void)flags;
  return nullptr;
}

//	The window procedure: the adapter's dispatch supplies the real
//	pump; the type is the documented Win32 signature.
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);

//	The message sending: the adapter's dispatch supplies the real pump;
//	headless messages have no handler.
inline LRESULT SendMessageA(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  (void)hWnd;
  (void)msg;
  (void)wParam;
  (void)lParam;
  return 0;
}
inline LRESULT SendMessageW(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  return SendMessageA(hWnd, msg, wParam, lParam);
}
inline LRESULT SendMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  return SendMessageA(hWnd, msg, wParam, lParam);
}

//	The message posting: the browser host owns the input dispatch and
//	there is no Win32 message queue, so posting fails (the Win32 result
//	for a window without a queue); the callers' failure paths handle it.
//	The adapter's dispatch (m-boot) replaces the pump where delivery is
//	actually needed.
inline BOOL PostMessageA(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  (void)hWnd;
  (void)msg;
  (void)wParam;
  (void)lParam;
  return FALSE;
}
inline BOOL PostMessageW(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  return PostMessageA(hWnd, msg, wParam, lParam);
}
inline BOOL PostMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  return PostMessageA(hWnd, msg, wParam, lParam);
}

//	The child window enumeration: the browser target has no child
//	windows, so the callback is never invoked and the enumeration
//	succeeds (the Win32 semantics for a window without children).
inline BOOL EnumChildWindows(HWND hWnd, WNDENUMPROC callback, LPARAM lParam) {
  (void)hWnd;
  (void)callback;
  (void)lParam;
  return TRUE;
}

//	The file copy: the real byte copy over the adapter's filesystem.
#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND 2
#endif
#ifndef ERROR_ACCESS_DENIED
#define ERROR_ACCESS_DENIED 5
#endif
#ifndef ERROR_FILE_EXISTS
#define ERROR_FILE_EXISTS 80
#endif
inline BOOL CopyFileA(const char *existing, const char *destination,
                      BOOL failIfExists) {
  if (!existing || !destination) {
    SetLastError(ERROR_FILE_NOT_FOUND);
    return FALSE;
  }
  if (failIfExists) {
    if (FILE *probe = fopen(destination, "rb")) {
      fclose(probe);
      SetLastError(ERROR_FILE_EXISTS);
      return FALSE;
    }
  }
  FILE *source = fopen(existing, "rb");
  if (!source) {
    SetLastError(ERROR_FILE_NOT_FOUND);
    return FALSE;
  }
  FILE *target = fopen(destination, "wb");
  if (!target) {
    fclose(source);
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
  }
  char buffer[4096];
  std::size_t read;
  while ((read = fread(buffer, 1, sizeof(buffer), source)) > 0) {
    if (fwrite(buffer, 1, read, target) != read) {
      fclose(source);
      fclose(target);
      SetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
    }
  }
  const bool ok = ferror(source) == 0;
  fclose(source);
  fclose(target);
  return ok ? TRUE : FALSE;
}

//	The timers: the browser target has no Win32 timer queue; creation
//	fails (no WM_TIMER is ever delivered) and killing succeeds (there
//	is nothing left running).
typedef void (*TIMERPROC)(HWND, UINT, UINT_PTR, DWORD);
inline UINT_PTR SetTimer(HWND hWnd, UINT_PTR id, UINT elapse,
                         TIMERPROC procedure) {
  (void)hWnd;
  (void)id;
  (void)elapse;
  (void)procedure;
  return 0;
}
inline BOOL KillTimer(HWND hWnd, UINT_PTR id) {
  (void)hWnd;
  (void)id;
  return TRUE;
}

//	The focus window: no Win32 windows exist; there is no foreground
//	window.
inline HWND GetFocus() {
  return nullptr;
}
inline HWND GetForegroundWindow() {
  return nullptr;
}

//	The caret blink time: the documented default (the browser host
//	owns the caret; the AUI draws its own).
inline UINT GetCaretBlinkTime() {
  return 500;
}

//	The window destruction: no Win32 windows exist; destroying the
//	nonexistent window fails (the caller's error path handles it).
inline BOOL DestroyWindow(HWND hWnd) {
  (void)hWnd;
  return FALSE;
}

//	The window creation: the browser target has no Win32 window classes;
//	creation fails (no fake HWND is returned).
inline HWND CreateWindowExA(DWORD exStyle, const char *className,
                            const char *windowName, DWORD style, int x, int y,
                            int width, int height, HWND parent, HMENU menu,
                            HINSTANCE instance, void *param) {
  (void)exStyle;
  (void)className;
  (void)windowName;
  (void)style;
  (void)x;
  (void)y;
  (void)width;
  (void)height;
  (void)parent;
  (void)menu;
  (void)instance;
  (void)param;
  return nullptr;
}
inline HWND CreateWindowExW(DWORD exStyle, const wchar_t *className,
                            const wchar_t *windowName, DWORD style, int x,
                            int y, int width, int height, HWND parent,
                            HMENU menu, HINSTANCE instance, void *param) {
  (void)className;
  (void)windowName;
  return CreateWindowExA(exStyle, nullptr, nullptr, style, x, y, width, height,
                          parent, menu, instance, param);
}
inline HWND CreateWindowEx(DWORD exStyle, const wchar_t *className,
                           const wchar_t *windowName, DWORD style, int x,
                           int y, int width, int height, HWND parent,
                           HMENU menu, HINSTANCE instance, void *param) {
  return CreateWindowExW(exStyle, className, windowName, style, x, y, width,
                         height, parent, menu, instance, param);
}
inline HWND CreateWindow(const wchar_t *className, const wchar_t *windowName,
                         DWORD style, int x, int y, int width, int height,
                         HWND parent, HMENU menu, HINSTANCE instance,
                         void *param) {
  return CreateWindowExW(0, className, windowName, style, x, y, width, height,
                         parent, menu, instance, param);
}

//	The window styles (the documented WinUser values).
#define WS_OVERLAPPED 0x00000000L
#define WS_POPUP 0x80000000L
#define WS_CHILD 0x40000000L
#define WS_MINIMIZE 0x20000000L
#define WS_VISIBLE 0x10000000L
#define WS_DISABLED 0x08000000L
#define WS_CLIPSIBLINGS 0x04000000L
#define WS_CLIPCHILDREN 0x02000000L
#define WS_MAXIMIZE 0x01000000L
#define WS_CAPTION 0x00C00000L
#define WS_BORDER 0x00800000L
#define WS_DLGFRAME 0x00400000L
#define WS_VSCROLL 0x00200000L
#define WS_HSCROLL 0x00100000L
#define WS_SYSMENU 0x00080000L
#define WS_THICKFRAME 0x00040000L
#define WS_MINIMIZEBOX 0x00020000L
#define WS_MAXIMIZEBOX 0x00010000L
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_EX_TOPMOST 0x00000008L
#define WS_EX_NOACTIVATE 0x08000000L
#define WS_EX_STATICEDGE 0x00020000L
#define WS_EX_LAYERED 0x00080000L

//	The window class styles (the documented WinUser values).
#define CS_VREDRAW 0x0001
#define CS_HREDRAW 0x0002
#define CS_DBLCLKS 0x0008
#define CS_OWNDC 0x0020
#define CS_CLASSDC 0x0040
#define CS_PARENTDC 0x0080
#define CS_NOCLOSE 0x0200
#define CS_SAVEBITS 0x0800
#define CS_BYTEALIGNCLIENT 0x1000
#define CS_BYTEALIGNWINDOW 0x2000
#define CS_GLOBALCLASS 0x4000

//	The window class registration: the browser target has no Win32
//	window classes (the host supplies the canvas window); registration
//	fails (0) and the callers' fallback paths handle it.
typedef struct tagWNDCLASSEXA {
  UINT cbSize;
  UINT style;
  WNDPROC lpfnWndProc;
  int cbClsExtra;
  int cbWndExtra;
  HINSTANCE hInstance;
  HICON hIcon;
  HCURSOR hCursor;
  HBRUSH hbrBackground;
  LPCSTR lpszMenuName;
  LPCSTR lpszClassName;
} WNDCLASSEXA;

typedef struct tagWNDCLASSEXW {
  UINT cbSize;
  UINT style;
  WNDPROC lpfnWndProc;
  int cbClsExtra;
  int cbWndExtra;
  HINSTANCE hInstance;
  HICON hIcon;
  HCURSOR hCursor;
  HBRUSH hbrBackground;
  LPCWSTR lpszMenuName;
  LPCWSTR lpszClassName;
  HICON hIconSm;
} WNDCLASSEXW;
typedef WNDCLASSEXW WNDCLASSEX;

inline ATOM RegisterClassExA(const WNDCLASSEXA *windowClass) {
  (void)windowClass;
  return 0;
}
inline ATOM RegisterClassExW(const WNDCLASSEXW *windowClass) {
  (void)windowClass;
  return 0;
}
inline ATOM RegisterClassEx(const WNDCLASSEXW *windowClass) {
  return RegisterClassExW(windowClass);
}

//	The resource-id macros and the system cursor/icon names (the
//	documented WinUser values; the browser host owns cursor/icon
//	rendering, so the loads report none).
#define MAKEINTRESOURCE(i) ((const wchar_t *)(std::uintptr_t)(i))
#define IDC_ARROW MAKEINTRESOURCE(32512)
#define IDC_IBEAM MAKEINTRESOURCE(32513)
#define IDC_WAIT MAKEINTRESOURCE(32514)
#define IDC_CROSS MAKEINTRESOURCE(32515)
#define IDC_UPARROW MAKEINTRESOURCE(32516)
#define IDI_APPLICATION MAKEINTRESOURCE(32512)
#define IDI_HAND MAKEINTRESOURCE(32513)
#define IDI_QUESTION MAKEINTRESOURCE(32514)
#define IDI_EXCLAMATION MAKEINTRESOURCE(32515)
#define IDI_ASTERISK MAKEINTRESOURCE(32516)
#define IDI_WINLOGO MAKEINTRESOURCE(32517)

inline HCURSOR LoadCursorA(HINSTANCE instance, const char *name) {
  (void)instance;
  (void)name;
  return nullptr;
}
inline HCURSOR LoadCursorW(HINSTANCE instance, const wchar_t *name) {
  (void)instance;
  (void)name;
  return nullptr;
}
inline HCURSOR LoadCursor(HINSTANCE instance, const wchar_t *name) {
  return LoadCursorW(instance, name);
}
inline HICON LoadIconA(HINSTANCE instance, const char *name) {
  (void)instance;
  (void)name;
  return nullptr;
}
inline HICON LoadIconW(HINSTANCE instance, const wchar_t *name) {
  (void)instance;
  (void)name;
  return nullptr;
}
inline HICON LoadIcon(HINSTANCE instance, const wchar_t *name) {
  return LoadIconW(instance, name);
}

//	The stock objects (the documented WinGDI indices).
#define WHITE_BRUSH 0
#define LTGRAY_BRUSH 1
#define GRAY_BRUSH 2
#define DKGRAY_BRUSH 3
#define BLACK_BRUSH 4
#define WHITE_PEN 6
#define BLACK_PEN 7
#define NULL_PEN 8
#define SYSTEM_FONT 13
#define DEFAULT_GUI_FONT 17
inline HGDIOBJ GetStockObject(int object) {
  (void)object;
  return nullptr;
}

//	The window rectangle adjustment: the canvas window is borderless
//	(the host owns the frame); the client rectangle is the window
//	rectangle, so the adjustment adds nothing and succeeds.
inline BOOL AdjustWindowRect(RECT *rect, DWORD style, BOOL menu) {
  (void)style;
  (void)menu;
  return rect ? TRUE : FALSE;
}

//	The extended variant: the same borderless canvas (the extended
//	style adds nothing).
inline BOOL AdjustWindowRectEx(RECT *rect, DWORD style, BOOL menu,
                               DWORD exStyle) {
  (void)style;
  (void)menu;
  (void)exStyle;
  return rect ? TRUE : FALSE;
}

//	The redraw request: a null window invalidates everything (the
//	vacuous success - no windows exist to invalidate); a specific
//	window fails (no such window).
inline BOOL InvalidateRect(HWND hWnd, const RECT *rect, BOOL erase) {
  (void)rect;
  (void)erase;
  return hWnd ? FALSE : TRUE;
}

//	The parent window: no Win32 windows exist; there is no parent.
inline HWND GetParent(HWND hWnd) {
  (void)hWnd;
  return nullptr;
}

//	The window of a DC: no GDI DCs exist on the browser target.
inline HWND WindowFromDC(HDC device) {
  (void)device;
  return nullptr;
}

//	The 64-bit integers (the documented Win32 typedefs; the same
//	int64_t spelling wtypes.h uses so the redeclaration is identical).
typedef std::int64_t LONGLONG, *PLONGLONG;
typedef std::uint64_t ULONGLONG, *PULONGLONG;

#ifndef CONST
#define CONST const
#endif

//	The hook handle (the browser target has no hookable message flow).
typedef void *HHOOK;

//	The IME composition string flags, notification actions and
//	composition actions (the documented imm.h values).
#define GCS_COMPREADSTR 0x0001
#define GCS_COMPREADATTR 0x0002
#define GCS_COMPREADCLAUSE 0x0004
#define GCS_COMPSTR 0x0008
#define GCS_COMPATTR 0x0010
#define GCS_COMPCLAUSE 0x0020
#define GCS_CURSORPOS 0x0080
#define GCS_DELTASTART 0x0100
#define GCS_RESULTREADSTR 0x0200
#define GCS_RESULTREADCLAUSE 0x0400
#define GCS_RESULTSTR 0x0800
#define GCS_RESULTCLAUSE 0x1000
#define SCS_SETSTR (GCS_COMPREADSTR | GCS_COMPSTR)
#define SCS_CHANGEATTR (GCS_COMPREADATTR | GCS_COMPATTR)
#define SCS_CHANGECLAUSE (GCS_COMPREADCLAUSE | GCS_COMPCLAUSE)
#define NI_OPENCANDIDATE 0x0010
#define NI_CLOSECANDIDATE 0x0011
#define NI_SELECTCANDIDATESTR 0x0012
#define NI_CHANGECANDIDATELIST 0x0013
#define NI_FINALIZECONVERSIONRESULT 0x0014
#define NI_COMPOSITIONSTR 0x0015
#define NI_SETCANDIDATE_PAGESTART 0x0016
#define NI_SETCANDIDATE_PAGESIZE 0x0017
#define NI_IMEMENUSELECTED 0x0018
#define CPS_COMPLETE 0x0001
#define CPS_CONVERT 0x0002
#define CPS_REVERT 0x0003
#define CPS_CANCEL 0x0004
#define IME_CAND_READ 0x0004

//	The IME control actions (the documented imm.h values).
#define IMC_GETCANDIDATEPOS 0x0007
#define IMC_SETCANDIDATEPOS 0x0008
#define IMC_GETCOMPOSITIONFONT 0x0009
#define IMC_SETCOMPOSITIONFONT 0x000A
#define IMC_GETCOMPOSITIONWINDOW 0x000B
#define IMC_SETCOMPOSITIONWINDOW 0x000C
#define IMC_GETSTATUSWINDOWPOS 0x000F
#define IMC_SETSTATUSWINDOWPOS 0x0010
#define IMC_CLOSESTATUSWINDOW 0x0021
#define IMC_OPENSTATUSWINDOW 0x0022

//	The IME control message (the documented winuser value) and the
//	locale info constant (the documented winnls value).
#define WM_IME_CONTROL 0x0283
#define LOCALE_IDEFAULTANSICODEPAGE 0x1004
#ifndef LOCALE_INVARIANT
#define LOCALE_INVARIANT 0x0000007F
#endif

//	The page protections (the documented WinBase values).
#define PAGE_READWRITE 0x04
#define PAGE_EXECUTE_READWRITE 0x40

//	The page protection change: wasm memory has no mprotect-style page
//	protections the port can apply; the call fails explicitly (the
//	defence export/IAT walk is unreachable - the module handles are
//	NULL).
inline BOOL VirtualProtect(LPVOID address, SIZE_T size, DWORD newProtect,
                           LPDWORD oldProtect) {
  (void)address;
  (void)size;
  (void)newProtect;
  if (oldProtect)
    *oldProtect = 0;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

//	The GDI text draw: no GDI target exists on the browser target (the
//	AUI renders through the WebGL adapter); the draw fails.
inline BOOL TextOutA(HDC device, int x, int y, const char *text, int length) {
  (void)device;
  (void)x;
  (void)y;
  (void)text;
  (void)length;
  return FALSE;
}

//	The GDI extended text draw: the same no-GDI-target failure (the
//	IME hook's candidate window drawing never runs on the browser).
inline BOOL ExtTextOutW(HDC device, int x, int y, UINT options,
                        const RECT *rect, const wchar_t *text, UINT count,
                        const int *dx) {
  (void)device;
  (void)x;
  (void)y;
  (void)options;
  (void)rect;
  (void)text;
  (void)count;
  (void)dx;
  return FALSE;
}

//	The text alignment options (the documented wingdi.h values).
#define TA_NOUPDATECP 0x0000
#define TA_UPDATECP 0x0001
#define TA_LEFT 0x0000
#define TA_RIGHT 0x0002
#define TA_CENTER 0x0006
#define TA_TOP 0x0000
#define TA_BOTTOM 0x0002
#define TA_BASELINE 0x0018
#define TA_RTLREADING 0x0100

//	The ExtTextOut options (the documented wingdi.h values).
#define ETO_OPAQUE 0x0004
#define ETO_CLIPPED 0x0010

//	The A/W-neutral name (the UNICODE build maps it to the wide form).
#ifdef UNICODE
#define ExtTextOut ExtTextOutW
#endif

//	The GDI text state setters: no GDI target exists on the browser
//	target (the AUI renders through the WebGL adapter); the documented
//	failure values apply.
#define CLR_INVALID 0xFFFFFFFF
inline COLORREF SetTextColor(HDC hDC, COLORREF color) {
  (void)hDC;
  (void)color;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return CLR_INVALID;
}
inline COLORREF SetBkColor(HDC hDC, COLORREF color) {
  (void)hDC;
  (void)color;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return CLR_INVALID;
}
inline UINT SetTextAlign(HDC hDC, UINT align) {
  (void)hDC;
  (void)align;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return CLR_INVALID;	//	GDI_ERROR, the documented SetTextAlign failure value.
}

//	The lead byte test: the process locale is the single-byte C locale
//	(the port converts UTF-16/UTF-8 directly); no lead bytes exist.
inline BOOL IsDBCSLeadByte(BYTE character) {
  (void)character;
  return FALSE;
}
inline BOOL IsDBCSLeadByteEx(DWORD codePage, BYTE character) {
  (void)codePage;
  (void)character;
  return FALSE;
}

//	The system metrics: no windowing adapter is registered yet; the
//	metrics report zero until the canvas adapter supplies the real
//	viewport (m-boot wires the feed).
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#define SM_CXICON 11
#define SM_CYICON 12
#define SM_CXFULLSCREEN 16
#define SM_CYFULLSCREEN 17
#define SM_CXMIN 28
#define SM_CYMIN 29
inline int GetSystemMetrics(int index) {
  (void)index;
  return 0;
}

//	The window operations: no Win32 window exists on the browser
//	target (the host supplies the canvas); the operations take the
//	empty-window results (Win32: nonzero if the window was previously
//	visible) and the callers' error paths handle them.
inline BOOL MoveWindow(HWND hWnd, int x, int y, int width, int height,
                       BOOL repaint) {
  (void)hWnd;
  (void)x;
  (void)y;
  (void)width;
  (void)height;
  (void)repaint;
  return FALSE;
}
inline BOOL ShowWindow(HWND hWnd, int command) {
  (void)hWnd;
  (void)command;
  return FALSE;
}
inline BOOL UpdateWindow(HWND hWnd) {
  (void)hWnd;
  return FALSE;
}
inline BOOL SetForegroundWindow(HWND hWnd) {
  (void)hWnd;
  return FALSE;
}
inline HCURSOR SetCursor(HCURSOR cursor) {
  (void)cursor;
  return nullptr;
}

//	Cursor object lifetime: no real cursor objects exist on the browser
//	target (the loader below fails first, so no handle is ever live);
//	the destroy is vacuous and the file loader reports none.
inline BOOL DestroyCursor(HCURSOR cursor) {
  (void)cursor;
  return TRUE;
}
inline HCURSOR LoadCursorFromFileA(const char *file) {
  (void)file;
  return NULL;
}

//	The window creation record (the documented CREATESTRUCTW layout;
//	WM_CREATE hands it to the window procedure).
typedef struct tagCREATESTRUCTW {
  LPVOID lpCreateParams;
  HINSTANCE hInstance;
  HMENU hMenu;
  HWND hwndParent;
  int cy;
  int cx;
  int y;
  int x;
  LONG style;
  LPCWSTR lpszName;
  LPCWSTR lpszClass;
} CREATESTRUCTW, *LPCREATESTRUCTW;
typedef CREATESTRUCTW CREATESTRUCT;
typedef CREATESTRUCTW *LPCREATESTRUCT;


//	The window's creator thread: no Win32 windows exist; there is no
//	creator thread or process to report.
inline DWORD GetWindowThreadProcessId(HWND hWnd, DWORD *processId) {
  (void)hWnd;
  if (processId)
    *processId = 0;
  return 0;
}

//	The current process/thread pseudo-handles (the documented Win32
//	sentinel values).
inline HANDLE GetCurrentProcess() { return (HANDLE)-1; }
inline HANDLE GetCurrentThread() { return (HANDLE)-2; }

#define DUPLICATE_SAME_ACCESS 0x00000002

//	The handle duplication: the browser target has no Win32 handle
//	table; the duplication fails (the caller's error path handles it).
inline BOOL DuplicateHandle(HANDLE sourceProcess, HANDLE sourceHandle,
                            HANDLE targetProcess, HANDLE *targetHandle,
                            DWORD desiredAccess, BOOL inheritHandle,
                            DWORD options) {
  (void)sourceProcess;
  (void)sourceHandle;
  (void)targetProcess;
  (void)desiredAccess;
  (void)inheritHandle;
  (void)options;
  if (targetHandle)
    *targetHandle = nullptr;
  return FALSE;
}

//	The process termination: the adapter's quit code records the exit
//	and the runtime exit ends the module (the browser target's real
//	termination).
inline void ExitProcess(UINT exitCode) {
  extern int pw153_quit_code;
  pw153_quit_code = static_cast<int>(exitCode);
  exit(static_cast<int>(exitCode));
}

//	The process access rights (the documented Win32 values).
#ifndef PROCESS_TERMINATE
#define PROCESS_TERMINATE 0x0001
#endif
#ifndef PROCESS_VM_READ
#define PROCESS_VM_READ 0x0010
#endif
#ifndef PROCESS_QUERY_INFORMATION
#define PROCESS_QUERY_INFORMATION 0x0400
#endif
#ifndef THREAD_QUERY_INFORMATION
#define THREAD_QUERY_INFORMATION 0x0040
#endif

//	The process open: no other processes exist on the browser target;
//	the open fails (NULL).
inline HANDLE OpenProcess(DWORD access, BOOL inherit, DWORD processId) {
  (void)access;
  (void)inherit;
  (void)processId;
  return nullptr;
}

//	The process id: the single browser module is process 1.
inline DWORD GetCurrentProcessId() { return 1; }

//	The command line: the browser module has no process arguments; the
//	string is empty (the debug gates that scan it stay off).
inline const char *GetCommandLineA() { return ""; }
inline const wchar_t *GetCommandLineW() { return L""; }
inline const wchar_t *GetCommandLine() { return GetCommandLineW(); }

//	The heap: the C malloc/free heap (GetProcessHeap's sentinel; the
//	calls route to malloc/free).
inline HANDLE GetProcessHeap() { return (HANDLE)1; }
inline LPVOID HeapAlloc(HANDLE heap, DWORD flags, SIZE_T bytes) {
  (void)heap;
  (void)flags;
  return malloc(bytes);
}
inline BOOL HeapFree(HANDLE heap, DWORD flags, LPVOID memory) {
  (void)heap;
  (void)flags;
  free(memory);
  return TRUE;
}

//	The window find: no desktop windows exist; the find fails (NULL).
inline HWND FindWindowA(const char *className, const char *windowName) {
  (void)className;
  (void)windowName;
  return nullptr;
}
inline HWND FindWindowW(const wchar_t *className, const wchar_t *windowName) {
  (void)className;
  (void)windowName;
  return nullptr;
}
inline HWND FindWindow(const wchar_t *className, const wchar_t *windowName) {
  return FindWindowW(className, windowName);
}

//	The window class name: no windows exist; the name is empty.
inline int GetClassNameA(HWND hWnd, char *className, int maxCount) {
  (void)hWnd;
  if (className && maxCount)
    className[0] = '\0';
  return 0;
}
inline int GetClassNameW(HWND hWnd, wchar_t *className, int maxCount) {
  (void)hWnd;
  if (className && maxCount)
    className[0] = L'\0';
  return 0;
}
inline int GetClassName(HWND hWnd, wchar_t *className, int maxCount) {
  return GetClassNameW(hWnd, className, maxCount);
}

//	The enabled check rides the same registry: headless there are no
//	windows, and Win32 reports invalid windows as not enabled.
inline BOOL IsWindowEnabled(HWND hWnd) {
  (void)hWnd;
  return FALSE;
}

//	The top-level window enumeration: no desktop windows exist, so the
//	callback is never invoked; the defence callers treat the empty
//	result as failure (FALSE) per their verified error paths.
inline BOOL EnumWindows(WNDENUMPROC callback, LPARAM lParam) {
  (void)callback;
  (void)lParam;
  return FALSE;
}

//	The system time to file time: the real civil-date conversion to
//	100ns units since 1601-01-01 (the documented Win32 contract).
inline BOOL SystemTimeToFileTime(const SYSTEMTIME *systemTime,
                                 FILETIME *fileTime) {
  if (!systemTime || !fileTime)
    return FALSE;
  long long y = systemTime->wYear;
  unsigned m = systemTime->wMonth, d = systemTime->wDay;
  y -= m <= 2;
  const long long era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = (unsigned)(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  const long long days = era * 146097 + (long long)doe - 719468 + 134774;
  const long long ft = (days * 86400 + systemTime->wHour * 3600
      + systemTime->wMinute * 60 + systemTime->wSecond) * 10000000LL
      + systemTime->wMilliseconds * 10000LL;
  fileTime->dwLowDateTime = (DWORD)(ft & 0xFFFFFFFF);
  fileTime->dwHighDateTime = (DWORD)((unsigned long long)ft >> 32);
  return TRUE;
}

//	The file time to local file time: the real conversion (the UTC
//	FILETIME shifted by the host timezone's local bias).
inline BOOL FileTimeToLocalFileTime(const FILETIME *fileTime,
                                    FILETIME *localFileTime) {
  if (!fileTime || !localFileTime)
    return FALSE;
  const std::uint64_t n = (static_cast<std::uint64_t>(fileTime->dwHighDateTime)
                           << 32) |
                          fileTime->dwLowDateTime;
  const std::time_t unixTime =
      static_cast<std::time_t>(n / 10000000ULL - 11644473600ULL);
  std::tm local{};
  if (!localtime_r(&unixTime, &local))
    return FALSE;
  const long bias = local.tm_gmtoff;	//	seconds east of UTC.
  const std::int64_t shifted =
      static_cast<std::int64_t>(n) + static_cast<std::int64_t>(bias) * 10000000LL;
  const std::uint64_t result = static_cast<std::uint64_t>(shifted);
  localFileTime->dwLowDateTime = static_cast<DWORD>(result);
  localFileTime->dwHighDateTime = static_cast<DWORD>(result >> 32);
  return TRUE;
}

//	The token privilege record (the documented WinNT layout).
typedef HANDLE *PHANDLE;
typedef struct _LUID {
  DWORD LowPart;
  LONG HighPart;
} LUID, *PLUID;

typedef struct _LUID_AND_ATTRIBUTES {
  LUID Luid;
  DWORD Attributes;
} LUID_AND_ATTRIBUTES, *PLUID_AND_ATTRIBUTES;

typedef struct _TOKEN_PRIVILEGES {
  DWORD PrivilegeCount;
  LUID_AND_ATTRIBUTES Privileges[1];
} TOKEN_PRIVILEGES, *PTOKEN_PRIVILEGES;

//	The token access rights and privilege constants (the documented
//	WinNT values; the port is a UNICODE seam, so the privilege name is
//	the wide literal).
#define TOKEN_QUERY 0x0008
#define TOKEN_ADJUST_PRIVILEGES 0x0020
#define SE_PRIVILEGE_ENABLED 0x00000002
#define SE_DEBUG_NAME L"SeDebugPrivilege"
#define ERROR_NO_TOKEN 1008
#define ERROR_NOT_ALL_ASSIGNED 1300

//	Access-token queries: the browser target has no process/thread
//	tokens and no privilege model; the calls fail explicitly (the crash
//	writer's null checks then skip the dump path).
inline BOOL OpenThreadToken(HANDLE thread, DWORD access, BOOL openAsSelf,
                            PHANDLE token) {
  (void)thread; (void)access; (void)openAsSelf; (void)token;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}
inline BOOL OpenProcessToken(HANDLE process, DWORD access, PHANDLE token) {
  (void)process; (void)access; (void)token;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}
inline BOOL LookupPrivilegeValue(const wchar_t *system, const wchar_t *name,
                                 PLUID luid) {
  (void)system; (void)name; (void)luid;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}
inline BOOL AdjustTokenPrivileges(HANDLE token, BOOL disableAll,
                                  const TOKEN_PRIVILEGES *newState,
                                  DWORD bufferLength,
                                  TOKEN_PRIVILEGES *previousState,
                                  DWORD *returnLength) {
  (void)token; (void)disableAll; (void)newState; (void)bufferLength;
  (void)previousState; (void)returnLength;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

//	The critical section pointer (criticalsection.h owns the type).
typedef CRITICAL_SECTION *LPCRITICAL_SECTION;

//	wsprintf: the user32 formatter without the count (the MSVC
//	contract: the caller guarantees the buffer; the port's wide
//	formatter writes nul-terminated the same way).
inline int wsprintfW(wchar_t *out, const wchar_t *format, ...) {
  va_list args;
  va_start(args, format);
  const int written = pw153::platform::wide_vsprintf(out, format, args);
  va_end(args);
  return written;
}
inline int wsprintf(wchar_t *out, const wchar_t *format, ...) {
  va_list args;
  va_start(args, format);
  const int written = pw153::platform::wide_vsprintf(out, format, args);
  va_end(args);
  return written;
}

//	The FILETIME to DOS date/time (the real conversion: the documented
//	DOS packed fields from the local time).
inline BOOL FileTimeToDosDateTime(const FILETIME *fileTime, WORD *dosDate,
                                   WORD *dosTime) {
  if (!fileTime || !dosDate || !dosTime)
    return FALSE;
  const std::uint64_t n = (static_cast<std::uint64_t>(fileTime->dwHighDateTime)
                           << 32) |
                          fileTime->dwLowDateTime;
  const std::time_t unixTime =
      static_cast<std::time_t>(n / 10000000ULL - 11644473600ULL);
  std::tm local{};
  if (!localtime_r(&unixTime, &local))
    return FALSE;
  const int year = local.tm_year + 1900;
  if (year < 1980 || year > 2107)
    return FALSE;
  *dosDate = static_cast<WORD>(((year - 1980) << 9) |
                               ((local.tm_mon + 1) << 5) |
                               local.tm_mday);
  *dosTime = static_cast<WORD>((local.tm_hour << 11) |
                               (local.tm_min << 5) |
                               (local.tm_sec / 2));
  return TRUE;
}

//	The debugger presence: no native debugger attaches to a wasm
//	module; the query reports none.
inline BOOL IsDebuggerPresent() {
  return FALSE;
}

//	The SEH continuation results (the documented values).
#define EXCEPTION_CONTINUE_EXECUTION (-1)
#define EXCEPTION_CONTINUE_SEARCH 0

//	The NT native query types live in the defence's own headers
//	(hardinfo.h declares its local UNICODE_STRING/OBJECT_ATTRIBUTES);
//	the seam provides the pointer typedefs they reference.
typedef SIZE_T *PSIZE_T;
typedef wchar_t *PWSTR;
typedef DWORD ACCESS_MASK;
#define ERROR_BUFFER_OVERFLOW 111

//	The virtual memory query: no foreign process address space exists;
//	the query reports nothing (0).
typedef struct _MEMORY_BASIC_INFORMATION {
  PVOID BaseAddress;
  PVOID AllocationBase;
  DWORD AllocationProtect;
  SIZE_T RegionSize;
  DWORD State;
  DWORD Protect;
  DWORD Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
inline SIZE_T VirtualQueryEx(HANDLE process, const void *address,
                             PMEMORY_BASIC_INFORMATION buffer, SIZE_T length) {
  (void)process;
  (void)address;
  (void)buffer;
  (void)length;
  return 0;
}

//	The process startup record (the documented STARTUPINFOW layout; the
//	game is UNICODE so STARTUPINFO is the wide variant).
typedef struct _STARTUPINFOW {
  DWORD cb;
  LPWSTR lpReserved;
  LPWSTR lpDesktop;
  LPWSTR lpTitle;
  DWORD dwX, dwY, dwXSize, dwYSize;
  DWORD dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
  WORD wShowWindow, cbReserved2;
  LPBYTE lpReserved2;
  HANDLE hStdInput, hStdOutput, hStdError;
} STARTUPINFOW, *LPSTARTUPINFOW;
typedef STARTUPINFOW STARTUPINFO;
typedef STARTUPINFOW *LPSTARTUPINFO;

typedef struct _PROCESS_INFORMATION {
  HANDLE hProcess;
  HANDLE hThread;
  DWORD dwProcessId;
  DWORD dwThreadId;
} PROCESS_INFORMATION, *LPPROCESS_INFORMATION;

//	The process creation: the browser target cannot spawn native processes;
//	CreateProcess fails (the caller's error path handles it).
inline BOOL CreateProcessW(const wchar_t *application, const wchar_t *command,
                           void *processAttrs, void *threadAttrs,
                           BOOL inheritHandles, DWORD creationFlags,
                           void *environment, const wchar_t *currentDir,
                           STARTUPINFOW *startup, PROCESS_INFORMATION *info) {
  (void)application;
  (void)command;
  (void)processAttrs;
  (void)threadAttrs;
  (void)inheritHandles;
  (void)creationFlags;
  (void)environment;
  (void)currentDir;
  if (info)
    std::memset(info, 0, sizeof(*info));
  (void)startup;
  return FALSE;
}
inline BOOL CreateProcess(const wchar_t *application, const wchar_t *command,
                          void *processAttrs, void *threadAttrs,
                          BOOL inheritHandles, DWORD creationFlags,
                          void *environment, const wchar_t *currentDir,
                          STARTUPINFOW *startup, PROCESS_INFORMATION *info) {
  return CreateProcessW(application, command, processAttrs, threadAttrs,
                        inheritHandles, creationFlags, environment, currentDir,
                        startup, info);
}

//	WinExec: the legacy process launcher; the browser target cannot spawn
//	native processes, so it reports file-not-found (the callers' error
//	paths handle it, like CreateProcess above).
inline UINT WinExec(const char *commandLine, UINT showCmd) {
  (void)commandLine;
  (void)showCmd;
  return ERROR_FILE_NOT_FOUND;
}

//	The OS version: the documented GetVersion encoding for Windows 5.1
//	build 2600 (the baseline the client targets; LOBYTE(LOWORD) == 5).
inline DWORD GetVersion() {
  return 0x0A280105u;
}

#define ERROR_CALL_NOT_IMPLEMENTED 120

#define INVALID_FILE_SIZE (static_cast<DWORD>(-1))

typedef struct IUnknown *LPUNKNOWN;

//	The COM apartment entry points: the port's activation registry
//	(atlcom_registry.cpp) owns the definitions. unknwn.h supplies the
//	GUID layout (its own Windows.h include is guard-blocked here).
//	The COM identifier layout (the documented GUID byte layout; the
//	__uuidof operator over __declspec(uuid) types yields references
//	to it).
typedef struct _GUID {
  unsigned long Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;

//	Pointer forms of the identifier (the HID/setupapi headers the game
//	declares through UsbHidKey.h use LPGUID; the const form shares the
//	surface).
typedef GUID *LPGUID;
typedef const GUID *LPCGUID;

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
//	The class-identifier comparison (the same documented byte compare
//	over the seam's opaque REFIID/REFCLSID pointer forms).
inline bool IsEqualCLSID(const GUID &a, const GUID &b) {
  return IsEqualGUID(a, b);
}
inline bool IsEqualCLSID(const void *a, const void *b) {
  return a && b && std::memcmp(a, b, 16) == 0;
}
inline bool IsEqualCLSID(const void *riid, const GUID &iid) {
  return IsEqualIID(riid, iid);
}
inline bool IsEqualCLSID(const GUID &iid, const void *riid) {
  return IsEqualIID(riid, iid);
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

#include "unknwn.h"
HRESULT CoInitialize(void *reserved);
void CoUninitialize();

//	The COM apartment init (the same activation registry; the apartment
//	model is the caller's bookkeeping on this target).
#define COINIT_APARTMENTTHREADED 0x2
#define COINIT_MULTITHREADED 0x0
inline HRESULT CoInitializeEx(void *reserved, DWORD coInit) {
  (void)coInit;
  return CoInitialize(reserved);
}

//	The direct COM activation: the port's activation registry
//	(atlcom_registry.cpp) is empty, so the activation fails explicitly
//	(the callers' SUCCEEDED/FAILED guards take the non-COM path). The
//	signature is the documented CoCreateInstance contract.
inline HRESULT CoCreateInstance(const GUID &classId, void *outer,
                                 DWORD classContext, const GUID &interfaceId,
                                 void **result) {
  (void)classId;
  (void)outer;
  (void)classContext;
  (void)interfaceId;
  if (result)
    *result = nullptr;
  return E_FAIL;
}

#ifndef TEXT
#define TEXT(text) L##text
#endif

//	The overlapped-I/O record (the documented OVERLAPPED layout; the
//	named-pipe callers memset it and set hEvent). The anonymous struct in
//	the union is the documented Win32 ABI shape, like the LARGE_INTEGER
//	layout above.
#pragma GCC diagnostic push
//\tThe two anonymous-struct flags are clang-only; GCC errors on the
//\tunknown option under -Werror=pragmas.
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#pragma GCC diagnostic ignored "-Wnested-anon-types"
#endif
#pragma GCC diagnostic ignored "-Wpedantic"
typedef struct _OVERLAPPED {
  ULONG_PTR Internal;
  ULONG_PTR InternalHigh;
  union {
    struct {
      DWORD Offset;
      DWORD OffsetHigh;
    };
    PVOID Pointer;
  };
  HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;
#pragma GCC diagnostic pop

//	The named-pipe open/mode flags (the documented Win32 values).
#define PIPE_ACCESS_INBOUND 0x00000001
#define PIPE_ACCESS_OUTBOUND 0x00000002
#define PIPE_ACCESS_DUPLEX 0x00000003
#define FILE_FLAG_OVERLAPPED 0x40000000
#define PIPE_WAIT 0x00000000
#define PIPE_NOWAIT 0x00000001
#define PIPE_READMODE_BYTE 0x00000000
#define PIPE_READMODE_MESSAGE 0x00000002
#define PIPE_TYPE_BYTE 0x00000000
#define PIPE_TYPE_MESSAGE 0x00000004

//	The overlapped operation queued (the documented Win32 value; the
//	pipe callers compare GetLastError against it).
#define ERROR_IO_PENDING 997

//	The named pipe server: the browser target has no named-pipe
//	filesystem; creation fails explicitly (the caller's error path
//	logs and disables the downloader integration). The signature is
//	the documented CreateNamedPipe contract.
inline HANDLE CreateNamedPipeW(const wchar_t *name, DWORD openMode,
                               DWORD pipeMode, DWORD maxInstances,
                               DWORD outBufferSize, DWORD inBufferSize,
                               DWORD defaultTimeout, void *security) {
  (void)name;
  (void)openMode;
  (void)pipeMode;
  (void)maxInstances;
  (void)outBufferSize;
  (void)inBufferSize;
  (void)defaultTimeout;
  (void)security;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return INVALID_HANDLE_VALUE;
}
inline HANDLE CreateNamedPipeA(const char *name, DWORD openMode,
                               DWORD pipeMode, DWORD maxInstances,
                               DWORD outBufferSize, DWORD inBufferSize,
                               DWORD defaultTimeout, void *security) {
  (void)name;
  (void)openMode;
  (void)pipeMode;
  (void)maxInstances;
  (void)outBufferSize;
  (void)inBufferSize;
  (void)defaultTimeout;
  (void)security;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return INVALID_HANDLE_VALUE;
}
inline HANDLE CreateNamedPipe(const wchar_t *name, DWORD openMode,
                              DWORD pipeMode, DWORD maxInstances,
                              DWORD outBufferSize, DWORD inBufferSize,
                              DWORD defaultTimeout, void *security) {
  return CreateNamedPipeW(name, openMode, pipeMode, maxInstances,
                          outBufferSize, inBufferSize, defaultTimeout,
                          security);
}

//	The named-pipe connect: no pipe instance exists (creation fails
//	above); the wait fails explicitly.
inline BOOL ConnectNamedPipe(HANDLE pipe, LPOVERLAPPED overlapped) {
  (void)pipe;
  (void)overlapped;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

//	File write: the handle came from CreateFileA's fopen - the adapter's
//	file registry resolves; an unregistered handle fails explicitly.
inline BOOL WriteFile(HANDLE handle, const void *buffer, DWORD toWrite,
                      DWORD *written, void *overlapped) {
  (void)handle;
  (void)buffer;
  (void)toWrite;
  (void)overlapped;
  if (written)
    *written = 0;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

//	The file size: the real filesystem stat (the documented semantics).
inline DWORD GetFileSize(HANDLE handle, DWORD *highPart) {
  if (highPart)
    *highPart = 0;
  //	The handle came from CreateFileA's fopen - the adapter's file
  //	registry resolves; a null/invalid handle reports invalid.
  if (!handle || handle == INVALID_HANDLE_VALUE)
    return INVALID_FILE_SIZE;
  return 0;
}

//	File read: the handle came from CreateFileA's fopen - the adapter's
//	file registry resolves; an unregistered handle fails explicitly.
inline BOOL ReadFile(HANDLE handle, void *buffer, DWORD toRead,
                     DWORD *read, void *overlapped) {
  (void)handle;
  (void)buffer;
  (void)toRead;
  (void)overlapped;
  if (read)
    *read = 0;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

//	File seek: the DirectShow stream wrapper positions through it; the
//	adapter's file registry owns real positioning, so an unregistered
//	handle fails explicitly (the documented INVALID_SET_FILE_POINTER).
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER 0xFFFFFFFF
#endif
inline DWORD SetFilePointer(HANDLE handle, LONG distance, LONG *highDistance,
                            DWORD method) {
  (void)handle;
  (void)distance;
  (void)highDistance;
  (void)method;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return INVALID_SET_FILE_POINTER;
}

//	The 32x32->64 product helper (the documented winnt macro; the
//	portable product form).
#define UInt32x32To64(a, b) \
  ((unsigned long long)(unsigned long)(a) * (unsigned long long)(unsigned long)(b))

//	The 32x32->64 signed product (the documented winnt macro).
#define Int32x32To64(a, b) \
  ((long long)(long)(a) * (long long)(long)(b))

//	File attributes: the real filesystem stat (the documented semantics).
inline DWORD GetFileAttributesA(const char *name) {
  if (!name)
    return INVALID_FILE_ATTRIBUTES;
  struct stat info;
  if (stat(name, &info) != 0)
    return INVALID_FILE_ATTRIBUTES;
  DWORD attributes = FILE_ATTRIBUTE_NORMAL;
  if (S_ISDIR(info.st_mode))
    attributes = FILE_ATTRIBUTE_DIRECTORY;
  return attributes;
}
inline DWORD GetFileAttributesW(const wchar_t *name) {
  std::string narrow;
  if (!pw153::platform::utf16_to_utf8(name ? name : L"", narrow))
    return INVALID_FILE_ATTRIBUTES;
  return GetFileAttributesA(narrow.c_str());
}

//	File attribute write: the read-only bit maps to the real filesystem
//	write permission (chmod); the other DOS attributes have no target
//	equivalent and are ignored (the callers only clear FILE_ATTRIBUTE_READONLY).
inline BOOL SetFileAttributesA(const char *name, DWORD attributes) {
  if (!name)
    return FALSE;
  struct stat info;
  if (stat(name, &info) != 0)
    return FALSE;
  mode_t mode = info.st_mode;
  if (attributes & FILE_ATTRIBUTE_READONLY)
    mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
  else
    mode |= S_IWUSR;
  return chmod(name, mode) == 0 ? TRUE : FALSE;
}
inline BOOL SetFileAttributesW(const wchar_t *name, DWORD attributes) {
  std::string narrow;
  if (!pw153::platform::utf16_to_utf8(name ? name : L"", narrow))
    return FALSE;
  return SetFileAttributesA(narrow.c_str(), attributes);
}

#define SW_HIDE 0
#define SW_SHOWNORMAL 1
#define SW_SHOWMINIMIZED 2
#define SW_SHOWMAXIMIZED 3
#define SW_SHOWNOACTIVATE 4
#define SW_SHOW 5
#define SW_MINIMIZE 6
#define SW_MAXIMIZE 3
#define SW_RESTORE 9

//	The window rect: the adapter's window registry supplies real windows;
//	headless the rect is empty.
inline BOOL GetWindowRect(HWND hWnd, RECT *rect) {
  (void)hWnd;
  if (rect) {
    rect->left = 0;
    rect->top = 0;
    rect->right = 0;
    rect->bottom = 0;
  }
  return TRUE;
}
inline BOOL SetWindowPos(HWND hWnd, HWND after, int x, int y, int width,
                         int height, unsigned flags) {
  (void)hWnd;
  (void)after;
  (void)x;
  (void)y;
  (void)width;
  (void)height;
  (void)flags;
  return TRUE;
}

#ifndef MAKELPARAM
#define MAKELPARAM(low, high) ((LPARAM)(static_cast<DWORD>(static_cast<WORD>(low)) | (static_cast<DWORD>(static_cast<WORD>(high)) << 16)))
#endif
#ifndef MAKEWPARAM
#define MAKEWPARAM(low, high) ((WPARAM)(static_cast<DWORD>(static_cast<WORD>(low)) | (static_cast<DWORD>(static_cast<WORD>(high)) << 16)))
#endif
#ifndef MAKELRESULT
#define MAKELRESULT(low, high) ((LRESULT)(static_cast<DWORD>(static_cast<WORD>(low)) | (static_cast<DWORD>(static_cast<WORD>(high)) << 16)))
#endif

inline BOOL GetClientRect(HWND hWnd, RECT *rect) {
  (void)hWnd;
  if (rect) {
    rect->left = 0;
    rect->top = 0;
    rect->right = 0;
    rect->bottom = 0;
  }
  return TRUE;
}

//	Window checks: the adapter's window registry supplies real windows;
//	headless there are none.
inline BOOL IsWindow(HWND hWnd) {
  (void)hWnd;
  return FALSE;
}
inline BOOL IsWindowVisible(HWND hWnd) {
  (void)hWnd;
  return FALSE;
}

//	The module file name: the browser target has no module files; the
//	call reports the empty name (the callers' fallbacks handle it).
inline DWORD GetModuleFileNameA(HMODULE module, char *name, DWORD size) {
  (void)module;
  if (name && size)
    name[0] = '\0';
  return 0;
}
inline DWORD GetModuleFileNameW(HMODULE module, wchar_t *name, DWORD size) {
  (void)module;
  if (name && size)
    name[0] = L'\0';
  return 0;
}
inline DWORD GetModuleFileName(HMODULE module, wchar_t *name, DWORD size) {
  return GetModuleFileNameW(module, name, size);
}

//	Dynamic library loading: the browser target has no DLLs; the calls
//	fail explicitly (the callers' null checks handle it).
inline HMODULE LoadLibraryA(const char *name) {
  (void)name;
  return nullptr;
}
inline HMODULE LoadLibraryW(const wchar_t *name) {
  (void)name;
  return nullptr;
}
//	The A/W-neutral name (the UNICODE build maps it to the wide form).
inline HMODULE LoadLibrary(const wchar_t *name) {
  return LoadLibraryW(name);
}
inline void *GetProcAddress(HMODULE module, const char *name) {
  (void)module;
  (void)name;
  return nullptr;
}
inline BOOL FreeLibrary(HMODULE module) {
  (void)module;
  return TRUE;
}
inline HMODULE GetModuleHandleA(const char *name) {
  (void)name;
  return nullptr;
}
inline HMODULE GetModuleHandleW(const wchar_t *name) {
  (void)name;
  return nullptr;
}

//	The working directory: the real POSIX cwd over the wide path
//	conversion (the documented Win32 semantics).
inline DWORD GetCurrentDirectoryW(DWORD length, wchar_t *buffer) {
  char narrow[MAX_PATH];
  if (!getcwd(narrow, sizeof(narrow)))
    return 0;
  std::wstring wide;
  if (!pw153::platform::utf8_to_utf16(narrow, wide))
    return 0;
  if (buffer && length > wide.size()) {
    std::memcpy(buffer, wide.data(), wide.size() * sizeof(wchar_t));
    buffer[wide.size()] = L'\0';
  }
  return static_cast<DWORD>(wide.size());
}

inline BOOL SetCurrentDirectoryW(const wchar_t *path) {
  std::string narrow;
  if (!pw153::platform::utf16_to_utf8(path ? path : L"", narrow))
    return FALSE;
  return chdir(narrow.c_str()) == 0 ? TRUE : FALSE;
}

inline DWORD GetCurrentDirectoryA(DWORD length, char *buffer) {
  char narrow[MAX_PATH];
  if (!getcwd(narrow, sizeof(narrow)))
    return 0;
  const DWORD size = static_cast<DWORD>(std::strlen(narrow));
  if (buffer && length > size)
    std::strcpy(buffer, narrow);
  return size;
}

inline DWORD GetCurrentDirectory(DWORD length, wchar_t *buffer) {
  return GetCurrentDirectoryW(length, buffer);
}

inline BOOL SetCurrentDirectoryA(const char *path) {
  return chdir(path) == 0 ? TRUE : FALSE;
}

//	The MSVC _splitpath: divides a path into drive/dir/fname/ext; any
//	component pointer may be NULL. POSIX paths have no drive component;
//	Windows drive letters ("X:") are still recognized.
inline void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext) {
  const char *p = path ? path : "";
  const char *driveEnd = p;
  if (((p[0] >= 'A' && p[0] <= 'Z') || (p[0] >= 'a' && p[0] <= 'z')) && p[1] == ':')
    driveEnd = p + 2;
  const char *dirStart = driveEnd;
  const char *dirEnd = driveEnd;
  for (const char *q = dirStart; *q; ++q) {
    if (*q == '/' || *q == '\\')
      dirEnd = q + 1;  //  one past the last separator
  }
  const char *fnameStart = dirEnd;
  const char *end = p + std::strlen(p);
  const char *extStart = nullptr;  //  last dot inside the file name
  for (const char *q = fnameStart; q < end; ++q) {
    if (*q == '.')
      extStart = q;
  }
  if (drive) {
    std::size_t n = static_cast<std::size_t>(driveEnd - p);
    std::memcpy(drive, p, n);
    drive[n] = '\0';
  }
  if (dir) {
    std::size_t n = static_cast<std::size_t>(dirEnd - dirStart);
    std::memcpy(dir, dirStart, n);
    dir[n] = '\0';
  }
  if (fname) {
    std::size_t n = static_cast<std::size_t>((extStart ? extStart : end) - fnameStart);
    std::memcpy(fname, fnameStart, n);
    fname[n] = '\0';
  }
  if (ext) {
    std::size_t n = extStart ? static_cast<std::size_t>(end - extStart) : 0;
    if (extStart)
      std::memcpy(ext, extStart, n);
    ext[n] = '\0';
  }
}

//	The MSVC _wfopen: opens through the UTF-8 path conversion (the same
//	file the fopen path resolves).
inline FILE *_wfopen(const wchar_t *path, const wchar_t *mode) {
  std::string narrowPath;
  if (!pw153::platform::utf16_to_utf8(path ? path : L"", narrowPath))
    return nullptr;
  std::string narrowMode;
  if (!pw153::platform::utf16_to_utf8(mode ? mode : L"", narrowMode))
    return nullptr;
  return std::fopen(narrowPath.c_str(), narrowMode.c_str());
}

//	The MSVC _wremove: deletes through the UTF-8 path conversion.
inline int _wremove(const wchar_t *path) {
  std::string narrow;
  if (!pw153::platform::utf16_to_utf8(path ? path : L"", narrow))
    return -1;
  return std::remove(narrow.c_str());
}

//	Window text: the documented Win32 accessors over the adapter's window
//	registry (headless, the text is stored by the dialog adapter).
inline BOOL SetWindowTextA(HWND hWnd, const char *text) {
  (void)hWnd;
  (void)text;
  return TRUE;
}
inline BOOL SetWindowTextW(HWND hWnd, const wchar_t *text) {
  (void)hWnd;
  (void)text;
  return TRUE;
}
inline BOOL SetWindowText(HWND hWnd, const wchar_t *text) {
  return SetWindowTextW(hWnd, text);
}
inline int GetWindowTextA(HWND hWnd, char *text, int count) {
  (void)hWnd;
  if (text && count > 0)
    text[0] = '\0';
  return 0;
}
inline int GetWindowTextW(HWND hWnd, wchar_t *text, int count) {
  (void)hWnd;
  if (text && count > 0)
    text[0] = L'\0';
  return 0;
}
inline int GetWindowText(HWND hWnd, wchar_t *text, int count) {
  return GetWindowTextW(hWnd, text, count);
}

//	The default window procedure: the adapter's dispatch supplies the real
//	one; headless, messages have no default handling.
inline LRESULT DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  (void)hWnd;
  (void)msg;
  (void)wParam;
  (void)lParam;
  return 0;
}

//	Window data accessors: the documented Win32 semantics over the window
//	registry the dialog adapter owns (no real windows exist headless).
inline LONG GetWindowLong(HWND hWnd, int index) {
  (void)hWnd;
  (void)index;
  return 0;
}
inline LONG SetWindowLong(HWND hWnd, int index, LONG value) {
  (void)hWnd;
  (void)index;
  (void)value;
  return 0;
}
inline LONG SetWindowLongA(HWND hWnd, int index, LONG value) {
  return SetWindowLong(hWnd, index, value);
}
inline LONG SetWindowLongW(HWND hWnd, int index, LONG value) {
  return SetWindowLong(hWnd, index, value);
}
inline LONG GetWindowLongA(HWND hWnd, int index) {
  return GetWindowLong(hWnd, index);
}
inline LONG GetWindowLongW(HWND hWnd, int index) {
  return GetWindowLong(hWnd, index);
}

inline LONG_PTR GetWindowLongPtr(HWND hWnd, int index) {
  return GetWindowLong(hWnd, index);
}
inline LONG_PTR SetWindowLongPtr(HWND hWnd, int index, LONG_PTR value) {
  return SetWindowLong(hWnd, index, static_cast<LONG>(value));
}
inline LONG_PTR GetWindowLongPtrA(HWND hWnd, int index) {
  return GetWindowLongPtr(hWnd, index);
}
inline LONG_PTR GetWindowLongPtrW(HWND hWnd, int index) {
  return GetWindowLongPtr(hWnd, index);
}
inline LONG_PTR SetWindowLongPtrA(HWND hWnd, int index, LONG_PTR value) {
  return SetWindowLongPtr(hWnd, index, value);
}
inline LONG_PTR SetWindowLongPtrW(HWND hWnd, int index, LONG_PTR value) {
  return SetWindowLongPtr(hWnd, index, value);
}

//	mmsystem's tick: the same platform clock as GetTickCount.
#define PW153_TIMEGETTIME_IN_WINDOWS
inline DWORD timeGetTime() {
  return static_cast<DWORD>(pw153::platform::monotonic_microseconds() / 1000);
}

inline DWORD GetTickCount() {
  return static_cast<DWORD>(
      pw153::platform::monotonic_microseconds() / 1000);
}
inline DWORD GetTickCount64() { return GetTickCount(); }
inline ULARGE_INTEGER GetTickCount64Full() {
  ULARGE_INTEGER value;
  value.QuadPart = pw153::platform::monotonic_microseconds();
  return value;
}
inline void GetLocalTime(SYSTEMTIME *time) {
  const pw153::platform::Calendar calendar = pw153::platform::local_calendar();
  time->wYear = calendar.year;
  time->wMonth = calendar.month;
  time->wDay = calendar.day;
  time->wDayOfWeek = calendar.weekday;
  time->wHour = calendar.hour;
  time->wMinute = calendar.minute;
  time->wSecond = calendar.second;
  time->wMilliseconds = calendar.millisecond;
}
inline void GetSystemTime(SYSTEMTIME *time) { GetLocalTime(time); }
inline void Sleep(DWORD milliseconds) {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}


//	MessageBox: the diagnostic sink (console + stderr), the same mapping the
//	original debug-window logging received in the ALog seam.
inline int MessageBox(HWND window, const char *text, const char *caption,
                      unsigned type) {
  (void)window;
  (void)type;
  std::fprintf(stderr, "[%s] %s\n", caption ? caption : "MessageBox",
               text ? text : "");
  return IDOK;
}
inline int MessageBoxA(HWND window, const char *text, const char *caption,
                       unsigned type) {
  return MessageBox(window, text, caption, type);
}
inline int MessageBoxW(HWND window, const wchar_t *text,
                       const wchar_t *caption, unsigned type) {
  (void)window;
  (void)type;
  std::string narrowText, narrowCaption;
  if (text && pw153::platform::utf16_to_utf8(text, narrowText) && caption &&
      pw153::platform::utf16_to_utf8(caption, narrowCaption))
    std::fprintf(stderr, "[%s]\n%s\n", narrowCaption.c_str(),
                 narrowText.c_str());
  return IDOK;
}
inline int MessageBox(HWND window, const wchar_t *text,
                      const wchar_t *caption, unsigned type) {
  return MessageBoxW(window, text, caption, type);
}

//	Charset conversion: the strict UTF-8/UTF-16 platform conversions back
//	these; the code page argument stays unused (the port's narrow encoding
//	is UTF-8, documented divergence from the original CP936 builds).
inline int MultiByteToWideChar(unsigned codePage, DWORD flags,
                               const char *input, int inputLength,
                               wchar_t *output, int outputLength) {
  (void)codePage;
  (void)flags;
  if (!input)
    return 0;
  const std::size_t length =
      inputLength < 0 ? std::strlen(input) : static_cast<std::size_t>(inputLength);
  std::wstring wide;
  if (!pw153::platform::utf8_to_utf16(
          std::string_view(input, length), wide))
    return 0;
  if (output && outputLength > 0) {
    const std::size_t copy =
        std::min<std::size_t>(wide.size(), static_cast<std::size_t>(outputLength));
    std::memcpy(output, wide.data(), copy * sizeof(wchar_t));
    if (copy < static_cast<std::size_t>(outputLength))
      output[copy] = L'\0';
  }
  return static_cast<int>(wide.size());
}

inline int WideCharToMultiByte(unsigned codePage, DWORD flags,
                               const wchar_t *input, int inputLength,
                               char *output, int outputLength,
                               const char *defaultChar, BOOL *usedDefault) {
  (void)codePage;
  (void)flags;
  (void)defaultChar;
  if (usedDefault)
    *usedDefault = FALSE;
  if (!input)
    return 0;
  std::size_t length = 0;
  if (inputLength >= 0)
    length = static_cast<std::size_t>(inputLength);
  else
    while (input[length])
      ++length;
  std::string narrow;
  if (!pw153::platform::utf16_to_utf8(input, narrow))
    return 0;
  if (output && outputLength > 0) {
    const std::size_t copy =
        std::min<std::size_t>(narrow.size(), static_cast<std::size_t>(outputLength));
    std::memcpy(output, narrow.data(), copy);
    if (copy < static_cast<std::size_t>(outputLength))
      output[copy] = '\0';
  }
  return static_cast<int>(narrow.size());
}


//	Window messages, virtual keys and mouse flags: the real Win32 values.
#define WM_NULL 0x0000
#define WM_CREATE 0x0001
#define WM_DESTROY 0x0002
#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_ACTIVATE 0x0006
//	The activation states (wParam of WM_ACTIVATE).
#define WA_INACTIVE 0
#define WA_ACTIVE 1
#define WA_CLICKACTIVE 2
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_QUIT 0x0012
#define WM_SHOWWINDOW 0x0018
#define WM_ACTIVATEAPP 0x001C
#define WM_SETCURSOR 0x0020
#define WM_MOUSEACTIVATE 0x0021
#define WM_CHILDACTIVATE 0x0022
#define WM_GETMINMAXINFO 0x0024
#define WM_NCACTIVATE 0x0086
#define WM_NCHITTEST 0x0084
//	The hit-test codes (WM_NCHITTEST results).
#define HTERROR (-2)
#define HTTRANSPARENT (-1)
#define HTNOWHERE 0
#define HTCLIENT 1
#define HTCAPTION 2
#define HTSYSMENU 3
#define HTGROWBOX 4
#define HTSIZE HTGROWBOX
#define HTMENU 5
#define HTHSCROLL 6
#define HTVSCROLL 7
#define HTMINBUTTON 8
#define HTMAXBUTTON 9
#define HTLEFT 10
#define HTRIGHT 11
#define HTTOP 12
#define HTTOPLEFT 13
#define HTTOPRIGHT 14
#define HTBOTTOM 15
#define HTBOTTOMLEFT 16
#define HTBOTTOMRIGHT 17
#define HTBORDER 18
#define HTOBJECT 19
#define HTCLOSE 20
#define HTHELP 21
#define WM_NCMOUSEMOVE 0x00A0
#define WM_NCDESTROY 0x0082
#define WM_STYLECHANGE 0x007D
#define WM_INPUTLANGCHANGE 0x0051
#define WM_INPUTLANGCHANGEREQUEST 0x00FF
#define WM_IME_STARTCOMPOSITION 0x010D
#define WM_IME_ENDCOMPOSITION 0x010E
#define WM_IME_COMPOSITION 0x010F
#define WM_IME_SETCONTEXT 0x0281
#define WM_IME_NOTIFY 0x0282
#define WM_IME_CHAR 0x0286
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_SYSCHAR 0x0106
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define WM_KEYFIRST 0x0100
#define WM_KEYLAST 0x0108
#define WM_COMMAND 0x0111
#define WM_SYSCOMMAND 0x0112
//	The system commands (wParam of WM_SYSCOMMAND).
#define SC_SIZE 0xF000
#define SC_MOVE 0xF010
#define SC_MINIMIZE 0xF020
#define SC_MAXIMIZE 0xF030
#define SC_NEXTWINDOW 0xF040
#define SC_PREVWINDOW 0xF050
#define SC_CLOSE 0xF060
#define SC_VSCROLL 0xF070
#define SC_HSCROLL 0xF080
#define SC_MOUSEMENU 0xF090
#define SC_KEYMENU 0xF100
#define SC_ARRANGE 0xF110
#define SC_RESTORE 0xF120
#define SC_TASKLIST 0xF130
#define SC_SCREENSAVE 0xF140
#define SC_HOTKEY 0xF150
#define SC_DEFAULT 0xF160
#define SC_MONITORPOWER 0xF170
#define SC_CONTEXTHELP 0xF180
#define SC_SEPARATOR 0xF0F0
#define WM_TIMER 0x0113
#define WM_HSCROLL 0x0114
#define WM_VSCROLL 0x0115
#define WM_MOUSEFIRST 0x0200
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONUP 0x0208
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MOUSEWHEEL 0x020A
#define WM_MOUSELAST 0x020A
#define WM_PARENTNOTIFY 0x0210
#define WM_SIZING 0x0214
#define WM_USER 0x0400
//	The ATL message forwarding (ATL's own registered message).
#define WM_FORWARDMSG 0x037F

#define MK_LBUTTON 0x0001
#define MK_RBUTTON 0x0002
#define MK_SHIFT 0x0004
#define MK_CONTROL 0x0008
#define MK_MBUTTON 0x0010

//	The accessible object identifiers (the OBJID_* constants).
#define OBJID_WINDOW 0x00000000L
#define OBJID_SYSMENU 0xFFFFFFFFL
#define OBJID_TITLEBAR 0xFFFFFFFEL
#define OBJID_MENU 0xFFFFFFFDL
#define OBJID_VSCROLL 0xFFFFFFFCL
#define OBJID_HSCROLL 0xFFFFFFFBL
#define OBJID_SIZEGRIP 0xFFFFFFFAL
#define OBJID_CARET 0xFFFFFFF9L
#define OBJID_CURSOR 0xFFFFFFF8L
#define OBJID_ALERT 0xFFFFFFF7L
#define OBJID_SOUND 0xFFFFFFF6L

//	The window state queries: the browser target's single canvas window
//	is neither iconified nor zoomed, and there is no previous active
//	window to return.
inline BOOL IsIconic(HWND hWnd) {
  (void)hWnd;
  return FALSE;
}
inline BOOL IsZoomed(HWND hWnd) {
  (void)hWnd;
  return FALSE;
}
inline HWND SetActiveWindow(HWND hWnd) {
  (void)hWnd;
  return nullptr;
}
inline HWND GetActiveWindow() {
  return nullptr;
}

//	The window positioning flags and z-order handles (the documented
//	WinUser values).
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SWP_NOZORDER 0x0004
#define SWP_NOACTIVATE 0x0010
#define SWP_FRAMECHANGED 0x0020
#define SWP_SHOWWINDOW 0x0040
#define SWP_HIDEWINDOW 0x0080
#define SWP_ASYNCWINDOWPOS 0x4000
#define HWND_TOP ((HWND)0)
#define HWND_BOTTOM ((HWND)1)
#define HWND_TOPMOST ((HWND)-1)
#define HWND_NOTOPMOST ((HWND)-2)

//	The menu bar info: the browser target has no menu bars, so the query
//	fails (the Win32 result for a window without a menu).
typedef struct tagMENUBARINFO {
  DWORD cbSize;
  RECT rcBar;
  HMENU hMenu;
  HWND hwndMenu;
  BOOL fBarFocused;
  BOOL fFocused;
} MENUBARINFO, *PMENUBARINFO;

inline BOOL GetMenuBarInfo(HWND hWnd, LONG object, LONG item,
                           PMENUBARINFO info) {
  (void)hWnd;
  (void)object;
  (void)item;
  (void)info;
  return FALSE;
}

//	The message pump: the browser host owns the input dispatch and there
//	is no Win32 message queue, so GetMessage reports the pending quit
//	(the pump loop exits) and translate/dispatch are never reached.
inline BOOL GetMessage(MSG *message, HWND hWnd, UINT min, UINT max) {
  (void)hWnd;
  (void)min;
  (void)max;
  if (message)
    message->message = WM_QUIT;
  return 0;
}
inline BOOL TranslateMessage(const MSG *message) {
  (void)message;
  return FALSE;
}
inline LRESULT DispatchMessage(const MSG *message) {
  (void)message;
  return 0;
}

#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_CANCEL 0x03
#define VK_MBUTTON 0x04
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_CLEAR 0x0C
#define VK_RETURN 0x0D
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_APPS 0x5D
#define VK_JUNJA 0x17
#define VK_FINAL 0x18
#define VK_HANJA 0x19
#define VK_KANJI 0x19
#define VK_CONVERT 0x1C
#define VK_NONCONVERT 0x1D
#define VK_ACCEPT 0x1E
#define VK_MODECHANGE 0x1F
#define VK_PROCESSKEY 0xE5
#define VK_OEM_1 0xBA
#define VK_OEM_PLUS 0xBB
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2 0xBF
#define VK_OEM_3 0xC0
#define VK_OEM_4 0xDC
#define VK_OEM_5 0xDE
#define VK_OEM_6 0xDF
#define VK_OEM_7 0xE2
#define VK_OEM_8 0xE3
#define VK_F24 0x87
#define VK_PAUSE 0x13
#define VK_CAPITAL 0x14
#define VK_ESCAPE 0x1B
#define VK_SPACE 0x20
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_SELECT 0x29
#define VK_PRINT 0x2A
#define VK_EXECUTE 0x2B
#define VK_SNAPSHOT 0x2C
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_HELP 0x2F
#define VK_NUMPAD0 0x60
#define VK_NUMPAD9 0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91
#define VK_LSHIFT 0xA0
#define VK_LCONTROL 0xA2
#define VK_LMENU 0xA4
#define VK_RSHIFT 0xA1
#define VK_RCONTROL 0xA3
#define VK_RMENU 0xA5

//	The game uses bare min/max (the original windows.h macros); real
//	functions keep std::min/std::max usable in the same translation units.
inline int min(int a, int b) { return a < b ? a : b; }
inline int max(int a, int b) { return a > b ? a : b; }
inline long min(long a, long b) { return a < b ? a : b; }
inline long max(long a, long b) { return a > b ? a : b; }
inline unsigned long min(unsigned long a, unsigned long b) {
  return a < b ? a : b;
}
inline unsigned long max(unsigned long a, unsigned long b) {
  return a > b ? a : b;
}
inline unsigned int min(unsigned int a, unsigned int b) {
  return a < b ? a : b;
}
inline unsigned int max(unsigned int a, unsigned int b) {
  return a > b ? a : b;
}
inline float min(float a, float b) { return a < b ? a : b; }
inline float max(float a, float b) { return a > b ? a : b; }
inline double min(double a, double b) { return a < b ? a : b; }
inline double max(double a, double b) { return a > b ? a : b; }

//	The MSVC _snprintf: the documented truncating semantics over the real
//	std::snprintf (returns the would-be length, always nul-terminates).
inline int _snprintf(char *buffer, std::size_t size, const char *format, ...) {
  va_list args;
  va_start(args, format);
  const int written = std::vsnprintf(buffer, size, format, args);
  va_end(args);
  return written;
}

//	The wide CRT conversions are the port's domain under -fshort-wchar.
inline int _wtoi(const wchar_t *text) { return pw153::platform::wide_toi(text); }
inline std::int64_t _wtoi64(const wchar_t *text) { return pw153::platform::wide_toi64(text); }
inline wchar_t *_itow(int value, wchar_t *buffer, int radix) {
  return pw153::platform::wide_itow(value, buffer, radix);
}
inline wchar_t *_i64tow(long long value, wchar_t *buffer, int radix) {
  return pw153::platform::wide_i64tow(value, buffer, radix);
}

//	Virtual-key state: the browser input adapter populates this table with
//	real events; before any input arrives the honest state is "no key down".
inline short GetKeyState(int key) {
  extern unsigned char pw153_key_state[256];
  return static_cast<short>(pw153_key_state[key & 0xFF]);
}
inline short GetAsyncKeyState(int key) { return GetKeyState(key); }

#define ERROR_SUCCESS 0
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_PATH_NOT_FOUND 3
#define ERROR_ACCESS_DENIED 5
#define ERROR_INVALID_HANDLE 6

//	File creation/open: the documented Win32 surface over fopen (the
//	read/write/seek the AFile family maps through stdio already).
inline HANDLE CreateFileA(const char *name, DWORD access, DWORD share,
                          void *security, DWORD disposition, DWORD flags,
                          HANDLE templateFile) {
  (void)share;
  (void)security;
  (void)flags;
  (void)templateFile;
  const char *mode = (access & GENERIC_WRITE) ? (disposition == CREATE_ALWAYS ? "wb+" : "rb+") : "rb";
  FILE *stream = std::fopen(name, mode);
  return stream ? reinterpret_cast<HANDLE>(stream) : INVALID_HANDLE_VALUE;
}
inline HANDLE CreateFileW(const wchar_t *name, DWORD access, DWORD share,
                          void *security, DWORD disposition, DWORD flags,
                          HANDLE templateFile) {
  std::string narrow;
  if (!pw153::platform::utf16_to_utf8(name, narrow))
    return INVALID_HANDLE_VALUE;
  return CreateFileA(narrow.c_str(), access, share, security, disposition,
                     flags, templateFile);
}
//	The A/W-neutral name (the UNICODE build maps it to the wide form).
inline HANDLE CreateFile(const wchar_t *name, DWORD access, DWORD share,
                         void *security, DWORD disposition, DWORD flags,
                         HANDLE templateFile) {
  return CreateFileW(name, access, share, security, disposition, flags,
                     templateFile);
}

struct WIN32_FIND_DATAA {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime;
  DWORD nFileSizeHigh, nFileSizeLow, dwReserved0, dwReserved1;
  char cFileName[MAX_PATH];
  char cAlternateFileName[14];
};
HANDLE FindFirstFileA(const char *pattern, WIN32_FIND_DATAA *data);
BOOL FindNextFileA(HANDLE handle, WIN32_FIND_DATAA *data);
BOOL FindClose(HANDLE handle);

// DeleteFileA accepts the Windows path separators used by the client.
inline BOOL DeleteFileA(const char *path) {
  if (!path || !*path) return FALSE;
  std::string normalized(path);
  std::replace(normalized.begin(), normalized.end(), '\\', '/');
  return ::unlink(normalized.c_str()) == 0 ? TRUE : FALSE;
}

//	Directory removal: the real POSIX rmdir with the Win32 return
//	contract (fails when the directory is not empty).
inline BOOL RemoveDirectoryA(const char *path) {
  if (!path || !*path)
    return FALSE;
  std::string normalized(path);
  std::replace(normalized.begin(), normalized.end(), '\\', '/');
  return ::rmdir(normalized.c_str()) == 0 ? TRUE : FALSE;
}

//	Directory creation: the real POSIX mkdir with the Win32 return contract.
#include <sys/stat.h>
inline BOOL CreateDirectoryA(const char *path, void *attributes) {
  (void)attributes;
  if (!path || !path[0])
    return FALSE;
  if (::mkdir(path, 0777) == 0)
    return TRUE;
  return FALSE;
}

//	OutputDebugString: the diagnostic sink (console + stderr), the same
//	mapping the original debug-window logging received.
inline void OutputDebugStringA(const char *text) {
  if (text)
    std::fputs(text, stderr);
}
inline void OutputDebugStringW(const wchar_t *text) {
  if (!text)
    return;
  std::string narrow;
  if (pw153::platform::utf16_to_utf8(text, narrow))
    std::fputs(narrow.c_str(), stderr);
}
#ifdef UNICODE
#define OutputDebugString OutputDebugStringW
#else
#define OutputDebugString OutputDebugStringA
#endif

#define ZeroMemory(destination, length) memset((destination), 0, (length))
#define CopyMemory(destination, source, length) memcpy((destination), (source), (length))
#define FillMemory(destination, length, fill) memset((destination), (fill), (length))
#define MoveMemory(destination, source, length) memmove((destination), (source), (length))

#define CP_ACP 0
#define CP_OEMCP 1
#define CP_MACCP 2
#define CP_UTF7 65000
#define CP_UTF8 65001

typedef void VOID;
typedef void *LPVOID_ALIAS_UNUSED;

//	The PE image headers (the documented layouts; the API-hooking defence
//	code reads them - no real images exist on the browser target).
struct IMAGE_DOS_HEADER {
  WORD e_magic;
  WORD e_cblp;
  WORD e_cp;
  WORD e_crlc;
  WORD e_cparhdr;
  WORD e_minalloc;
  WORD e_maxalloc;
  WORD e_ss;
  WORD e_sp;
  WORD e_csum;
  WORD e_ip;
  WORD e_cs;
  WORD e_lfarlc;
  WORD e_ovno;
  WORD e_res[4];
  WORD e_oemid;
  WORD e_oeminfo;
  WORD e_res2[10];
  LONG e_lfanew;
};

typedef IMAGE_DOS_HEADER *PIMAGE_DOS_HEADER;

struct IMAGE_FILE_HEADER {
  WORD Machine;
  WORD NumberOfSections;
  DWORD TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD SizeOfOptionalHeader;
  WORD Characteristics;
};

struct IMAGE_OPTIONAL_HEADER32 {
  WORD Magic;
  BYTE MajorLinkerVersion;
  BYTE MinorLinkerVersion;
  DWORD SizeOfCode;
  DWORD SizeOfInitializedData;
  DWORD SizeOfUninitializedData;
  DWORD AddressOfEntryPoint;
  DWORD BaseOfCode;
  DWORD BaseOfData;
  DWORD ImageBase;
  DWORD SectionAlignment;
  DWORD FileAlignment;
  WORD MajorOperatingSystemVersion;
  WORD MinorOperatingSystemVersion;
  WORD MajorImageVersion;
  WORD MinorImageVersion;
  WORD MajorSubsystemVersion;
  WORD MinorSubsystemVersion;
  DWORD Win32VersionValue;
  DWORD SizeOfImage;
  DWORD SizeOfHeaders;
  DWORD CheckSum;
  WORD Subsystem;
  WORD DllCharacteristics;
  DWORD SizeOfStackReserve;
  DWORD SizeOfStackCommit;
  DWORD SizeOfHeapReserve;
  DWORD SizeOfHeapCommit;
  DWORD LoaderFlags;
  DWORD NumberOfRvaAndSizes;
};

struct IMAGE_NT_HEADERS32 {
  DWORD Signature;
  IMAGE_FILE_HEADER FileHeader;
  IMAGE_OPTIONAL_HEADER32 OptionalHeader;
};

typedef IMAGE_NT_HEADERS32 *PIMAGE_NT_HEADERS32;

//	The 32-bit alias (winnt.h names the 32-bit optional header
//	IMAGE_OPTIONAL_HEADER on non-64-bit targets) and the section record
//	the PE reader walks.
typedef IMAGE_OPTIONAL_HEADER32 IMAGE_OPTIONAL_HEADER;

struct IMAGE_SECTION_HEADER {
  BYTE Name[8];
  union {
    DWORD PhysicalAddress;
    DWORD VirtualSize;	//	the Misc union member the PE reader queries.
  } Misc;
  DWORD VirtualAddress;
  DWORD SizeOfRawData;
  DWORD PointerToRawData;
  DWORD PointerToRelocations;
  DWORD PointerToLinenumbers;
  WORD NumberOfRelocations;
  WORD NumberOfLinenumbers;
  DWORD Characteristics;
};

typedef IMAGE_SECTION_HEADER *PIMAGE_SECTION_HEADER;

//	The PE signature constants (the documented values: "MZ" and "PE\0\0").
#define IMAGE_DOS_SIGNATURE 0x5A4D
#define IMAGE_NT_SIGNATURE 0x00004550

inline HMODULE GetModuleHandle(const char *name) {
  (void)name;
  return nullptr;
}

//	Pointer validation: the WASM memory model has no guard pages like
//	Win32; the adapter's memory map supplies the real ranges. Until it
//	registers, the checks fail closed (treat every pointer as bad).
inline BOOL IsBadWritePtr(const void *pointer, std::size_t size) {
  (void)pointer;
  (void)size;
  return TRUE;
}
inline BOOL IsBadReadPtr(const void *pointer, std::size_t size) {
  (void)pointer;
  (void)size;
  return TRUE;
}

#include "winsync.h"	//	real event/semaphore/thread wait primitives (hosted last: needs the types above).
#include "eh.h"	//	SEH record layouts (EXCEPTION_RECORD/CONTEXT/EXCEPTION_POINTERS) for the minidump surface.

// ---------------------------------------------------------------------------
// IMM32 / clipboard / GDI-capture / caret / input-synthesis surface.
//
// HONEST BOUNDARY: the browser host owns the IME (the input adapter), the
// clipboard, the caret rendering and real keyboard synthesis; there is no
// IMM32 context, no Win32 clipboard, no GDI device contexts and no OS caret
// in the page. Every function here therefore fails explicitly instead of
// faking success: ImmGetContext returns NULL (the callers' null checks
// bail, e.g. AUIEditBox::OnImeComposition), the GDI capture functions
// return NULL/FALSE so the capture paths degrade to "no image", and the
// void input/caret calls are honest no-ops (the adapter owns the real
// state). Constants keep the documented Win32 values the sources compile
// against. This is the owning boundary for these APIs; it is not a copy of
// the SDK headers.
// ---------------------------------------------------------------------------

// IMM composition-string and notification constants (documented IMM values).
#define GCS_RESULTSTR 0x0800
#define GCS_COMPSTR 0x0008
#define GCS_COMPATTR 0x0010
#define GCS_CURSORPOS 0x0080
#define CFS_POINT 0x0002
#define CFS_RECT 0x0001
//	The short IMN_ aliases over the documented values (defined above).
#define IMN_OPENCANDIDATE IMN_OPENCANDIDATEWINDOW
#define IMN_CHANGECANDIDATE IMN_CHANGECANDIDATEWINDOW
#define IMN_CLOSECANDIDATE IMN_CLOSECANDIDATEWINDOW
#define IGP_PROPERTY 0x0004
#define CPS_COMPLETE 0x0001
#define IME_PROP_AT_CARET 0x00010000

//	The clipboard formats and SEH result (documented values).
#define CF_UNICODETEXT 0x000D
#define EXCEPTION_EXECUTE_HANDLER 1

//	The bitmap file header (the documented wingdi layout; the
//	screenshot writer emits the file container itself).
typedef struct tagBITMAPFILEHEADER {
  WORD bfType;
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER;

//	The GDI raster-op, bitmap constants and the system color index (documented).
#define SRCCOPY 0x00CC0020
#define BLACKNESS 0x00000042
#define WHITENESS 0x00FF0062
#define BI_RGB 0
#define DIB_RGB_COLORS 0
#define COLOR_WINDOW 5

//	The paint record and the paint boundary (the documented winuser
//	layout; the browser host owns the presentation, so the paint
//	boundary reports an empty region and succeeds - the WM_PAINT
//	handler's drawing is a no-op over it).
typedef struct tagPAINTSTRUCT {
  HDC hdc;
  BOOL fErase;
  RECT rcPaint;
  BOOL fRestore;
  BOOL fIncUpdate;
  BYTE rgbReserved[32];
} PAINTSTRUCT, *LPPAINTSTRUCT;

inline HDC BeginPaint(HWND hWnd, PAINTSTRUCT *paint) {
  (void)hWnd;
  if (!paint)
    return nullptr;
  paint->hdc = nullptr;
  paint->fErase = FALSE;
  paint->rcPaint = RECT{0, 0, 0, 0};
  return nullptr;
}
inline BOOL EndPaint(HWND hWnd, const PAINTSTRUCT *paint) {
  (void)hWnd;
  (void)paint;
  return TRUE;
}

//	The window sizing bounds (the documented winuser layout).
typedef struct tagMINMAXINFO {
  POINT ptReserved;
  POINT ptMaxSize;
  POINT ptMaxPosition;
  POINT ptMinTrackSize;
  POINT ptMaxTrackSize;
} MINMAXINFO, *LPMINMAXINFO;

//	The window sizing edges (the documented winuser values; WM_SIZING's
//	wParam).
#define WMSZ_BOTTOM 1
#define WMSZ_BOTTOMLEFT 2
#define WMSZ_BOTTOMRIGHT 3
#define WMSZ_LEFT 4
#define WMSZ_RIGHT 5
#define WMSZ_TOP 6
#define WMSZ_TOPLEFT 7
#define WMSZ_TOPRIGHT 8
#define DEFAULT_CHARSET 1

// Global memory flags (documented): GHND = moveable | zero-init, LPTR = fixed | zero-init.
#define GMEM_MOVEABLE 0x0002
#define GMEM_ZEROINIT 0x0040
#define GHND (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define LPTR GMEM_ZEROINIT

// Window-long indices and z-order constants (documented); GWL_EXSTYLE is
// defined with the window-long indices earlier in this seam.
#define GW_HWNDFIRST 0
#define GW_HWNDPREV 3

// Peek-message removal flag and keyboard-event flag (documented).
#define PM_NOREMOVE 0x0000
#define KEYEVENTF_KEYUP 0x0002

// A macro the SDK defines for unreferenced formal parameters.
#define UNREFERENCED_PARAMETER(P) (P)

typedef HANDLE HIMC;
typedef HANDLE HGLOBAL;

typedef struct tagRGBQUAD {
  BYTE rgbBlue;
  BYTE rgbGreen;
  BYTE rgbRed;
  BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAP {
  LONG bmType;
  LONG bmWidth;
  LONG bmHeight;
  LONG bmWidthBytes;
  WORD bmPlanes;
  WORD bmBitsPixel;
  LPVOID bmBits;
} BITMAP, *LPBITMAP;

typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO;

typedef struct tagCOMPOSITIONFORM {
  DWORD dwStyle;
  POINT ptCurrentPos;
  RECT rcArea;
} COMPOSITIONFORM, *LPCOMPOSITIONFORM;

typedef struct tagCANDIDATEFORM {
  DWORD dwIndex;
  DWORD dwStyle;
  POINT ptCurrentPos;
  RECT rcArea;
} CANDIDATEFORM, *LPCANDIDATEFORM;

// RGB extraction macros (documented: COLORREF is 0x00BBGGRR).
#define GetRValue(rgb) ((BYTE)(((DWORD_PTR)(rgb)) & 0xFF))
#define GetGValue(rgb) ((BYTE)((((DWORD_PTR)(rgb)) >> 8) & 0xFF))
#define GetBValue(rgb) ((BYTE)((((DWORD_PTR)(rgb)) >> 16) & 0xFF))

// IMM32: the browser has no IMM context; ImmGetContext returns NULL so the
// composition/candidate paths bail (the input adapter owns the IME).
inline HIMC ImmGetContext(HWND hWnd) { (void)hWnd; return NULL; }
inline BOOL ImmSetConversionStatus(HIMC hIMC, DWORD conversion, DWORD sentence) {
  (void)hIMC; (void)conversion; (void)sentence;
  return FALSE;	//	no IMM context on the browser target.
}
inline BOOL ImmSetOpenStatus(HIMC hIMC, BOOL open) {
  (void)hIMC; (void)open;
  return FALSE;	//	no IMM context on the browser target.
}
//	IME belongs to the input adapter, so the queries report none).
typedef struct tagCANDIDATELIST {
  DWORD dwSize;
  DWORD dwStyle;
  DWORD dwCount;
  DWORD dwSelection;
  DWORD dwPageStart;
  DWORD dwPageSize;
  DWORD dwOffset[1];
} CANDIDATELIST, *LPCANDIDATELIST;

inline DWORD ImmGetCandidateListA(HIMC context, DWORD index,
                                  LPCANDIDATELIST list, DWORD size) {
  (void)context;
  (void)index;
  (void)list;
  (void)size;
  return 0;	//	no candidate list on the browser target.
}
inline DWORD ImmGetCandidateListW(HIMC context, DWORD index,
                                  LPCANDIDATELIST list, DWORD size) {
  return ImmGetCandidateListA(context, index, list, size);
}
inline UINT ImmGetIMEFileNameA(HKL layout, LPSTR file, UINT buffer) {
  (void)layout; (void)file; (void)buffer;
  return 0;	//	no IME file on the browser target.
}
inline BOOL ImmSimulateHotKey(HWND window, DWORD hotKey) {
  (void)window; (void)hotKey;
  return FALSE;	//	no IMM context on the browser target.
}
inline UINT ImmGetVirtualKey(HIMC hIMC) {
  (void)hIMC;
  return 0;	//	no IMM context on the browser target.
}
inline HWND ImmGetDefaultIMEWnd(HWND hWnd) {
  (void)hWnd;
  return nullptr;	//	no IME window on the browser target.
}
inline BOOL ImmGetOpenStatus(HIMC hIMC) {
  (void)hIMC;
  return FALSE;	//	no IMM context on the browser target.
}
inline BOOL ImmSetCompositionString(HIMC hIMC, DWORD index, const void *composition,
                                     DWORD compositionLength, const void *read,
                                     DWORD readLength) {
  (void)hIMC; (void)index; (void)composition; (void)compositionLength;
  (void)read; (void)readLength;
  return FALSE;	//	no IMM context on the browser target.
}

//	The locale sorting and comparison constants (the documented WinNT
//	values).
#define SORT_DEFAULT 0
#define SORT_INVARIANT 0x73
#define NORM_IGNORECASE 0x00000001
#define NORM_IGNORENONSPACE 0x00000002
#define NORM_IGNORESYMBOLS 0x00000004
#define LCID_INVARIANT 0x0000007F
#define MAKELCID(langId, sortId) \
  ((((DWORD)(sortId)) << 16) | (DWORD)(langId))

//	The string comparison results (the documented CSTR_* values).
#define CSTR_LESS_THAN 1
#define CSTR_EQUAL 2
#define CSTR_GREATER_THAN 3

//	The string comparison: the real invariant-locale comparison (the
//	documented contract; ASCII case handling for NORM_IGNORECASE, a
//	negative length reads to the terminator).
inline int CompareStringA(DWORD locale, DWORD flags, const char *left,
                          int leftLength, const char *right,
                          int rightLength) {
  (void)locale;
  if (!left || !right)
    return 0;
  std::size_t i = 0;
  const bool ignoreCase = (flags & NORM_IGNORECASE) != 0;
  const std::size_t leftSize =
      leftLength < 0 ? std::strlen(left) : static_cast<std::size_t>(leftLength);
  const std::size_t rightSize =
      rightLength < 0 ? std::strlen(right)
                      : static_cast<std::size_t>(rightLength);
  while (i < leftSize && i < rightSize) {
    char a = left[i];
    char b = right[i];
    if (ignoreCase) {
      a = static_cast<char>(std::tolower(static_cast<unsigned char>(a)));
      b = static_cast<char>(std::tolower(static_cast<unsigned char>(b)));
    }
    if (a != b)
      return a < b ? CSTR_LESS_THAN : CSTR_GREATER_THAN;
    ++i;
  }
  if (i < leftSize)
    return CSTR_GREATER_THAN;
  if (i < rightSize)
    return CSTR_LESS_THAN;
  return CSTR_EQUAL;
}

//	The wide form (the documented CompareStringW contract; the same
//	invariant-locale code-point comparison).
inline int CompareStringW(DWORD locale, DWORD flags, const wchar_t *left,
                          int leftLength, const wchar_t *right,
                          int rightLength) {
  (void)locale;
  if (!left || !right)
    return 0;
  std::size_t i = 0;
  const std::size_t leftSize =
      leftLength < 0 ? std::wcslen(left)
                     : static_cast<std::size_t>(leftLength);
  const std::size_t rightSize =
      rightLength < 0 ? std::wcslen(right)
                      : static_cast<std::size_t>(rightLength);
  while (i < leftSize && i < rightSize) {
    wchar_t a = left[i];
    wchar_t b = right[i];
    if (a != b)
      return a < b ? CSTR_LESS_THAN : CSTR_GREATER_THAN;
    ++i;
  }
  if (i < leftSize)
    return CSTR_GREATER_THAN;
  if (i < rightSize)
    return CSTR_LESS_THAN;
  return CSTR_EQUAL;
}

//	The file version queries: no native DLLs exist on the browser
//	target, so the version surface reports none (the callers' size
//	checks bail).
inline DWORD GetFileVersionInfoSizeA(const char *file, DWORD *handle) {
  (void)file;
  if (handle)
    *handle = 0;
  return 0;
}
inline BOOL GetFileVersionInfoA(const char *file, DWORD handle, DWORD length,
                               void *data) {
  (void)file;
  (void)handle;
  (void)length;
  (void)data;
  return FALSE;
}
inline BOOL VerQueryValueA(const void *block, const char *query, void **buffer,
                           unsigned *length) {
  (void)block;
  (void)query;
  if (buffer)
    *buffer = nullptr;
  if (length)
    *length = 0;
  return FALSE;
}

//	The fixed version record (the documented ver.h layout; the queries
//	above report none on the browser target, the struct only shapes the
//	caller's reads).
typedef struct tagVS_FIXEDFILEINFO {
  DWORD dwSignature;
  DWORD dwStrucVersion;
  DWORD dwFileVersionMS;
  DWORD dwFileVersionLS;
  DWORD dwProductVersionMS;
  DWORD dwProductVersionLS;
  DWORD dwFileFlagsMask;
  DWORD dwFileFlags;
  DWORD dwFileOS;
  DWORD dwFileType;
  DWORD dwFileSubtype;
  DWORD dwFileDateMS;
  DWORD dwFileDateLS;
} VS_FIXEDFILEINFO;

//	The registry surface: the browser target has no Win32 registry, so
//	the queries fail explicitly (the callers' error paths handle it).
typedef void *HKEY;
#define HKEY_CLASSES_ROOT ((HKEY)0x80000000)
#define HKEY_CURRENT_USER ((HKEY)0x80000001)
#define HKEY_LOCAL_MACHINE ((HKEY)0x80000002)
#define HKEY_USERS ((HKEY)0x80000003)
#define KEY_READ 0x20019
#define KEY_WRITE 0x20006
#define KEY_ALL_ACCESS 0xF003F
#define REG_SZ 1
#define REG_BINARY 3
#define REG_DWORD 4

inline LONG RegOpenKeyExA(HKEY key, const char *subKey, DWORD options,
                          DWORD access, HKEY *result) {
  (void)key;
  (void)subKey;
  (void)options;
  (void)access;
  if (result)
    *result = nullptr;
  return 2;	//	ERROR_FILE_NOT_FOUND: no registry on the browser target.
}
inline LONG RegQueryValueExA(HKEY key, const char *value, DWORD *reserved,
                             DWORD *type, BYTE *data, DWORD *size) {
  (void)key;
  (void)value;
  (void)reserved;
  if (type)
    *type = 0;
  if (size)
    *size = 0;
  (void)data;
  return 2;	//	ERROR_FILE_NOT_FOUND.
}
inline LONG RegCloseKey(HKEY key) {
  (void)key;
  return 0;	//	ERROR_SUCCESS (nothing was opened).
}

//	The locale info query: the process locale is the single-byte C
//	locale (the port converts UTF-16/UTF-8 directly); the query
//	reports none (the callers' fallbacks handle it).
inline int GetLocaleInfoA(DWORD locale, DWORD type, char *buffer, int size) {
  (void)locale;
  (void)type;
  if (buffer && size > 0)
    buffer[0] = '\0';
  return 0;
}

//	The shell launch: the browser target opens URLs through the JS host
//	bridge (the network/JS-bridge cycle wires window.open); until it
//	registers, the launch fails explicitly (the callers' error paths
//	handle it).
inline HINSTANCE ShellExecuteA(HWND window, const char *operation,
                               const char *file, const char *parameters,
                               const char *directory, int show) {
  (void)window;
  (void)operation;
  (void)file;
  (void)parameters;
  (void)directory;
  (void)show;
  return reinterpret_cast<HINSTANCE>(0);	//	the documented failure (<32).
}
inline HINSTANCE ShellExecuteW(HWND window, const wchar_t *operation,
                               const wchar_t *file, const wchar_t *parameters,
                               const wchar_t *directory, int show) {
  (void)operation;
  (void)file;
  (void)parameters;
  (void)directory;
  return ShellExecuteA(window, nullptr, nullptr, nullptr, nullptr, show);
}
inline HINSTANCE ShellExecute(HWND window, const wchar_t *operation,
                              const wchar_t *file, const wchar_t *parameters,
                              const wchar_t *directory, int show) {
  return ShellExecuteW(window, operation, file, parameters, directory, show);
}

//	The shell execute record (the documented shellapi layout) and the
//	extended launcher (the same explicit failure as ShellExecute).
typedef struct _SHELLEXECUTEINFOA {
  DWORD cbSize;
  ULONG fMask;
  HWND hwnd;
  LPCSTR lpVerb;
  LPCSTR lpFile;
  LPCSTR lpParameters;
  LPCSTR lpDirectory;
  int nShow;
  HINSTANCE hInstApp;
  void *lpIDList;
  LPCSTR lpClass;
  void *hkeyClass;
  DWORD dwHotKey;
  void *hIcon;
  HANDLE hProcess;
} SHELLEXECUTEINFOA, *LPSHELLEXECUTEINFOA;

typedef struct _SHELLEXECUTEINFOW {
  DWORD cbSize;
  ULONG fMask;
  HWND hwnd;
  LPCWSTR lpVerb;
  LPCWSTR lpFile;
  LPCWSTR lpParameters;
  LPCWSTR lpDirectory;
  int nShow;
  HINSTANCE hInstApp;
  void *lpIDList;
  LPCWSTR lpClass;
  void *hkeyClass;
  DWORD dwHotKey;
  void *hIcon;
  HANDLE hProcess;
} SHELLEXECUTEINFOW, *LPSHELLEXECUTEINFOW;
typedef SHELLEXECUTEINFOW SHELLEXECUTEINFO;
typedef SHELLEXECUTEINFOW *LPSHELLEXECUTEINFO;

inline BOOL ShellExecuteExA(SHELLEXECUTEINFOA *info) {
  (void)info;
  return FALSE;	//	the JS host bridge owns window.open.
}
inline BOOL ShellExecuteExW(SHELLEXECUTEINFOW *info) {
  (void)info;
  return FALSE;
}
inline BOOL ShellExecuteEx(SHELLEXECUTEINFOW *info) {
  return ShellExecuteExW(info);
}

//	The shell folder selection: no shell exists on the browser target;
//	the call fails explicitly.
inline HRESULT SHOpenFolderAndSelectItems(void *itemList, UINT count,
                                          void *items, DWORD flags) {
  (void)itemList;
  (void)count;
  (void)items;
  (void)flags;
  return E_FAIL;
}

//	The startup info flag (the documented WinBase value).
#define STARTF_USESHOWWINDOW 0x00000001

//	The legacy local-memory handle (the documented Win32 alias).
typedef HANDLE HLOCAL;
typedef HANDLE HGLOBAL;
#define LMEM_ZEROINIT 0x0040

//	The volume information: no native volume exists on the browser
//	target; the query fails (the hardware-id code's fallbacks handle
//	it).
inline BOOL GetVolumeInformationA(const char *root, char *name,
                                   DWORD nameSize, DWORD *serial,
                                   DWORD *maxLength, DWORD *flags,
                                   char *fsName, DWORD fsNameSize) {
  (void)root;
  (void)name;
  (void)nameSize;
  if (serial)
    *serial = 0;
  if (maxLength)
    *maxLength = 0;
  if (flags)
    *flags = 0;
  (void)fsName;
  (void)fsNameSize;
  return FALSE;
}

//	The legacy OpenFile existence probe (the documented OF_* semantics:
//	OF_EXIST opens and closes, reporting existence through the handle).
typedef int HFILE;
#define HFILE_ERROR ((HFILE)-1)
#define OF_READ 0x0000
#define OF_SHARE_EXCLUSIVE 0x0010
#define OF_EXIST 0x4000
typedef struct _OFSTRUCT {
  BYTE cBytes;
  BYTE fFixedDisc;
  WORD nErrCode;
  WORD Reserved1;
  WORD Reserved2;
  char szPathName[128];
} OFSTRUCT, *LPOFSTRUCT;

inline HFILE OpenFile(const char *name, OFSTRUCT *info, UINT action) {
  if (!name)
    return HFILE_ERROR;
  std::string normalized(name);
  std::replace(normalized.begin(), normalized.end(), '\\', '/');
  std::FILE *probe = std::fopen(normalized.c_str(), "rb");
  if (!probe)
    return HFILE_ERROR;
  std::fclose(probe);
  if (info) {
    std::memset(info, 0, sizeof(*info));
    info->cBytes = sizeof(*info);
    std::strncpy(info->szPathName, normalized.c_str(),
                 sizeof(info->szPathName) - 1);
  }
  //	OF_EXIST closes the handle after probing (the documented
  //	behavior); a real read handle is not needed by the callers.
  if (action & OF_EXIST)
    return 1;
  return HFILE_ERROR;
}

//	No more find matches (the documented WinBase value).
#define ERROR_NO_MORE_FILES 18

//	Enum-device-interfaces reached the end of the list (the documented
//	setupapi value; UsbHidKey.h's enumeration loop stops on it).
#define ERROR_NO_MORE_ITEMS 259

//	The setupapi detail-data buffer is too small (the documented WinBase
//	value; the enumeration loop grows the buffer on it).
#define ERROR_INSUFFICIENT_BUFFER 122

//	The flexible-array member size of the setupapi detail-data structs
//	(the documented WinBase value of 1).
#define ANYSIZE_ARRAY 1

//	The disk free space: the real statvfs numbers over the adapter's
//	filesystem (the documented Win32 out-parameters).
inline BOOL GetDiskFreeSpaceA(const char *root, DWORD *sectorsPerCluster,
                               DWORD *bytesPerSector, DWORD *freeClusters,
                               DWORD *totalClusters) {
  if (!root || !*root)
    return FALSE;
  std::string normalized(root);
  std::replace(normalized.begin(), normalized.end(), '\\', '/');
  struct statvfs disk {};
  if (statvfs(normalized.c_str(), &disk) != 0)
    return FALSE;
  if (sectorsPerCluster)
    *sectorsPerCluster = static_cast<DWORD>(disk.f_frsize ? disk.f_bsize / disk.f_frsize : 1);
  if (bytesPerSector)
    *bytesPerSector = static_cast<DWORD>(disk.f_frsize ? disk.f_frsize : disk.f_bsize);
  if (freeClusters)
    *freeClusters = static_cast<DWORD>(disk.f_bavail);
  if (totalClusters)
    *totalClusters = static_cast<DWORD>(disk.f_blocks);
  return TRUE;
}
inline BOOL ImmReleaseContext(HWND hWnd, HIMC hIMC) { (void)hWnd; (void)hIMC; return TRUE; }
inline BOOL ImmIsIME(HKL hKL) { (void)hKL; return FALSE; }

//	The IME candidate list (the documented imm.h layout; the browser


inline DWORD ImmGetCandidateList(HIMC context, DWORD index,
                                 LPCANDIDATELIST list, DWORD size) {
  (void)context;
  (void)index;
  (void)list;
  (void)size;
  return 0;	//	no candidate list on the browser target.
}
inline LONG ImmGetCompositionStringW(HIMC hIMC, DWORD index, LPVOID buffer, DWORD bufLen) {
  (void)hIMC; (void)index; (void)buffer; (void)bufLen;
  return -1;
}
inline LONG ImmGetCompositionString(HIMC hIMC, DWORD index, LPVOID buffer,
                                     DWORD bufLen) {
  return ImmGetCompositionStringW(hIMC, index, buffer, bufLen);
}
inline LONG ImmGetCompositionStringA(HIMC hIMC, DWORD index, LPVOID buffer,
                                      DWORD bufLen) {
  return ImmGetCompositionStringW(hIMC, index, buffer, bufLen);
}
inline HKL GetKeyboardLayout(DWORD idThread) { (void)idThread; return NULL; }
inline HIMC ImmAssociateContext(HWND hWnd, HIMC hIMC) { (void)hWnd; (void)hIMC; return NULL; }
inline BOOL ImmSetCompositionWindow(HIMC hIMC, LPCOMPOSITIONFORM pComp) { (void)hIMC; (void)pComp; return FALSE; }
inline BOOL ImmSetCandidateWindow(HIMC hIMC, LPCANDIDATEFORM pCand) { (void)hIMC; (void)pCand; return FALSE; }
inline BOOL ImmNotifyIME(HIMC hIMC, DWORD action, DWORD index, DWORD value) {
  (void)hIMC; (void)action; (void)index; (void)value;
  return FALSE;
}
inline DWORD ImmGetProperty(HKL hKL, DWORD index) { (void)hKL; (void)index; return 0; }
inline UINT ImmGetDescriptionW(HKL hKL, LPWSTR buffer, UINT bufLen) { (void)hKL; (void)buffer; (void)bufLen; return 0; }
inline BOOL ImmGetConversionStatus(HIMC hIMC, LPDWORD pConv, LPDWORD pSent) {
  (void)hIMC; (void)pConv; (void)pSent;
  return FALSE;
}

// Clipboard: the host owns the clipboard; the page cannot grab it
// synchronously, so the open/transfer calls fail (the callers degrade to
// "no clipboard data").
inline BOOL OpenClipboard(HWND hWnd) { (void)hWnd; return FALSE; }
inline BOOL CloseClipboard() { return FALSE; }
inline BOOL EmptyClipboard() { return FALSE; }
inline HANDLE GetClipboardData(UINT format) { (void)format; return NULL; }
inline HANDLE SetClipboardData(UINT format, HANDLE data) { (void)format; (void)data; return NULL; }

// Caret: the AUI renders its own caret in the browser; the OS caret calls
// are honest no-ops (the adapter owns the real caret, like SetCapture).
inline BOOL CreateCaret(HWND hWnd, HBITMAP hBitmap, int nWidth, int nHeight) {
  (void)hWnd; (void)hBitmap; (void)nWidth; (void)nHeight;
  return TRUE;
}
inline BOOL SetCaretPos(int x, int y) { (void)x; (void)y; return TRUE; }

// GDI capture: the browser has no GDI device contexts; the capture functions
// return NULL/FALSE so the screenshot paths bail (AUIWindowPicture checks
// the DC and returns early; CaptureWindow's BitBlt is a no-op over NULL).
inline HDC GetWindowDC(HWND hWnd) { (void)hWnd; return NULL; }
inline int ReleaseDC(HWND hWnd, HDC hDC) { (void)hWnd; (void)hDC; return 0; }

//	The desktop window: the browser host owns the single canvas; there is
//	no desktop window and the handle is none (the font-enumeration callers
//	fail on the NULL DC, like GetDC above).
inline HWND GetDesktopWindow(void) { return NULL; }

//	The screen device context and capability probe: no GDI contexts
//	exist on the browser target (the canvas adapter owns the real
//	metrics, like GetSystemMetrics); the context is none and the
//	capability reports zero (the documented unknown-capability value).
#define BITSPIXEL 12
#define VREFRESH 116
inline HDC GetDC(HWND hWnd) { (void)hWnd; return NULL; }
inline int GetDeviceCaps(HDC hDC, int index) {
  (void)hDC;
  (void)index;
  return 0;
}
inline HDC CreateCompatibleDC(HDC hDC) { (void)hDC; return NULL; }
inline HBITMAP CreateCompatibleBitmap(HDC hDC, int width, int height) {
  (void)hDC; (void)width; (void)height;
  return NULL;
}
inline HGDIOBJ SelectObject(HDC hDC, HGDIOBJ object) { (void)hDC; (void)object; return NULL; }
inline BOOL DeleteObject(HGDIOBJ object) { (void)object; return FALSE; }
inline BOOL BitBlt(HDC dest, int x, int y, int width, int height,
                   HDC src, int x1, int y1, DWORD rop) {
  (void)dest; (void)x; (void)y; (void)width; (void)height;
  (void)src; (void)x1; (void)y1; (void)rop;
  return FALSE;
}
inline int GetObject(HGDIOBJ object, int count, LPVOID buffer) {
  (void)object; (void)count; (void)buffer;
  return 0;
}
inline int GetDIBits(HDC hDC, HBITMAP hBitmap, UINT start, UINT lines,
                     LPVOID bits, LPBITMAPINFO info, UINT usage) {
  (void)hDC; (void)hBitmap; (void)start; (void)lines;
  (void)bits; (void)info; (void)usage;
  return 0;
}

//	The GDI font/text-metric surface: no GDI target exists on the
//	browser target (the browser IME adapter owns the real font
//	selection). Font creation and the DIB section report none, the
//	mapping-mode change reports failure and the extent probe zeroes
//	its record and reports none - the font manager's Init fails on
//	the memory DC and its callers' error paths handle it.
#define MM_TEXT 1
#define FW_NORMAL 400
#define FW_BOLD 700
#define OUT_DEFAULT_PRECIS 0
#define CLIP_DEFAULT_PRECIS 0
#define ANTIALIASED_QUALITY 4
#define VARIABLE_PITCH 2
inline HFONT CreateFontW(int height, int width, int escapement,
                         int orientation, DWORD weight, DWORD italic,
                         DWORD underline, DWORD strikeOut, DWORD charset,
                         DWORD outPrecision, DWORD clipPrecision,
                         DWORD quality, DWORD pitchAndFamily,
                         const wchar_t *faceName) {
  (void)height; (void)width; (void)escapement; (void)orientation;
  (void)weight; (void)italic; (void)underline; (void)strikeOut;
  (void)charset; (void)outPrecision; (void)clipPrecision; (void)quality;
  (void)pitchAndFamily; (void)faceName;
  return NULL;
}
//	The UNICODE build maps the unprefixed text GDI to the W forms.
inline HFONT CreateFont(int height, int width, int escapement,
                        int orientation, DWORD weight, DWORD italic,
                        DWORD underline, DWORD strikeOut, DWORD charset,
                        DWORD outPrecision, DWORD clipPrecision,
                        DWORD quality, DWORD pitchAndFamily,
                        const wchar_t *faceName) {
  return CreateFontW(height, width, escapement, orientation, weight, italic,
                     underline, strikeOut, charset, outPrecision,
                     clipPrecision, quality, pitchAndFamily, faceName);
}
inline HBITMAP CreateDIBSection(HDC hDC, const BITMAPINFO *info, UINT usage,
                                void **bits, HANDLE section, DWORD offset) {
  (void)hDC; (void)info; (void)usage; (void)section; (void)offset;
  if (bits)
    *bits = NULL;
  return NULL;
}
inline int SetMapMode(HDC hDC, int mode) {
  (void)hDC;
  (void)mode;
  return 0;
}
inline BOOL GetTextExtentPoint32W(HDC hDC, const wchar_t *text, int count,
                                  SIZE *size) {
  (void)hDC; (void)text; (void)count;
  if (size) {
    size->cx = 0;
    size->cy = 0;
  }
  return FALSE;
}
inline BOOL GetTextExtentPoint32(HDC hDC, const wchar_t *text, int count,
                                 SIZE *size) {
  return GetTextExtentPoint32W(hDC, text, count, size);
}
inline BOOL DeleteDC(HDC hDC) {
  (void)hDC;
  return TRUE;	//	no real DC objects exist (the creator fails first).
}
inline COLORREF GetSysColor(int index) { (void)index; return 0; }

// Global memory: the heap is the C library; the GDI capture never reaches
// these (GetWindowDC fails first), but they fail explicitly for any caller.
inline HGLOBAL GlobalAlloc(UINT flags, SIZE_T bytes) { (void)flags; (void)bytes; return NULL; }
inline LPVOID GlobalLock(HGLOBAL memory) { (void)memory; return NULL; }
inline BOOL GlobalUnlock(HGLOBAL memory) { (void)memory; return FALSE; }
inline HGLOBAL GlobalFree(HGLOBAL memory) { (void)memory; return NULL; }

// The Windows directory: the browser target has no Windows directory; the
// font scan falls through to the game's configured fonts (the callers'
// _findfirst failure path returns early).
inline UINT GetWindowsDirectoryA(LPSTR buffer, UINT size) {
  if (buffer && size) buffer[0] = '\0';
  return 0;
}

// Input synthesis: the input adapter owns the real keyboard; synthesized OS
// key events have no target, so keybd_event is an honest no-op.
inline void keybd_event(BYTE bVk, BYTE bScan, DWORD dwFlags, ULONG_PTR dwExtraInfo) {
  (void)bVk; (void)bScan; (void)dwFlags; (void)dwExtraInfo;
}

// The single hosted window has no siblings; GetWindow reports none.
inline HWND GetWindow(HWND hWnd, UINT cmd) { (void)hWnd; (void)cmd; return NULL; }

// The W/A message-pump aliases forward to the charset-neutral seam
// implementations above (the host owns the input dispatch).
inline BOOL PeekMessageW(LPMSG message, HWND hWnd, UINT min, UINT max, UINT remove) {
  (void)message; (void)hWnd; (void)min; (void)max; (void)remove;
  return FALSE;
}
inline BOOL PeekMessage(MSG *message, HWND hWnd, UINT min, UINT max, UINT remove) {
  return PeekMessageW(message, hWnd, min, max, remove);
}
inline BOOL PeekMessageA(MSG *message, HWND hWnd, UINT min, UINT max, UINT remove) {
  return PeekMessageW(message, hWnd, min, max, remove);
}
//	The peek modes (the documented WinUser values).
#define PM_NOREMOVE 0x0000
#define PM_REMOVE 0x0001
#define PM_NOYIELD 0x0002
inline BOOL GetMessageW(LPMSG message, HWND hWnd, UINT min, UINT max) {
  return GetMessage(message, hWnd, min, max);
}
inline BOOL GetMessageA(MSG *message, HWND hWnd, UINT min, UINT max) {
  return GetMessage(message, hWnd, min, max);
}
inline LRESULT DispatchMessageA(const MSG *message) { return DispatchMessage(message); }

// The GUI-thread info query: the browser host owns the caret; there is no
// per-thread GUI info, so the query fails (hwndCaret stays NULL and the
// callers bail, e.g. CWndThreadManager::GetCaretWindowPos).
inline BOOL GetGUIThreadInfo(DWORD idThread, GUITHREADINFO *info) {
  (void)idThread;
  if (info)
    info->hwndCaret = NULL;
  return FALSE;
}
// The window is hosted as Unicode (UTF-16) by the seam; the A/W message
// dispatch the original code branches on is neutral here.
inline BOOL IsWindowUnicode(HWND hWnd) { (void)hWnd; return TRUE; }
// No owned windows to enumerate in the single-window host; the callback is
// never invoked and the call reports failure (the refresh loop skips).
inline BOOL EnumThreadWindows(DWORD idThread, WNDENUMPROC lpEnumFunc, LPARAM lParam) {
  (void)idThread; (void)lpEnumFunc; (void)lParam;
  return FALSE;
}
// Local memory mirrors the global heap: the C library backs it; the calls
// fail explicitly (the clipboard path that uses them is dead - OpenClipboard
// fails first).
inline HANDLE LocalAlloc(UINT flags, SIZE_T bytes) { (void)flags; (void)bytes; return NULL; }
inline HANDLE LocalFree(HANDLE memory) { (void)memory; return NULL; }

#ifndef ERROR_ALREADY_EXISTS
#define ERROR_ALREADY_EXISTS 183
#endif

#endif // PW153_PLATFORM_WINDOWS_H

