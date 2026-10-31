/*
 * FILE: toolhelp.cpp (platform seam)
 *
 * DESCRIPTION: The Toolhelp process-snapshot implementations. The browser
 * environment has no process enumeration, so every entry point fails
 * explicitly with ERROR_CALL_NOT_IMPLEMENTED (GetLastError 0x78 / 120)
 * instead of faking success: CreateToolhelp32Snapshot returns
 * INVALID_HANDLE_VALUE and the enumeration functions return FALSE. The
 * defence sources already treat those as the "no processes found" path
 * (they check INVALID_HANDLE_VALUE / FALSE), so the defence feature
 * degrades exactly as it would on a locked-down system. Declarations
 * live in tlhelp32.h with C linkage, matching the SDK API surface.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#include "tlhelp32.h"

//	The UNICODE mapping (defined at the end of tlhelp32.h) redirects the
//	unqualified names to the W variants; the definitions below spell out
//	every A and W function explicitly, so disable the mapping first. The
//	header declares both variants unconditionally, so these definitions
//	match those declarations under either setting.
#undef Process32First
#undef Process32Next
#undef PROCESSENTRY32
#undef PPROCESSENTRY32
#undef LPPROCESSENTRY32
#undef Module32First
#undef Module32Next
#undef MODULEENTRY32
#undef PMODULEENTRY32
#undef LPMODULEENTRY32
#undef Thread32First
#undef Thread32Next
#undef THREADENTRY32
#undef PTHREADENTRY32
#undef LPTHREADENTRY32
#undef Heap32ListFirst
#undef Heap32ListNext
#undef HEAPLIST32
#undef PHEAPLIST32
#undef LPHEAPLIST32
#undef Heap32First
#undef Heap32Next
#undef HEAPENTRY32
#undef PHEAPENTRY32
#undef LPHEAPENTRY32

extern "C" {

HANDLE WINAPI CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID) {
  (void)dwFlags;
  (void)th32ProcessID;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return INVALID_HANDLE_VALUE;
}

BOOL WINAPI Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe) {
  (void)hSnapshot;
  (void)lppe;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe) {
  (void)hSnapshot;
  (void)lppe;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Process32FirstW(HANDLE hSnapshot, LPPROCESSENTRY32W lppe) {
  (void)hSnapshot;
  (void)lppe;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Process32NextW(HANDLE hSnapshot, LPPROCESSENTRY32W lppe) {
  (void)hSnapshot;
  (void)lppe;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Module32First(HANDLE hSnapshot, LPMODULEENTRY32 lpme) {
  (void)hSnapshot;
  (void)lpme;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Module32Next(HANDLE hSnapshot, LPMODULEENTRY32 lpme) {
  (void)hSnapshot;
  (void)lpme;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Module32FirstW(HANDLE hSnapshot, LPMODULEENTRY32W lpme) {
  (void)hSnapshot;
  (void)lpme;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Module32NextW(HANDLE hSnapshot, LPMODULEENTRY32W lpme) {
  (void)hSnapshot;
  (void)lpme;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Thread32First(HANDLE hSnapshot, LPTHREADENTRY32 lpte) {
  (void)hSnapshot;
  (void)lpte;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Thread32Next(HANDLE hSnapshot, LPTHREADENTRY32 lpte) {
  (void)hSnapshot;
  (void)lpte;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Thread32FirstW(HANDLE hSnapshot, LPTHREADENTRY32W lpte) {
  (void)hSnapshot;
  (void)lpte;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Thread32NextW(HANDLE hSnapshot, LPTHREADENTRY32W lpte) {
  (void)hSnapshot;
  (void)lpte;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Heap32ListFirst(HANDLE hSnapshot, LPHEAPLIST32 lphl) {
  (void)hSnapshot;
  (void)lphl;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Heap32ListNext(HANDLE hSnapshot, LPHEAPLIST32 lphl) {
  (void)hSnapshot;
  (void)lphl;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Heap32ListFirstW(HANDLE hSnapshot, LPHEAPLIST32W lphl) {
  (void)hSnapshot;
  (void)lphl;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Heap32ListNextW(HANDLE hSnapshot, LPHEAPLIST32W lphl) {
  (void)hSnapshot;
  (void)lphl;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Heap32First(LPHEAPENTRY32 lphe, DWORD th32ProcessID,
                        ULONG_PTR th32HeapID) {
  (void)lphe;
  (void)th32ProcessID;
  (void)th32HeapID;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Heap32Next(LPHEAPENTRY32 lphe) {
  (void)lphe;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Heap32FirstW(LPHEAPENTRY32W lphe, DWORD th32ProcessID,
                         ULONG_PTR th32HeapID) {
  (void)lphe;
  (void)th32ProcessID;
  (void)th32HeapID;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Heap32NextW(LPHEAPENTRY32W lphe) {
  (void)lphe;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

BOOL WINAPI Toolhelp32ReadProcessMemory(DWORD th32ProcessID,
                                        LPCVOID lpBaseAddress, LPVOID lpBuffer,
                                        SIZE_T cbRead,
                                        SIZE_T *lpNumberOfBytesRead) {
  (void)th32ProcessID;
  (void)lpBaseAddress;
  (void)lpBuffer;
  (void)cbRead;
  if (lpNumberOfBytesRead)
    *lpNumberOfBytesRead = 0;
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
  return FALSE;
}

}  // extern "C"