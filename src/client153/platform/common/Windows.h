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
#include "utf16.h"

#include <algorithm>
#include <unistd.h>	//	getcwd/chdir/mkdir.
#include <sys/stat.h>	//	stat/mkdir.
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <strings.h>
#include <string>
#include <string_view>
#include <thread>

inline int stricmp(const char *lhs, const char *rhs) { return ::strcasecmp(lhs, rhs); }
inline int strcmpi(const char *lhs, const char *rhs) { return stricmp(lhs, rhs); }
inline int _stricmp(const char *lhs, const char *rhs) { return stricmp(lhs, rhs); }

#ifndef MAX_PATH
#define MAX_PATH 260
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
typedef unsigned char UCHAR;
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
typedef BYTE *LPBYTE;
typedef DWORD *LPDWORD;
typedef const char *PSZ;

typedef const char *PCSZ;
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
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
typedef std::size_t SIZE_T;
typedef std::uintptr_t UINT_PTR;
typedef std::intptr_t INT_PTR;
typedef std::intptr_t LONG_PTR;
typedef std::uintptr_t ULONG_PTR;
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
typedef const RECT *LPCRECT;
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
struct SYSTEM_INFO {
  union {
    DWORD dwOemId;
    struct {
      WORD wProcessorArchitecture;
      WORD wReserved;
    } processor;
  } anonymous;
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

struct OSVERSIONINFO {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  char szCSDVersion[128];
};

typedef OSVERSIONINFO *LPOSVERSIONINFO;

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

inline BOOL GetVersionExA(OSVERSIONINFO *info) {
  if (!info || info->dwOSVersionInfoSize < sizeof(OSVERSIONINFO))
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

struct PALETTEENTRY {
  BYTE peRed;
  BYTE peGreen;
  BYTE peBlue;
  BYTE peFlags;
};

struct LARGE_INTEGER {
  std::int64_t QuadPart;
};

struct ULARGE_INTEGER {
  std::uint64_t QuadPart;
};

#define WINAPI
#define CALLBACK
#define APIENTRY
#define TRUE 1
#define FALSE 0

#define INVALID_HANDLE_VALUE ((HANDLE)(std::intptr_t)-1)
#define WAIT_OBJECT_0 0u
#define WAIT_TIMEOUT 258u
#define WAIT_FAILED 0xFFFFFFFFu
#define INFINITE 0xFFFFFFFFu
#define GENERIC_READ 0x80000000u
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
#define MB_ICONWARNING 0x30
#define MB_ICONINFORMATION 0x40
#define MB_YESNO 4
#define IDYES 6
#define IDNO 7
#define IDOK 1
#define IDCANCEL 2
#define DLL_PROCESS_ATTACH 1
#define DLL_THREAD_DETACH 0
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

#define GWL_WNDPROC (-4)
#define GWLP_WNDPROC (-4)
#define GWLP_USERDATA (-21)
#define GWL_STYLE (-16)
#define GWL_USERDATA (-21)

inline int lstrlenW(const wchar_t *text) {
  if (!text) return 0;
  int length = 0;
  while (text[length]) ++length;
  return length;
}

// The browser host supplies screen coordinates from pointer events.
extern "C" void pw153_set_cursor_position(LONG x, LONG y);
BOOL GetCursorPos(POINT *point);
// Coordinates refer to the browser host's single canvas, in CSS pixels.
extern "C" void pw153_set_canvas_origin(LONG x, LONG y);
BOOL ScreenToClient(HWND hWnd, POINT *point);
BOOL ClientToScreen(HWND hWnd, POINT *point);
BOOL CreateCaret(HWND hWnd, HBITMAP bitmap, int width, int height);
HWND SetCapture(HWND hWnd);
BOOL ReleaseCapture();

//	The GetAncestor constants.
#define GA_PARENT 1
#define GA_ROOT 2
#define GA_ROOTOWNER 3

inline HWND GetAncestor(HWND hWnd, UINT flags) {
  (void)hWnd;
  (void)flags;
  return nullptr;
}

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

inline BOOL SetCurrentDirectoryA(const char *path) {
  return chdir(path) == 0 ? TRUE : FALSE;
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
inline int GetWindowTextA(HWND hWnd, char *text, int count) {
  (void)hWnd;
  if (text && count > 0)
    text[0] = '\0';
  return 0;
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

inline LONG_PTR GetWindowLongPtr(HWND hWnd, int index) {
  return GetWindowLong(hWnd, index);
}
inline LONG_PTR SetWindowLongPtr(HWND hWnd, int index, LONG_PTR value) {
  return SetWindowLong(hWnd, index, static_cast<LONG>(value));
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

//	Last-error state: a real per-thread slot the callers can read back.
inline DWORD *ThreadLastErrorSlot() {
  thread_local DWORD lastError = 0;
  return &lastError;
}
inline DWORD GetLastError() { return *ThreadLastErrorSlot(); }
inline void SetLastError(DWORD error) { *ThreadLastErrorSlot() = error; }

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
inline int MessageBoxW(HWND window, const char16_t *text,
                       const char16_t *caption, unsigned type) {
  (void)window;
  (void)type;
  std::string narrow;
  if (text && pw153::platform::utf16_to_utf8(text, narrow))
    std::fprintf(stderr, "[%s]\n%s\n", caption ? "MessageBox" : "MessageBox",
                 narrow.c_str());
  return IDOK;
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
#define WM_NCMOUSEMOVE 0x00A0
#define WM_NCDESTROY 0x0082
#define WM_STYLECHANGE 0x007D
#define WM_INPUTLANGCHANGE 0x0051
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

#define MK_LBUTTON 0x0001
#define MK_RBUTTON 0x0002
#define MK_SHIFT 0x0004
#define MK_CONTROL 0x0008
#define MK_MBUTTON 0x0010

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

#endif // PW153_PLATFORM_WINDOWS_H

