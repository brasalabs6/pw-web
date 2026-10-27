/*
 * FILE: Psapi.h (platform seam)
 *
 * DESCRIPTION: The process status API surface the defence code compiles
 * against. The browser target has no process enumeration: the documented
 * type surface lets the sources compile, and every function fails
 * explicitly (returns zero/FALSE with GetLastError set to
 * ERROR_CALL_NOT_IMPLEMENTED) - the defence feature degrades to "no
 * processes found" exactly as on a locked-down system, the paths the
 * defence code already checks.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_PSAPI_H
#define PW153_PLATFORM_PSAPI_H

#include "Windows.h"

struct PROCESS_MEMORY_COUNTERS {
  DWORD cb;
  DWORD PageFaultCount;
  std::size_t PeakWorkingSetSize;
  std::size_t WorkingSetSize;
  std::size_t QuotaPeakPagedPoolUsage;
  std::size_t QuotaPagedPoolUsage;
  std::size_t QuotaPeakNonPagedPoolUsage;
  std::size_t QuotaNonPagedPoolUsage;
  std::size_t PagefileUsage;
  std::size_t PeakPagefileUsage;
};

typedef PROCESS_MEMORY_COUNTERS *PPROCESS_MEMORY_COUNTERS;

#define LIST_MODULES_ALL 0x03

struct MODULEINFO {
  DWORD lpBaseOfDll;
  DWORD SizeOfImage;
  DWORD EntryPoint;
};

typedef MODULEINFO *LPMODULEINFO;

//\tThe process enumeration: no processes exist on the browser target.
inline BOOL EnumProcesses(DWORD *processIds, DWORD cb, DWORD *bytesReturned) {
  if (bytesReturned)
    *bytesReturned = 0;
  if (processIds && cb)
    processIds[0] = 0;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

inline BOOL EnumProcessModules(HANDLE process, HMODULE *modules, DWORD cb,
                               DWORD *needed) {
  (void)process;
  if (needed)
    *needed = 0;
  if (modules && cb)
    modules[0] = nullptr;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

inline DWORD GetModuleBaseNameA(HANDLE process, HMODULE module,
                                char *baseName, DWORD size) {
  (void)process;
  (void)module;
  if (baseName && size)
    baseName[0] = '\0';
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return 0;
}

inline DWORD GetModuleBaseNameW(HANDLE process, HMODULE module,
                                wchar_t *baseName, DWORD size) {
  (void)process;
  (void)module;
  if (baseName && size)
    baseName[0] = L'\0';
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return 0;
}

inline DWORD GetModuleFileNameExA(HANDLE process, HMODULE module,
                                  char *baseName, DWORD size) {
  (void)process;
  (void)module;
  if (baseName && size)
    baseName[0] = '\0';
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return 0;
}

inline BOOL GetProcessMemoryInfo(HANDLE process,
                                 PROCESS_MEMORY_COUNTERS *counters, DWORD cb) {
  (void)process;
  if (counters && cb >= sizeof(PROCESS_MEMORY_COUNTERS)) {
    counters->cb = sizeof(PROCESS_MEMORY_COUNTERS);
    counters->PageFaultCount = 0;
    counters->WorkingSetSize = 0;
    counters->PeakWorkingSetSize = 0;
  }
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

#endif // PW153_PLATFORM_PSAPI_H
