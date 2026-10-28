/*
 * FILE: atlwin.h (platform seam)
 *
 * DESCRIPTION: The ATL windowing surface the browser-dialog wrapper uses
 * (CWindow, the message-map macros and CWindowImpl through SafeWinImpl.h).
 * The windowing types carry the documented members; the browser-dialog
 * adapter (HTML overlay in the UI cycle) supplies the runtime - the message
 * pump maps to the browser event loop.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_ATLWIN_H
#define PW153_PLATFORM_ATLWIN_H

#include "Windows.h"

//	The ATL version gate SafeWinImpl.h checks.
#define _ATL_VER 0x0800

//\tThe ATL assertion macro (the documented assert spelling; NDEBUG in the
//\tgame configuration makes it a no-op).
#include <cassert>
#define ATLASSERT(expr) assert(expr)

//	The ATL window state flags.
#define WINSTATE_DESTROYED 0x00000002

//	The windowing result and string types (the documented Win32 values).
#ifndef LRESULT
#define LRESULT LONG_PTR
#endif
#ifndef LPRESULT
typedef LRESULT *LPRESULT;
#endif
#ifndef LPCTSTR
#define LPCTSTR const wchar_t *
#endif
//	The WS_/WS_EX_ window styles live in Windows.h (the umbrella owns
//	the Win32 surface; this header includes it).

class CWindow {
public:
  CWindow() : m_hWnd(nullptr) {}
  CWindow(HWND hWnd) : m_hWnd(hWnd) {}
  operator HWND() const { return m_hWnd; }
  BOOL IsWindow() const { return ::IsWindow(m_hWnd); }
  BOOL DestroyWindow() { return ::DestroyWindow(m_hWnd); }
  LONG GetWindowLong(int index) const { return ::GetWindowLong(m_hWnd, index); }
  LONG SetWindowLong(int index, LONG value) {
    return ::SetWindowLong(m_hWnd, index, value);
  }
  BOOL SetWindowPos(HWND after, LPCRECT bounds, UINT flags) {
    return bounds ? ::SetWindowPos(m_hWnd, after, bounds->left, bounds->top,
                                   bounds->right - bounds->left,
                                   bounds->bottom - bounds->top, flags)
                  : FALSE;
  }
  UINT_PTR SetTimer(UINT_PTR id, UINT elapse) {
    return ::SetTimer(m_hWnd, id, elapse, nullptr);
  }
  BOOL KillTimer(UINT_PTR id) { return ::KillTimer(m_hWnd, id); }
  //\tThe remaining CWindow members the browser wrapper drives (the
  //\tdocumented ATL signatures over the Win32 seam functions).
  int MessageBox(LPCTSTR text, LPCTSTR caption = nullptr, UINT type = MB_OK) {
    return ::MessageBox(m_hWnd, text, caption, type);
  }
  BOOL ShowWindow(int command) { return ::ShowWindow(m_hWnd, command); }
  BOOL MoveWindow(LPCRECT bounds, BOOL repaint) {
    return bounds ? ::MoveWindow(m_hWnd, bounds->left, bounds->top,
                                 bounds->right - bounds->left,
                                 bounds->bottom - bounds->top, repaint)
                  : FALSE;
  }
  HWND m_hWnd;
};

//\tThe message-map macros: the original expanded to ATL's message map
//\tmachinery — a ProcessWindowMessage override of the CWindowImpl pure
//\tvirtual (the entry SafeWinImpl's window procedure dispatches through);
//\tthe adapter's dispatch consumes the same entries.
#define BEGIN_MSG_MAP(theClass) \
public: \
  BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT &result, DWORD dwMsgMapID) { \
    (void)hWnd; (void)dwMsgMapID; (void)result; \
    BOOL handled = TRUE;
#define MESSAGE_HANDLER(msg, func) \
    if (uMsg == (msg)) { handled = TRUE; result = func(uMsg, wParam, lParam, handled); if (handled) return TRUE; }
#define END_MSG_MAP() \
    return FALSE; \
  }

#define DECLARE_WND_CLASS(className) \
  static LPCTSTR GetWndClassName() { return className; }

//	WNDPROC lives in Windows.h (the umbrella owns the Win32 surface).

//\tThe ATL control traits (the documented defaults).
class CControlWinTraits {
public:
  static DWORD GetWndStyle(DWORD style) { return style | WS_CHILD | WS_VISIBLE; }
  static DWORD GetWndExStyle(DWORD exStyle) { return exStyle; }
};

//\t_ATL_MSG: the current-message record SafeWinImpl threads through the
//\twindow procedure.
struct _ATL_MSG {
  HWND hWnd;
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
  _ATL_MSG *previous;
  BOOL handled;
  LRESULT result;
  _ATL_MSG(HWND window, UINT msg, WPARAM wp, LPARAM lp)
      : hWnd(window), message(msg), wParam(wp), lParam(lp),
        previous(nullptr), handled(FALSE), result(0) {}
};

//\tCWindowImpl: the window implementation base. The adapter's dispatch
//\tcreates the platform window (an overlay element on the browser target).
template <class T, class Base = CWindow, class Traits = CControlWinTraits>
class CWindowImpl : public Base {
public:
  HWND Create(HWND parent, RECT &bounds, LPCTSTR title = (LPCTSTR)0,
              DWORD style = 0, DWORD exStyle = 0, UINT id = 0,
              void *createParam = nullptr) {
    (void)parent;
    (void)bounds;
    (void)title;
    (void)style;
    (void)exStyle;
    (void)id;
    (void)createParam;
    //\tThe browser-dialog adapter supplies the real window; until it
    //\tregisters, creation fails explicitly (no fake HWND).
    m_hWnd = nullptr;
    return m_hWnd;
  }
  BOOL DestroyWindow() { return TRUE; }
  //\tThe message-map contract (the original CWindowImplRoot pure virtual;
  //\tthe derived class supplies it through BEGIN_MSG_MAP — the entry
  //\tSafeWinImpl's window procedure dispatches through).
  virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, LRESULT &result,
                                    DWORD dwMsgMapID) = 0;
  //\tThe final-message hook (the original CWindowImplRoot default; the
  //\tsafe window procedure calls it after the window is destroyed).
  virtual void OnFinalMessage(HWND hWnd) { (void)hWnd; }
  const _ATL_MSG *m_pCurrentMsg = nullptr;
  HWND m_hWnd = nullptr;
  DWORD m_dwState = 0;
  WNDPROC m_pfnSuperWindowProc = &DefWindowProcWrapper;
  static LRESULT DefWindowProcWrapper(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
  }
  LRESULT DefWindowProc(UINT msg, WPARAM wParam, LPARAM lParam) {
    return ::DefWindowProc(m_hWnd, msg, wParam, lParam);
  }
};

#endif // PW153_PLATFORM_ATLWIN_H
