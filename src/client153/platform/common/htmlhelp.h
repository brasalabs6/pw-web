/*
 * FILE: htmlhelp.h (platform seam)
 *
 * DESCRIPTION: The HTML Help API surface. The browser target has no HTML
 * help; the documented type surface lets the sources compile, and the help
 * calls fail explicitly (the callers' fallback paths handle it).
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_HTMLHELP_H
#define PW153_PLATFORM_HTMLHELP_H

#include "Windows.h"

typedef HWND HWND_TYPE_HTMLHELP;
typedef DWORD DWORD_TYPE_HTMLHELP;

inline HWND HtmlHelpA(HWND caller, const char *file, unsigned int command,
                      DWORD data) {
  (void)caller;
  (void)file;
  (void)command;
  (void)data;
  return nullptr;
}

inline HWND HtmlHelpW(HWND caller, const wchar_t *file, unsigned int command,
                      DWORD data) {
  (void)caller;
  (void)file;
  (void)command;
  (void)data;
  return nullptr;
}

#endif
