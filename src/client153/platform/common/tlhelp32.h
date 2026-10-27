/*
 * FILE: tlhelp32.h (platform seam)
 *
 * DESCRIPTION: The Toolhelp process-snapshot surface the defence and
 * system-introspection sources compile against. Types and constants keep
 * the documented Win32 layout (PROCESSENTRY32/MODULEENTRY32/THREADENTRY32
 * plus the heap-list/entry structs and the TH32CS_* snapshot flags), and
 * every function is declared with C linkage exactly like the SDK header.
 *
 * HONEST BOUNDARY: the browser environment has NO process enumeration -
 * CreateToolhelp32Snapshot cannot return real system data here. The
 * implementations in toolhelp.cpp therefore fail explicitly instead of
 * faking success: they return INVALID_HANDLE_VALUE / FALSE and set
 * GetLastError to ERROR_CALL_NOT_IMPLEMENTED, the documented "not
 * supported on this platform" behavior. The defence sources already
 * handle that path (they check for INVALID_HANDLE_VALUE / FALSE), so the
 * defence feature degrades to "no processes found" exactly as it would
 * on a locked-down system.
 *
 * API surface per the Windows SDK tlhelp32.h (Microsoft); this seam
 * declares the documented interface, it is not a copy of the SDK header.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_TLHELP32_H
#define PW153_PLATFORM_TLHELP32_H

#include "Windows.h"

#ifndef ERROR_CALL_NOT_IMPLEMENTED
#define ERROR_CALL_NOT_IMPLEMENTED 120 /* 0x78: "This function is not supported on this system." */
#endif

#define TH32CS_SNAPHEAPLIST 0x00000001
#define TH32CS_SNAPPROCESS  0x00000002
#define TH32CS_SNAPTHREAD   0x00000004
#define TH32CS_SNAPMODULE   0x00000008
#define TH32CS_SNAPALL      (TH32CS_SNAPHEAPLIST | TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD | TH32CS_SNAPMODULE)
#define TH32CS_INHERIT      0x80000000

#ifndef MAX_MODULE_NAME32
#define MAX_MODULE_NAME32 255
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***** Process walking *************************************************/

typedef struct tagPROCESSENTRY32
{
    DWORD   dwSize;
    DWORD   cntUsage;
    DWORD   th32ProcessID;          // this process
    DWORD   th32DefaultHeapID;
    DWORD   th32ModuleID;           // associated exe
    DWORD   cntThreads;
    DWORD   th32ParentProcessID;    // this process's parent process
    LONG    pcPriClassBase;         // Base priority of process's threads
    DWORD   dwFlags;
    CHAR    szExeFile[MAX_PATH];    // Path
} PROCESSENTRY32;
typedef PROCESSENTRY32 *  PPROCESSENTRY32;
typedef PROCESSENTRY32 *  LPPROCESSENTRY32;

typedef struct tagPROCESSENTRY32W
{
    DWORD   dwSize;
    DWORD   cntUsage;
    DWORD   th32ProcessID;          // this process
    DWORD   th32DefaultHeapID;
    DWORD   th32ModuleID;           // associated exe
    DWORD   cntThreads;
    DWORD   th32ParentProcessID;    // this process's parent process
    LONG    pcPriClassBase;         // Base priority of process's threads
    DWORD   dwFlags;
    WCHAR   szExeFile[MAX_PATH];    // Path
} PROCESSENTRY32W;
typedef PROCESSENTRY32W *  PPROCESSENTRY32W;
typedef PROCESSENTRY32W *  LPPROCESSENTRY32W;

HANDLE
WINAPI
CreateToolhelp32Snapshot(
    DWORD dwFlags,
    DWORD th32ProcessID
    );

BOOL
WINAPI
Process32First(
    HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe
    );

BOOL
WINAPI
Process32Next(
    HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe
    );

BOOL
WINAPI
Process32FirstW(
    HANDLE hSnapshot,
    LPPROCESSENTRY32W lppe
    );

BOOL
WINAPI
Process32NextW(
    HANDLE hSnapshot,
    LPPROCESSENTRY32W lppe
    );

/***** Module walking ***************************************************/

typedef struct tagMODULEENTRY32
{
    DWORD   dwSize;
    DWORD   th32ModuleID;       // This module
    DWORD   th32ProcessID;      // owning process
    DWORD   GlblcntUsage;       // Global usage count on the module
    DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
    BYTE *  modBaseAddr;        // Base address of module in th32ProcessID's context
    DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
    HMODULE hModule;            // The hModule of this module in th32ProcessID's context
    CHAR    szModule[MAX_MODULE_NAME32 + 1]; // The name of module
    CHAR    szExePath[MAX_PATH];     // The path of the module
} MODULEENTRY32;
typedef MODULEENTRY32 *  PMODULEENTRY32;
typedef MODULEENTRY32 *  LPMODULEENTRY32;

typedef struct tagMODULEENTRY32W
{
    DWORD   dwSize;
    DWORD   th32ModuleID;       // This module
    DWORD   th32ProcessID;      // owning process
    DWORD   GlblcntUsage;       // Global usage count on the module
    DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
    BYTE *  modBaseAddr;        // Base address of module in th32ProcessID's context
    DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
    HMODULE hModule;            // The hModule of this module in th32ProcessID's context
    WCHAR   szModule[MAX_MODULE_NAME32 + 1]; // The name of module
    WCHAR   szExePath[MAX_PATH];     // The path of the module
} MODULEENTRY32W;
typedef MODULEENTRY32W *  PMODULEENTRY32W;
typedef MODULEENTRY32W *  LPMODULEENTRY32W;

BOOL
WINAPI
Module32First(
    HANDLE hSnapshot,
    LPMODULEENTRY32 lpme
    );

BOOL
WINAPI
Module32Next(
    HANDLE hSnapshot,
    LPMODULEENTRY32 lpme
    );

BOOL
WINAPI
Module32FirstW(
    HANDLE hSnapshot,
    LPMODULEENTRY32W lpme
    );

BOOL
WINAPI
Module32NextW(
    HANDLE hSnapshot,
    LPMODULEENTRY32W lpme
    );

/***** Thread walking **************************************************/

typedef struct tagTHREADENTRY32
{
    DWORD   dwSize;
    DWORD   cntUsage;
    DWORD   th32ThreadID;       // this thread
    DWORD   th32OwnerProcessID; // Process this thread is associated with
    LONG    tpBasePri;
    LONG    tpDeltaPri;
    DWORD   dwFlags;
} THREADENTRY32;
typedef THREADENTRY32 *  PTHREADENTRY32;
typedef THREADENTRY32 *  LPTHREADENTRY32;

typedef struct tagTHREADENTRY32W
{
    DWORD   dwSize;
    DWORD   cntUsage;
    DWORD   th32ThreadID;       // this thread
    DWORD   th32OwnerProcessID; // Process this thread is associated with
    LONG    tpBasePri;
    LONG    tpDeltaPri;
    DWORD   dwFlags;
} THREADENTRY32W;
typedef THREADENTRY32W *  PTHREADENTRY32W;
typedef THREADENTRY32W *  LPTHREADENTRY32W;

BOOL
WINAPI
Thread32First(
    HANDLE hSnapshot,
    LPTHREADENTRY32 lpte
    );

BOOL
WINAPI
Thread32Next(
    HANDLE hSnapshot,
    LPTHREADENTRY32 lpte
    );

BOOL
WINAPI
Thread32FirstW(
    HANDLE hSnapshot,
    LPTHREADENTRY32W lpte
    );

BOOL
WINAPI
Thread32NextW(
    HANDLE hSnapshot,
    LPTHREADENTRY32W lpte
    );

/***** Heap walking ****************************************************/

typedef struct tagHEAPLIST32
{
    SIZE_T     dwSize;
    DWORD      th32ProcessID;   // owning process
    ULONG_PTR  th32HeapID;      // heap (in owning process's context!)
    DWORD      dwFlags;
} HEAPLIST32;
typedef HEAPLIST32 *  PHEAPLIST32;
typedef HEAPLIST32 *  LPHEAPLIST32;

typedef struct tagHEAPLIST32W
{
    SIZE_T     dwSize;
    DWORD      th32ProcessID;   // owning process
    ULONG_PTR  th32HeapID;      // heap (in owning process's context!)
    DWORD      dwFlags;
} HEAPLIST32W;
typedef HEAPLIST32W *  PHEAPLIST32W;
typedef HEAPLIST32W *  LPHEAPLIST32W;

BOOL
WINAPI
Heap32ListFirst(
    HANDLE hSnapshot,
    LPHEAPLIST32 lphl
    );

BOOL
WINAPI
Heap32ListNext(
    HANDLE hSnapshot,
    LPHEAPLIST32 lphl
    );

BOOL
WINAPI
Heap32ListFirstW(
    HANDLE hSnapshot,
    LPHEAPLIST32W lphl
    );

BOOL
WINAPI
Heap32ListNextW(
    HANDLE hSnapshot,
    LPHEAPLIST32W lphl
    );

typedef struct tagHEAPENTRY32
{
    SIZE_T   dwSize;
    HANDLE   hHandle;
    DWORD    dwAddress;
    DWORD    dwBlockSize;
    DWORD    dwFlags;
    DWORD    dwLockCount;
    DWORD    dwResvd;
    DWORD    th32ProcessID;
} HEAPENTRY32;
typedef HEAPENTRY32 *  PHEAPENTRY32;
typedef HEAPENTRY32 *  LPHEAPENTRY32;

typedef struct tagHEAPENTRY32W
{
    SIZE_T   dwSize;
    HANDLE   hHandle;
    DWORD    dwAddress;
    DWORD    dwBlockSize;
    DWORD    dwFlags;
    DWORD    dwLockCount;
    DWORD    dwResvd;
    DWORD    th32ProcessID;
} HEAPENTRY32W;
typedef HEAPENTRY32W *  PHEAPENTRY32W;
typedef HEAPENTRY32W *  LPHEAPENTRY32W;

BOOL
WINAPI
Heap32First(
    LPHEAPENTRY32 lphe,
    DWORD th32ProcessID,
    ULONG_PTR th32HeapID
    );

BOOL
WINAPI
Heap32Next(
    LPHEAPENTRY32 lphe
    );

BOOL
WINAPI
Heap32FirstW(
    LPHEAPENTRY32W lphe,
    DWORD th32ProcessID,
    ULONG_PTR th32HeapID
    );

BOOL
WINAPI
Heap32NextW(
    LPHEAPENTRY32W lphe
    );

/***** Process memory **************************************************/

BOOL
WINAPI
Toolhelp32ReadProcessMemory(
    DWORD th32ProcessID,
    LPCVOID lpBaseAddress,
    LPVOID lpBuffer,
    SIZE_T cbRead,
    SIZE_T * lpNumberOfBytesRead
    );

#ifdef __cplusplus
}
#endif

//	The SDK header maps the A names to the W variants under UNICODE; the
//	adapted defence sources rely on that mapping (infocollection.h undefs
//	the process/module names to restore the A functions it loads).
#ifdef UNICODE
#define Process32First      Process32FirstW
#define Process32Next       Process32NextW
#define PROCESSENTRY32      PROCESSENTRY32W
#define PPROCESSENTRY32     PPROCESSENTRY32W
#define LPPROCESSENTRY32    LPPROCESSENTRY32W
#define Module32First       Module32FirstW
#define Module32Next        Module32NextW
#define MODULEENTRY32       MODULEENTRY32W
#define PMODULEENTRY32      PMODULEENTRY32W
#define LPMODULEENTRY32     LPMODULEENTRY32W
#define Thread32First       Thread32FirstW
#define Thread32Next        Thread32NextW
#define THREADENTRY32       THREADENTRY32W
#define PTHREADENTRY32      PTHREADENTRY32W
#define LPTHREADENTRY32     LPTHREADENTRY32W
#define Heap32ListFirst     Heap32ListFirstW
#define Heap32ListNext      Heap32ListNextW
#define HEAPLIST32          HEAPLIST32W
#define PHEAPLIST32         PHEAPLIST32W
#define LPHEAPLIST32        LPHEAPLIST32W
#define Heap32First         Heap32FirstW
#define Heap32Next          Heap32NextW
#define HEAPENTRY32         HEAPENTRY32W
#define PHEAPENTRY32        PHEAPENTRY32W
#define LPHEAPENTRY32       LPHEAPENTRY32W
#endif // UNICODE

#endif // PW153_PLATFORM_TLHELP32_H