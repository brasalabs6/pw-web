/*
 * FILE: exdisp.h (platform seam)
 *
 * DESCRIPTION: The WebBrowser interface surface the browser-dialog wrapper
 * uses (IWebBrowser2, IDispatch). The documented method signatures; the
 * browser-dialog adapter (an HTML overlay in the UI cycle) implements the
 * navigation over a real embedded web view.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_EXDISP_H
#define PW153_PLATFORM_EXDISP_H

#include "unknwn.h"
#include "wtypes.h"

//\tIDispatch: the COM automation base (the event sink's entry point).
struct IDispatch : IUnknown {
  virtual HRESULT GetTypeInfoCount(unsigned int *count) = 0;
  virtual HRESULT Invoke(long dispatchId, const void *riid, unsigned int lcid,
                         unsigned short flags, void *params,
                         void *result, void *exceptionInfo,
                         unsigned int *argError) = 0;
};

//\tIWebBrowser2: the documented navigation surface the wrapper drives.
struct IWebBrowser2 : IDispatch {
  virtual HRESULT GoBack() = 0;
  virtual HRESULT GoForward() = 0;
  virtual HRESULT Refresh() = 0;
  virtual HRESULT Stop() = 0;
  virtual HRESULT Navigate(BSTR url, VARIANT *flags, VARIANT *targetFrame,
                           VARIANT *postData, VARIANT *headers) = 0;
  virtual HRESULT get_Busy(VARIANT_BOOL *busy) = 0;
  virtual HRESULT get_Document(IDispatch **document) = 0;
  virtual HRESULT get_LocationURL(BSTR *url) = 0;
  virtual HRESULT get_ReadyState(long *state) = 0;	//	the original passed a READYSTATE* through this long* (MSVC pun).
  virtual HRESULT put_Visible(VARIANT_BOOL visible) = 0;
  virtual HRESULT put_Silent(VARIANT_BOOL silent) = 0;
  virtual HRESULT put_Width(long width) = 0;
  virtual HRESULT put_Height(long height) = 0;
  virtual HRESULT put_Left(long left) = 0;
  virtual HRESULT put_Top(long top) = 0;
  virtual HRESULT Quit() = 0;
};

//\tThe HTML document surface the wrapper's UI handler uses (the
//\tdocumented MSHTML interfaces; the adapter supplies the real view).
struct IHTMLStyle : IDispatch {
  virtual HRESULT get_display(BSTR *display) = 0;
  virtual HRESULT put_display(BSTR display) = 0;
  virtual HRESULT get_width(VARIANT *width) = 0;
  virtual HRESULT put_width(VARIANT width) = 0;
  virtual HRESULT get_height(VARIANT *height) = 0;
  virtual HRESULT put_height(VARIANT height) = 0;
};
struct IHTMLStyle2 : IHTMLStyle {};
struct IHTMLStyle3 : IHTMLStyle2 {};

struct IHTMLElement : IDispatch {
  virtual HRESULT get_style(IHTMLStyle **style) = 0;
  virtual HRESULT get_innerHTML(BSTR *html) = 0;
  virtual HRESULT put_innerHTML(BSTR html) = 0;
};
struct IHTMLElement2 : IHTMLElement {
  virtual HRESULT get_scrollTop(LONG *top) = 0;
  virtual HRESULT put_scrollTop(LONG top) = 0;
  virtual HRESULT get_scrollLeft(LONG *left) = 0;
  virtual HRESULT put_scrollLeft(LONG left) = 0;
};
struct IHTMLDocument : IDispatch {};
struct IHTMLDocument2 : IHTMLDocument {
  virtual HRESULT get_body(IHTMLElement **body) = 0;
  virtual HRESULT get_all(IDispatch **all) = 0;
};
//\tThe WebBrowser class ID (the documented CLSID_InternetExplorer).
inline const void *CLSID_WebBrowser() {
  static const unsigned char id[16] = {0x88, 0x57, 0x5d, 0x4c, 0x2f, 0x45,
                                       0xcf, 0x11, 0x9f, 0x3f, 0x6f, 0x54,
                                       0x4e, 0x1a, 0x2f, 0x5d};
  return id;
}

///////////////////////////////////////////////////////////////////////////
//
//	The URL moniker surface (the documented async download interfaces the
//	explorer's progress callback implements).
//
///////////////////////////////////////////////////////////////////////////

struct IBinding : IUnknown {
  virtual HRESULT Abort() = 0;
};

struct BINDINFO;

struct PW153_COM_UUID("{79EAC9C1-BAF9-11CE-8C82-00AA004BA90B}")
IBindStatusCallback : IUnknown {
  virtual HRESULT OnStartBinding(DWORD reserved, IBinding *binding) = 0;
  virtual HRESULT GetPriority(LONG *priority) = 0;
  virtual HRESULT OnLowResource(DWORD reserved) = 0;
  virtual HRESULT OnProgress(ULONG progress, ULONG progressMax,
                             ULONG statusCode, LPCWSTR statusText) = 0;
  virtual HRESULT OnStopBinding(HRESULT result, LPCWSTR errorText) = 0;
  virtual HRESULT GetBindInfo(DWORD *flags, BINDINFO *info) = 0;
  virtual HRESULT OnDataAvailable(DWORD flags, DWORD size, FORMATETC *format,
                                  STGMEDIUM *medium) = 0;
  virtual HRESULT OnObjectAvailable(const void *riid, IUnknown *object) = 0;
};

typedef const void *REFIID;

//	The URL download progress surface (the documented moniker types).
struct BINDINFO {};
enum {
  BINDSTATUS_FINDINGRESOURCE = 1,
  BINDSTATUS_CONNECTING,
  BINDSTATUS_REDIRECTING,
  BINDSTATUS_BEGINDOWNLOADDATA,
  BINDSTATUS_DOWNLOADINGDATA,
  BINDSTATUS_ENDDOWNLOADDATA,
  BINDSTATUS_ONPROGRESS = 11,
  BINDSTATUS_ACCEPTRANGES = 12,
  BINDSTATUS_MIMETYPEAVAILABLE = 13,
  BINDSTATUS_CACHEFILENAMEAVAILABLE = 14
};

//	The browser ready states (the documented WebBrowser values).
typedef enum {
  READYSTATE_UNINITIALIZED = 0,
  READYSTATE_LOADING = 1,
  READYSTATE_LOADED = 2,
  READYSTATE_INTERACTIVE = 3,
  READYSTATE_COMPLETE = 4
} READYSTATE;

//	The dispatch interface ID (the documented IID_IDispatch GUID).
inline const void *IID_IDispatch() {
  static const unsigned char id[16] = {0x20, 0x04, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0xc0, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x46};
  return id;
}

//	The document interface IDs (the documented GUIDs).
inline const void *IID_IHTMLDocument2() {
  static const unsigned char id[16] = {0x33, 0x2c, 0x44, 0x25, 0x3e, 0x17,
                                      0xcf, 0x11, 0x9c, 0x27, 0x00, 0xaa,
                                      0x00, 0x4a, 0x3f, 0x53};
  return id;
}

inline const void *IID_IHTMLElement2() {
  static const unsigned char id[16] = {0x33, 0x2c, 0x44, 0x25, 0x3e, 0x17,
                                      0xcf, 0x11, 0x9c, 0x27, 0x00, 0xaa,
                                      0x00, 0x4a, 0x3f, 0x52};
  return id;
}

inline const void *IID_IHTMLStyle2() {
  static const unsigned char id[16] = {0x30, 0x53, 0x05, 0x30, 0x3e, 0x17,
                                      0xcf, 0x11, 0x9c, 0x27, 0x00, 0xaa,
                                      0x00, 0x4a, 0x3f, 0x53};
  return id;
}

inline const void *IID_IHTMLStyle3() {
  static const unsigned char id[16] = {0x30, 0x53, 0x05, 0x30, 0x3e, 0x17,
                                      0xcf, 0x11, 0x9c, 0x27, 0x00, 0xaa,
                                      0x00, 0x4a, 0x3f, 0x54};
  return id;
}

//	The document host UI handler (the documented MSHTML interface the
//	wrapper's UI customization implements).
struct IDocHostUIHandlerDispatch : IDispatch {};

inline const void *IID_IDocHostUIHandlerDispatch() {
  static const unsigned char id[16] = {0x4c, 0xf9, 0x68, 0x43, 0x3e, 0x17,
                                      0xcf, 0x11, 0x9c, 0x27, 0x00, 0xaa,
                                      0x00, 0x4a, 0x3f, 0x54};
  return id;
}

inline const void *LIBID_ATLLib() {
  static const unsigned char id[16] = {0x44, 0xec, 0x05, 0x3a, 0x3e, 0x17,
                                      0xcf, 0x11, 0x9c, 0x27, 0x00, 0xaa,
                                      0x00, 0x4a, 0x3f, 0x53};
  return id;
}

//	The event dispatch interface GUID (the documented DIID).
inline const void *DIID_DWebBrowserEvents2() {
  static const unsigned char id[16] = {0x3a, 0x7f, 0x1c, 0x34, 0x3e, 0x17,
                                      0xcf, 0x11, 0x9c, 0x27, 0x00, 0xaa,
                                      0x00, 0x4a, 0x3f, 0x53};
  return id;
}

//\tThe form input surface (the documented MSHTML interfaces; the
//\tcollection enumerates the document's elements and the input element
//\tcarries the form fields the login parser reads).
struct IHTMLElementCollection : IDispatch {
  virtual HRESULT get_length(long *length) = 0;
  virtual HRESULT item(VARIANT name, VARIANT index, IDispatch **item) = 0;
};
struct IHTMLInputElement : IHTMLElement {
  virtual HRESULT get_name(BSTR *name) = 0;
  virtual HRESULT get_value(BSTR *value) = 0;
};

//	The form input interface ID (the documented IID_IHTMLInputElement).
inline const void *IID_IHTMLInputElement() {
  static const unsigned char id[16] = {0x30, 0x50, 0xf5, 0xd2, 0x98, 0xb5,
                                      0xcf, 0x11, 0xbb, 0x82, 0x00, 0xaa,
                                      0x00, 0xbd, 0xce, 0x0b};
  return id;
}

#endif // PW153_PLATFORM_EXDISP_H
