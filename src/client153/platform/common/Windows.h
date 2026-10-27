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
#include "utf16.h"

#include <algorithm>
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

struct MSG {
  HWND hwnd;
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
  DWORD time;
  POINT pt;
};

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
#define OPEN_EXISTING 3u
#define CREATE_ALWAYS 2u
#define FILE_ATTRIBUTE_NORMAL 0x80u
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFFu
#define FORMAT_MESSAGE_FROM_SYSTEM 0x1000u
#define MB_OK 0
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

#define CP_ACP 0
#define CP_OEMCP 1
#define CP_MACCP 2
#define CP_UTF7 65000
#define CP_UTF8 65001

typedef void VOID;
typedef void *LPVOID_ALIAS_UNUSED;

#include "winsync.h"	//	real event/semaphore/thread wait primitives (hosted last: needs the types above).

#endif // PW153_PLATFORM_WINDOWS_H

