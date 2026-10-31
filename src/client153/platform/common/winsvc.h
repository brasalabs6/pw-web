/*
 * FILE: winsvc.h (platform seam)
 *
 * DESCRIPTION: The Win32 service-control surface the defence code
 * compiles against. The browser target has no service manager; the
 * queries fail explicitly (the callers' error paths handle it).
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_WINSVC_H
#define PW153_PLATFORM_WINSVC_H

#include "Windows.h"

//	The service manager database handle (the documented Win32 handle).
typedef void *SC_HANDLE;
typedef SC_HANDLE *LPSC_HANDLE;

//	The service control types and states (the documented WinSvc values).
#define SERVICE_ALL_ACCESS 0xF01FF
#define SERVICE_QUERY_STATUS 0x0004
#define SERVICE_RUNNING 0x00000004
#define SERVICE_STOPPED 0x00000001

//	The service status record (the documented layout).
typedef struct _SERVICE_STATUS {
  DWORD dwServiceType;
  DWORD dwCurrentState;
  DWORD dwControlsAccepted;
  DWORD dwWin32ExitCode;
  DWORD dwServiceSpecificExitCode;
  DWORD dwCheckPoint;
  DWORD dwWaitHint;
} SERVICE_STATUS, *LPSERVICE_STATUS;

//	The service manager open: no service manager exists on the browser
//	target; the open fails (NULL).
inline SC_HANDLE OpenSCManagerA(const char *machine, const char *database,
                                DWORD access) {
  (void)machine;
  (void)database;
  (void)access;
  return nullptr;
}
inline SC_HANDLE OpenSCManagerW(const wchar_t *machine,
                               const wchar_t *database, DWORD access) {
  (void)machine;
  (void)database;
  (void)access;
  return nullptr;
}
inline SC_HANDLE OpenSCManager(const wchar_t *machine,
                               const wchar_t *database, DWORD access) {
  return OpenSCManagerW(machine, database, access);
}

//	The service open and control: the same explicit failure.
inline SC_HANDLE OpenServiceA(SC_HANDLE manager, const char *service,
                              DWORD access) {
  (void)manager;
  (void)service;
  (void)access;
  return nullptr;
}
inline BOOL QueryServiceStatus(SC_HANDLE service,
                               SERVICE_STATUS *status) {
  (void)service;
  if (status)
    std::memset(status, 0, sizeof(*status));
  return FALSE;
}
inline BOOL CloseServiceHandle(SC_HANDLE handle) {
  (void)handle;
  return TRUE;
}

#endif // PW153_PLATFORM_WINSVC_H
