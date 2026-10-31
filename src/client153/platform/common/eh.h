/*
 * FILE: eh.h (platform seam)
 *
 * DESCRIPTION: The Win32 structured-exception DATA types the prebuilt SDK
 * headers (jexception.h's crash reporter) compile against. These are the
 * documented x86 record layouts. On the browser target there is no SEH
 * delivery: the crash path goes through the diagnostic log; the game's
 * __try guards were converted to try/catch (the C++-exception subset is
 * preserved) and hardware faults trap to the runtime.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_EH_H
#define PW153_PLATFORM_EH_H

#include "Windows.h"

#define EXCEPTION_MAXIMUM_PARAMETERS 15

typedef struct _FLOATING_SAVE_AREA {
  DWORD ControlWord;
  DWORD StatusWord;
  DWORD TagWord;
  DWORD ErrorOffset;
  DWORD ErrorSelector;
  DWORD DataOffset;
  DWORD DataSelector;
  BYTE RegisterArea[80];
  DWORD Cr0NpxState;
} FLOATING_SAVE_AREA;

typedef struct _EXCEPTION_RECORD {
  DWORD ExceptionCode;
  DWORD ExceptionFlags;
  struct _EXCEPTION_RECORD *ExceptionRecord;
  PVOID ExceptionAddress;
  DWORD NumberParameters;
  ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD;

//	The x86 context record: the documented layout the crash reporter
//	stores. Fields are only meaningful on x86; the browser crash path
//	never fills one.
typedef struct _CONTEXT {
  DWORD ContextFlags;
  DWORD Dr0;
  DWORD Dr1;
  DWORD Dr2;
  DWORD Dr3;
  DWORD Dr6;
  DWORD Dr7;
  FLOATING_SAVE_AREA *FloatSave;
  DWORD SegGs;
  DWORD SegFs;
  DWORD SegEs;
  DWORD SegDs;
  DWORD Edi;
  DWORD Esi;
  DWORD Ebx;
  DWORD Edx;
  DWORD Ecx;
  DWORD Eax;
  DWORD Ebp;
  DWORD Eip;
  DWORD SegCs;
  DWORD EFlags;
  DWORD Esp;
  DWORD SegSs;
} CONTEXT;

typedef CONTEXT *PCONTEXT;

typedef struct _EXCEPTION_POINTERS {
  EXCEPTION_RECORD *ExceptionRecord;
  CONTEXT *ContextRecord;
} EXCEPTION_POINTERS;

typedef EXCEPTION_POINTERS *PEXCEPTION_POINTERS;


#define EXCEPTION_ACCESS_VIOLATION 0xC0000005
#define EXCEPTION_DATATYPE_MISALIGNMENT 0x80000002
#define EXCEPTION_BREAKPOINT 0x80000003
#define EXCEPTION_SINGLE_STEP 0x80000004
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED 0xC000008C
#define EXCEPTION_FLT_DENORMAL_OPERAND 0xC0000090
#define EXCEPTION_FLT_DIVIDE_BY_ZERO 0xC0000091
#define EXCEPTION_INT_DIVIDE_BY_ZERO 0xC0000094
#define EXCEPTION_INT_OVERFLOW 0xC0000095
#define EXCEPTION_ILLEGAL_INSTRUCTION 0xC000001D
#define EXCEPTION_STACK_OVERFLOW 0xC00000FD
#define EXCEPTION_IN_PAGE_ERROR 0xC0000006
#define EXCEPTION_NONCONTINUABLE_EXCEPTION 0xC0000025

#endif // PW153_PLATFORM_EH_H
