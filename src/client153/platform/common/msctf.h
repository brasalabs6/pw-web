/*
 * FILE: msctf.h (platform seam)
 *
 * DESCRIPTION: The TSF (Text Services Framework) surface the client's
 * IME code compiles against. The browser target's IME belongs to the
 * input adapter, so the COM activation fails explicitly (the registry
 * is empty) and the SUCCEEDED guards in the caller take the non-TSF
 * path. The interface UUIDs are the documented msctf/ctfutb values;
 * the coclass ids are inert here (activation fails before any class
 * id is consulted) and carry zero bytes pending a consumer that
 * actually resolves them.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_MSCTF_H
#define PW153_PLATFORM_MSCTF_H

#include "Windows.h"
#include "unknwn.h"	//	IUnknown, GUID, PW153_COM_UUID.
#include "wtypes.h"	//	BSTR and the OLE types.

//	The TSF handles (the documented msctf typedefs).
typedef DWORD TfClientId;
typedef DWORD TfEditCookie;
typedef DWORD TfGuidAtom;

//	The thread manager activation flags (the documented msctf values).
#define TF_TMAE_NOACTIVATETIP 0x00000001
#define TF_TMAE_SECUREMODE 0x00000002
#define TF_TMAE_UIELEMENTENABLEDONLY 0x00000004
#define TF_TMAE_COMLESS 0x00000008
#define TF_TMAE_WOW16 0x00000010
#define TF_TMAE_NOACTIVATEKEYBOARDLAYOUT 0x00000020
#define TF_TMAE_CONSOLE 0x00000040

//	The language bar float styles (the documented ctfutb enumeration).
#define TF_SFT_SHOWNORMAL 0x00000000
#define TF_SFT_DESKBAND 0x00000001
#define TF_SFT_FLOATING 0x00000002
#define TF_SFT_MINIMIZED 0x00000003
#define TF_SFT_HIDDEN 0x00000004

//	The invalid advise cookie (the documented msctf value).
#define TF_INVALID_COOKIE 0xffffffff

//	The class context (the documented COM value).
#ifndef CLSCTX_INPROC_SERVER
#define CLSCTX_INPROC_SERVER 0x1
#endif

//	The thread manager (the documented interface; the methods the
// client calls, with the documented signatures).
struct ITfDocumentMgr;
struct ITfContext;

struct PW153_COM_UUID("{aa80e801-2021-11d2-93e0-0060b067b86e}")
ITfThreadMgr : IUnknown {
  virtual HRESULT STDMETHODCALLTYPE Activate(TfClientId *clientId) = 0;
  virtual HRESULT STDMETHODCALLTYPE Deactivate() = 0;
  virtual HRESULT STDMETHODCALLTYPE
  CreateDocumentMgr(ITfDocumentMgr **documentManager) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  GetFocus(ITfDocumentMgr **documentManager) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  SetFocus(ITfDocumentMgr *documentManager) = 0;
  virtual HRESULT STDMETHODCALLTYPE
  AssociateFocus(HWND window, ITfDocumentMgr *newDocumentManager,
                 ITfDocumentMgr **previousDocumentManager) = 0;
};

//	The document manager (the documented interface; the client uses the
// type and the enumeration below is the documented core).
struct PW153_COM_UUID("{aa80e7f4-2021-11d2-93e0-0060b067b86e}")
ITfDocumentMgr : IUnknown {
  virtual HRESULT STDMETHODCALLTYPE CreateContext(
      TfEditCookie cookie, DWORD flags, IUnknown *punk,
      ITfContext **context, TfEditCookie *editCookie) = 0;
  virtual HRESULT STDMETHODCALLTYPE Push(ITfContext *context) = 0;
  virtual HRESULT STDMETHODCALLTYPE Pop(DWORD flags) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetTop(ITfContext **context) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetBase(ITfContext **context) = 0;
};

//	The context (the client uses the type only; the input adapter owns
// the real editing surface).
struct PW153_COM_UUID("{2433bf8e-0f9b-435c-ba2c-180611978c30}")
ITfContext : IUnknown {};

//	The event source (the documented sink advise surface; the sink id
//	arrives as the __uuidof GUID reference, the documented REFIID form).
struct PW153_COM_UUID("{4ea48a35-60ae-446f-8fd6-e6a8d82459f7}")
ITfSource : IUnknown {
  virtual HRESULT STDMETHODCALLTYPE AdviseSink(const GUID &sinkId,
                                               IUnknown *unknown,
                                               DWORD *cookie) = 0;
  virtual HRESULT STDMETHODCALLTYPE UnadviseSink(DWORD cookie) = 0;
};

//	The language bar manager (the client calls ShowFloating and reads
//	the float status).
struct PW153_COM_UUID("{87955690-e627-11d2-8ddb-00105a2799b5}")
ITfLangBarMgr : IUnknown {
  virtual HRESULT STDMETHODCALLTYPE ShowFloating(DWORD style) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetShowFloatingStatus(DWORD *status) = 0;
};

//	The input processor profiles (the client reads the current language
//	and the default profile description).
struct PW153_COM_UUID("{1f02b6c5-7842-4ee6-8a0b-9a24183a95ca}")
ITfInputProcessorProfiles : IUnknown {
  virtual HRESULT STDMETHODCALLTYPE GetCurrentLanguage(LANGID *langid) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDefaultLanguageProfile(
      LANGID langid, const GUID &category, CLSID *profile,
      GUID *profileGuid) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetLanguageProfileDescription(
      const CLSID &profile, LANGID langid, const GUID &profileGuid,
      BSTR *description) = 0;
};

//	The active language profile notify sink (the client's sink passes
// this id to AdviseSink).
struct PW153_COM_UUID("{b246cb75-a93e-4652-bf8c-b3fe0cfd7e57}")
ITfActiveLanguageProfileNotifySink : IUnknown {};

//	The compartments (the client's sink derives the event sink and uses
// the manager/compartment types).
struct PW153_COM_UUID("{743abd5f-f26d-48df-8cc5-238492419b64}")
ITfCompartmentEventSink : IUnknown {};

struct PW153_COM_UUID("{bb08f7a9-607a-4384-8623-056892b64371}")
ITfCompartment : IUnknown {
  virtual HRESULT STDMETHODCALLTYPE GetValue(VARIANT *value) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetValue(VARIANT *value) = 0;
};

struct PW153_COM_UUID("{7dcf57ac-18ad-438b-824d-979bffb74b7c}")
ITfCompartmentMgr : IUnknown {
  virtual HRESULT STDMETHODCALLTYPE GetCompartment(REFGUID guidCompartment,
                                                   ITfCompartment **compartment) = 0;
};

//	The TSF coclass ids: inert on the browser target (the activation
//	registry is empty and CoCreateInstance fails before consulting any
//	class id); zero bytes pending a consumer that resolves them. Declared
//	as the original's const CLSID variables so the client's un-parenthesized
//	call sites compile unchanged.
inline const GUID CLSID_TF_ThreadMgr = {};
inline const GUID CLSID_TF_LangBarMgr = {};
inline const GUID CLSID_TF_InputProcessorProfiles = {};

//	The TSF keyboard category id: inert for the same reason (the
//	profile enumeration never runs without an activated thread
//	manager).
inline const GUID GUID_TFCAT_TIP_KEYBOARD = {};

//	The compartment ids: the open/convert mode compartments are inert
//	for the same reason (the compartment queries never succeed without
//	an activated thread manager).
inline const GUID GUID_COMPARTMENT_KEYBOARD_OPENCLOSE = {};
inline const GUID GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION = {};

//	The interface ids (the documented bytes; GUID variables so the
//	AdviseSink/GetCompartment reference forms bind directly - the
//	QueryInterface call sites cast to the seam's opaque id pointer).
inline const GUID IID_ITfCompartmentMgr = {0x7dcf57ac, 0x18ad, 0x438b,
                                           {0x82, 0x4d, 0x97, 0x9b, 0xff,
                                            0xb7, 0x4b, 0x7c}};
inline const GUID IID_ITfSource = {0x4ea48a35, 0x60ae, 0x446f,
                                   {0x8f, 0xd6, 0xe6, 0xa8, 0xd8, 0x24,
                                    0x59, 0xf7}};
inline const GUID IID_ITfCompartmentEventSink = {0x743abd5f, 0xf26d, 0x48df,
                                                 {0x8c, 0xc5, 0x23, 0x84,
                                                  0x92, 0x41, 0x9b, 0x64}};

#endif // PW153_PLATFORM_MSCTF_H
