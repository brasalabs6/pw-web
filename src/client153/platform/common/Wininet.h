/*
 * FILE: Wininet.h (platform seam)
 *
 * DESCRIPTION: The WinINet HTTP surface the client's BBS/download code
 * compiles against. The browser target has no WinINet: the HTTP surface
 * maps to the fetch-backed network adapter in the network cycle. The
 * documented type surface lets the sources compile; every function fails
 * explicitly (returns a null handle / FALSE with GetLastError set to
 * ERROR_INTERNET_ not supported) - the callers' error paths handle it,
 * and the fetch adapter registers the real implementations when the
 * network cycle lands.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_WININET_H
#define PW153_PLATFORM_WININET_H

#include "Windows.h"

typedef void *HINTERNET;
typedef const wchar_t *LPCWSTR_INTERNET_UNUSED;

#define INTERNET_FLAG_RELOAD 0x80000000
#define INTERNET_FLAG_NO_AUTO_REDIRECT 0x00200000
#define INTERNET_FLAG_EXISTING_CONNECT 0x20000000
#define INTERNET_FLAG_DONT_CACHE 0x04000000
#define INTERNET_FLAG_SECURE 0x00800000

#define INTERNET_OPEN_TYPE_PRECONFIG 0
#define INTERNET_OPEN_TYPE_DIRECT 1
#define INTERNET_OPEN_TYPE_PROXY 3
#define INTERNET_DEFAULT_HTTP_PORT 80
#define INTERNET_DEFAULT_HTTPS_PORT 443
#define INTERNET_SERVICE_HTTP 1

#define HTTP_STATUS_OK 200
#define HTTP_STATUS_PARTIAL_CONTENT 206
#define HTTP_STATUS_MOVED 301
#define HTTP_STATUS_REDIRECT 302
#define HTTP_STATUS_REDIRECT_METHOD 303
#define HTTP_QUERY_CONTENT_LENGTH 5
#define HTTP_QUERY_STATUS_CODE 19
#define HTTP_STATUS_REDIRECT 302

#define HTTP_QUERY_STATUS_CODE 19
#define HTTP_QUERY_RAW_HEADERS_CRLF 22
#define HTTP_QUERY_FLAG_REQUEST_HEADERS 0x80000000

#define HTTP_ADDREQ_FLAG_ADD_IF_NEW 0x10000000
#define HTTP_ADDREQ_FLAG_ADD 0x20000000
#define HTTP_ADDREQ_FLAG_REPLACE 0x80000000

#define ERROR_INTERNET_NOT_SUPPORTED 12012

//\tThe WinINet session/handle creation: no WinINet exists on the browser
//\ttarget; the calls fail explicitly (the callers check for null handles).
inline HINTERNET InternetOpenA(const char *agent, DWORD accessType,
                               const char *proxy, const char *proxyBypass,
                               DWORD flags) {
  (void)agent;
  (void)accessType;
  (void)proxy;
  (void)proxyBypass;
  (void)flags;
  SetLastError(ERROR_INTERNET_NOT_SUPPORTED);
  return nullptr;
}

inline HINTERNET InternetOpenW(const wchar_t *agent, DWORD accessType,
                               const wchar_t *proxy,
                               const wchar_t *proxyBypass, DWORD flags) {
  (void)agent;
  (void)accessType;
  (void)proxy;
  (void)proxyBypass;
  (void)flags;
  SetLastError(ERROR_INTERNET_NOT_SUPPORTED);
  return nullptr;
}

inline HINTERNET InternetConnectA(HINTERNET session, const char *server,
                                  WORD port, const char *user,
                                  const char *password, DWORD service,
                                  DWORD flags, DWORD context) {
  (void)session;
  (void)server;
  (void)port;
  (void)user;
  (void)password;
  (void)service;
  (void)flags;
  (void)context;
  SetLastError(ERROR_INTERNET_NOT_SUPPORTED);
  return nullptr;
}

inline HINTERNET HttpOpenRequestA(HINTERNET connection, const char *verb,
                                  const char *object, const char *version,
                                  const char *referrer, const char **acceptTypes,
                                  DWORD flags, DWORD context) {
  (void)connection;
  (void)verb;
  (void)object;
  (void)version;
  (void)referrer;
  (void)acceptTypes;
  (void)flags;
  (void)context;
  SetLastError(ERROR_INTERNET_NOT_SUPPORTED);
  return nullptr;
}

inline BOOL HttpSendRequestA(HINTERNET request, const char *headers,
                             DWORD headersLength, const void *optional,
                             DWORD optionalLength) {
  (void)request;
  (void)headers;
  (void)headersLength;
  (void)optional;
  (void)optionalLength;
  SetLastError(ERROR_INTERNET_NOT_SUPPORTED);
  return FALSE;
}

inline HINTERNET InternetOpenUrlA(HINTERNET session, const char *url,
                                  const char *headers, DWORD headersLength,
                                  DWORD flags, DWORD context) {
  (void)session;
  (void)url;
  (void)headers;
  (void)headersLength;
  (void)flags;
  (void)context;
  SetLastError(ERROR_INTERNET_NOT_SUPPORTED);
  return nullptr;
}

inline HINTERNET InternetOpenUrlW(HINTERNET session, const wchar_t *url,
                                  const wchar_t *headers, DWORD headersLength,
                                  DWORD flags, DWORD context) {
  (void)session;
  (void)url;
  (void)headers;
  (void)headersLength;
  (void)flags;
  (void)context;
  SetLastError(ERROR_INTERNET_NOT_SUPPORTED);
  return nullptr;
}

inline BOOL InternetReadFile(HINTERNET file, void *buffer, DWORD toRead,
                             DWORD *read) {
  (void)file;
  (void)buffer;
  (void)toRead;
  if (read)
    *read = 0;
  SetLastError(ERROR_INTERNET_NOT_SUPPORTED);
  return FALSE;
}

inline BOOL HttpAddRequestHeadersA(HINTERNET request, const char *headers,
                                  DWORD headersLength, DWORD modifiers) {
  (void)request;
  (void)headers;
  (void)headersLength;
  (void)modifiers;
  SetLastError(ERROR_INTERNET_NOT_SUPPORTED);
  return FALSE;
}

inline BOOL HttpQueryInfoA(HINTERNET request, DWORD infoLevel, void *buffer,
                           DWORD *bufferLength, DWORD *index) {
  (void)request;
  (void)infoLevel;
  if (buffer)
    *static_cast<char *>(buffer) = '\0';
  if (bufferLength)
    *bufferLength = 0;
  (void)index;
  SetLastError(ERROR_INTERNET_NOT_SUPPORTED);
  return FALSE;
}

inline BOOL InternetCloseHandle(HINTERNET handle) {
  (void)handle;
  return TRUE;
}

#endif // PW153_PLATFORM_WININET_H
