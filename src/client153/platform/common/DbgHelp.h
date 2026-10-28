/*
 * FILE: DbgHelp.h (platform seam)
 *
 * DESCRIPTION: The DbgHelp minidump surface the client's crash writer
 * compiles against. The browser target has no dbghelp.dll to load (the
 * game loads it dynamically; LoadLibrary fails explicitly, so the dump
 * writer reports failure through its own null checks). The types carry
 * the documented dbghelp layouts.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_DBGHELP_H
#define PW153_PLATFORM_DBGHELP_H

#include "Windows.h"

//	The minidump type flags (the documented MINIDUMP_TYPE values).
typedef enum _MINIDUMP_TYPE {
  MiniDumpNormal = 0x0000,
  MiniDumpWithDataSegs = 0x0001,
  MiniDumpWithFullMemory = 0x0002,
  MiniDumpWithHandleData = 0x0004,
  MiniDumpFilterMemory = 0x0008,
  MiniDumpScanMemory = 0x0010,
  MiniDumpWithUnloadedModules = 0x0020,
  MiniDumpWithIndirectlyReferencedMemory = 0x0040,
  MiniDumpFilterModulePaths = 0x0080,
  MiniDumpWithProcessThreadData = 0x0100,
  MiniDumpWithPrivateReadWriteMemory = 0x0200,
  MiniDumpWithoutOptionalData = 0x0400,
  MiniDumpWithFullMemoryInfo = 0x0800,
  MiniDumpWithThreadInfo = 0x1000,
  MiniDumpWithCodeSegs = 0x2000
} MINIDUMP_TYPE;

//	The minidump exception record (the documented layout).
typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
  DWORD ThreadId;
  PEXCEPTION_POINTERS ExceptionPointers;
  BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;

//	The dynamic entry the crash writer resolves from dbghelp.dll (the
//	documented signature; the load fails on the browser target so the
//	pointer stays null and the writer reports failure).
typedef HRESULT(WINAPI *MINIDUMPWRITEDUMP)(
    HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType,
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, void *UserStreamParam,
    void *CallbackParam);

#endif // PW153_PLATFORM_DBGHELP_H
