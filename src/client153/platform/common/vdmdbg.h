/*
 * FILE: vdmdbg.h (platform seam)
 *
 * DESCRIPTION: The 16-bit VDM (Virtual DOS Machine) debug interface the
 * defence process enumeration reaches (VDMEnumTaskWOWEx and the task
 * structures). The browser target has no VDM: the documented type surface
 * lets the sources compile, and the enumeration degrades to "no tasks"
 * through the honest failure semantics - the defence code already checks
 * for empty results.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_VDMDBG_H
#define PW153_PLATFORM_VDMDBG_H

#include "Windows.h"

#define MAX_MODULE_NAME32 255

#ifndef PW153_MODULEENTRY32W_DEFINED
#define PW153_MODULEENTRY32W_DEFINED
struct tagMODULEENTRY32W {
  DWORD dwSize;
  DWORD th32ModuleID;
  DWORD th32ProcessID;
  DWORD GlblcntUsage;
  DWORD ProccntUsage;
  BYTE *modBaseAddr;
  DWORD modBaseSize;
  HMODULE hModule;
  wchar_t szModule[MAX_MODULE_NAME32 + 1];
  wchar_t szExePath[MAX_PATH];
};

typedef struct tagMODULEENTRY32W MODULEENTRY32W;
typedef MODULEENTRY32W *LPMODULEENTRY32W;
#endif	//	PW153_MODULEENTRY32W_DEFINED

//	VDM task enumeration callback: receives the process info per 16-bit task.
typedef BOOL( *TASKENUMPROCEX)(DWORD dwThreadId, WORD hMod16, WORD hTask16,
                               const char *pszModName, const char *pszFileName,
                               LPARAM lpUserDefined);

//	The VDM enumeration entry points: the honest browser semantics - no VDM
//	exists, so the enumeration reports zero tasks (the documented empty-set
//	result, not a fake success).
inline BOOL VDMEnumTaskWOWEx(DWORD dwProcessId, TASKENUMPROCEX callback,
                             LPARAM lParam) {
  (void)dwProcessId;
  (void)callback;
  (void)lParam;
  return 0;
}

inline BOOL VDMEnumTaskWOW(DWORD dwProcessId, void *callback, LPARAM lParam) {
  (void)dwProcessId;
  (void)callback;
  (void)lParam;
  return 0;
}

#endif // PW153_PLATFORM_VDMDBG_H
