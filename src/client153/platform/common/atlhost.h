/*
 * FILE: atlhost.h (platform seam)
 *
 * DESCRIPTION: The ATL ActiveX hosting surface the browser-dialog wrapper
 * uses (AtlAxWin, CAxWindow, AtlAxGetControl, AtlAxCreateControl). The
 * browser-dialog adapter (an HTML overlay in the UI cycle) implements the
 * hosting: the embedded web view maps to a real iframe/overlay element.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_ATLHOST_H
#define PW153_PLATFORM_ATLHOST_H

#include "atlbase.h"
#include "atlwin.h"

//\tThe ActiveX hosting window class name (the original registered it).
#define _T(text) L##text
static const wchar_t AtlAxWin[] = L"AtlAxWin";

class CAxWindow : public CWindow {
public:
  CAxWindow(HWND hWnd = nullptr) : CWindow(hWnd) {}
  //\tHost creation: the adapter's overlay supplies the real host window;
  //\tuntil it registers, creation fails explicitly.
  HWND Create(HWND parent, RECT &bounds, LPCTSTR title = nullptr,
              DWORD style = 0, DWORD exStyle = 0) {
    (void)parent;
    (void)bounds;
    (void)title;
    (void)style;
    (void)exStyle;
    m_hWnd = nullptr;
    return m_hWnd;
  }
};

//\tControl retrieval: resolves the hosted control's dispatch interface.
//\tThe adapter registry resolves the web view; without the adapter it
//\tfails explicitly (no fake interface).
inline HRESULT AtlAxGetControl(HWND hWnd, IUnknown **unknown) {
  (void)hWnd;
  if (unknown)
    *unknown = nullptr;
  return E_FAIL;
}

inline HRESULT AtlAxCreateControl(const wchar_t *name, HWND parent, RECT *bounds,
                                  IUnknown **stream = nullptr) {
  (void)name;
  (void)parent;
  (void)bounds;
  if (stream)
    *stream = nullptr;
  return E_FAIL;
}

#endif // PW153_PLATFORM_ATLHOST_H
