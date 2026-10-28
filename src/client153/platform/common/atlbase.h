/*
 * FILE: atlbase.h (platform seam)
 *
 * DESCRIPTION: The ATL base surface the client's browser-dialog wrapper
 * (PWAtl.h/PWBrowser) compiles against. CComPtr/CComQIPtr are real COM
 * smart pointers over the unknwn.h IUnknown seam; CComBSTR/CComVariant are
 * real wrappers over the OLE string types; the conversion macros route
 * through the strict UTF-8/UTF-16 platform conversions. COM activation
 * resolves through the adapter registry by class ID - the browser-dialog
 * adapter (an HTML overlay in the UI cycle) registers the web view class;
 * unknown classes fail explicitly.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_ATLBASE_H
#define PW153_PLATFORM_ATLBASE_H

#include "Windows.h"
#include "unknwn.h"
#include "wtypes.h"

//	The COM result codes (the documented values).
#ifndef S_OK
#define S_OK 0
#endif
#ifndef E_FAIL
#define E_FAIL 0x80004005
#endif
#ifndef E_NOTIMPL
#define E_NOTIMPL 0x80004001
#endif
#ifndef S_FALSE
#define S_FALSE 1
#endif

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

///////////////////////////////////////////////////////////////////////////
//
//	The OLE string allocator (documented BSTR semantics: length prefix).
//
///////////////////////////////////////////////////////////////////////////

inline BSTR SysAllocString(const wchar_t *text) {
  if (!text)
    return nullptr;
  std::size_t length = 0;
  while (text[length])
    ++length;
  auto *header = static_cast<std::uint32_t *>(std::malloc(sizeof(std::uint32_t) + (length + 1) * sizeof(wchar_t)));
  if (!header)
    return nullptr;
  *header = static_cast<std::uint32_t>(length);
  auto *data = reinterpret_cast<wchar_t *>(header + 1);
  std::memcpy(data, text, (length + 1) * sizeof(wchar_t));
  return data;
}

inline void SysFreeString(BSTR text) {
  if (text)
    std::free(reinterpret_cast<std::uint32_t *>(text) - 1);
}

inline std::uint32_t SysStringLen(BSTR text) {
  if (!text)
    return 0;
  return *(reinterpret_cast<std::uint32_t *>(text) - 1);
}

///////////////////////////////////////////////////////////////////////////
//
//	Conversion macros: the original allocated a stack buffer per use; the
//	port routes through the strict conversions with the same contract.
//
///////////////////////////////////////////////////////////////////////////

inline LPWSTR A2W_helper(const char *text) {
  thread_local static std::wstring buffer;
  if (!pw153::platform::utf8_to_utf16(text ? text : "", buffer))
    buffer.clear();
  return const_cast<LPWSTR>(buffer.c_str());
}
inline LPSTR W2A_helper(const wchar_t *text) {
  thread_local static std::string buffer;
  if (!pw153::platform::utf16_to_utf8(text ? text : L"", buffer))
    buffer.clear();
  return const_cast<LPSTR>(buffer.c_str());
}

#define USES_CONVERSION ((void)0)
#define A2W(text) A2W_helper(text)
#define W2A(text) W2A_helper(text)
#define T2W(text) A2W(text)
#define W2T(text) W2A(text)
#define T2A(text) (const_cast<LPSTR>(text))
#define A2T(text) (const_cast<LPSTR>(text))
#define OLE2A(text) W2A(text)
#define A2CW(text) A2W(text)
#define W2CW(text) W2A(text)
#define T2CW(text) A2W(text)
#define OLE2CW(text) A2W(text)
#define A2OLE(text) A2W(text)
#define OLE2T(text) W2A(text)
#define T2OLE(text) A2W(text)

///////////////////////////////////////////////////////////////////////////
//
//	CComBSTR / CComVariant: real wrappers over the OLE types.
//
///////////////////////////////////////////////////////////////////////////

class CComBSTR {
public:
  CComBSTR() : m_str(nullptr) {}
  CComBSTR(const wchar_t *text) : m_str(SysAllocString(text)) {}
  CComBSTR(const CComBSTR &other) : m_str(SysAllocString(other.m_str)) {}
  ~CComBSTR() { SysFreeString(m_str); }
  CComBSTR &operator=(const CComBSTR &other) {
    if (this != &other) {
      SysFreeString(m_str);
      m_str = SysAllocString(other.m_str);
    }
    return *this;
  }
  operator BSTR() const { return m_str; }
  BSTR *operator&() { return &m_str; }
  BSTR m_str;
};

class CComVariant : public tagVARIANT {
public:
  CComVariant() { vt = VT_EMPTY; }
  CComVariant(int value) { vt = VT_I4; lVal = value; }
  CComVariant(const wchar_t *value) { vt = VT_BSTR; bstrVal = SysAllocString(value); }
  ~CComVariant() { Clear(); }
  void Clear() {
    if (vt == VT_BSTR && bstrVal) {
      SysFreeString(bstrVal);
      bstrVal = nullptr;
    }
    vt = VT_EMPTY;
  }
  CComVariant &operator=(int value) { Clear(); vt = VT_I4; lVal = value; return *this; }
};

///////////////////////////////////////////////////////////////////////////
//
//	COM activation: the adapter registry resolves by class ID.
//
///////////////////////////////////////////////////////////////////////////

HRESULT __CoCreateInstance(const void *classId, void **result);
HRESULT CoInitialize(void *reserved);
void CoUninitialize();

///////////////////////////////////////////////////////////////////////////
//
//	CComPtr / CComQIPtr: real COM smart pointers.
//
///////////////////////////////////////////////////////////////////////////

template <class T>
class CComPtr {
  T *pointer_ = nullptr;

public:
  CComPtr() = default;
  CComPtr(T *pointer) : pointer_(pointer) {
    if (pointer_)
      pointer_->AddRef();
  }
  CComPtr(const CComPtr &other) : pointer_(other.pointer_) {
    if (pointer_)
      pointer_->AddRef();
  }
  ~CComPtr() {
    if (pointer_)
      pointer_->Release();
  }
  CComPtr &operator=(const CComPtr &other) {
    if (this != &other) {
      if (other.pointer_)
        other.pointer_->AddRef();
      if (pointer_)
        pointer_->Release();
      pointer_ = other.pointer_;
    }
    return *this;
  }
  CComPtr &operator=(T *pointer) {
    if (pointer != pointer_) {
      if (pointer)
        pointer->AddRef();
      if (pointer_)
        pointer_->Release();
      pointer_ = pointer;
    }
    return *this;
  }
  operator T *() const { return pointer_; }
  T &operator*() const { return *pointer_; }
  T *operator->() const { return pointer_; }
  T **operator&() { return &pointer_; }
  bool operator!() const { return pointer_ == nullptr; }
  bool operator==(T *pointer) const { return pointer_ == pointer; }
  void Release() {
    if (pointer_) {
      pointer_->Release();
      pointer_ = nullptr;
    }
  }
  void Attach(T *pointer) {
    if (pointer_)
      pointer_->Release();
    pointer_ = pointer;
  }
  T *Detach() {
    T *result = pointer_;
    pointer_ = nullptr;
    return result;
  }
  //\tCOM activation: the adapter registry resolves the class ID and
  //\treturns the interface this smart pointer holds.
  HRESULT CoCreateInstance(const void *classId) {
    return __CoCreateInstance(classId, reinterpret_cast<void **>(&pointer_));
  }
};

template <class T>
class CComQIPtr {
  T *pointer_ = nullptr;

public:
  CComQIPtr() = default;
  CComQIPtr(IUnknown *unknown) {
    if (unknown)
      unknown->QueryInterface(nullptr, reinterpret_cast<void **>(&pointer_));
  }
  //	Converting constructor from CComPtr<IUnknown> (the original ATL
  //	assignment pattern the wrapper uses).
  CComQIPtr(const CComPtr<IUnknown> &unknown) {
    T *raw = static_cast<T *>(static_cast<IUnknown *>(unknown));
    if (raw)
      raw->AddRef();
    pointer_ = raw;
  }
  operator T *() const { return pointer_; }
  T *operator->() const { return pointer_; }
  T **operator&() { return &pointer_; }
  void Release() {
    if (pointer_) {
      pointer_->Release();
      pointer_ = nullptr;
    }
  }
  T *Detach() {
    T *result = pointer_;
    pointer_ = nullptr;
    return result;
  }
};

//	The ATL declaration macros (no-op expansions of the MSVC attributes).
#define COM_INTERFACE_ENTRY_IID(iid, interface)
#define BEGIN_SINK_MAP(theClass)
#define SINK_ENTRY_EX(...)
#define END_SINK_MAP()

#define BEGIN_COM_MAP(theClass)
#define COM_INTERFACE_ENTRY(interface)
#define END_COM_MAP()

#define ATL_NO_VTABLE
#define ATL_API
#define DECLARE_PROTECT_FINAL_CONSTRUCT()

///////////////////////////////////////////////////////////////////////////
//
//	CComObject: the ATL aggregation-safe object wrapper (the documented
//	creation/destruction semantics over the real AddRef/Release pair).
//
///////////////////////////////////////////////////////////////////////////

template <class T>
class CComObject : public T {
public:
  static CComObject<T> *CreateInstance() { return new CComObject<T>(); }
};

template <class T>
class CComObjectStack : public T {};

///////////////////////////////////////////////////////////////////////////
//
//	IDispatchImpl: the ATL dispatch implementation base (the documented
//	template parameters; the adapter supplies the real type library).
//
///////////////////////////////////////////////////////////////////////////

template <class T, const void *iid = nullptr, const void *libId = nullptr>
class IDispatchImpl {};

///////////////////////////////////////////////////////////////////////////
//
//	IDispEventImpl: the ATL event sink base (the documented sink template;
//	the adapter binds the sink by type).
//
///////////////////////////////////////////////////////////////////////////

template <unsigned int id, class T, class DispInterface = void,
          const void *eventGuid = nullptr, const void *libId = nullptr,
          unsigned int major = 0, unsigned int minor = 0>
class IDispEventImpl {
public:
  //	Event binding: the adapter registry resolves the sink; without the
  //	adapter the binding fails explicitly.
  HRESULT DispEventAdvise(IUnknown *unknown) {
    (void)unknown;
    return E_FAIL;
  }
  HRESULT DispEventUnadvise(IUnknown *unknown) {
    (void)unknown;
    return E_FAIL;
  }
};

///////////////////////////////////////////////////////////////////////////
//
//	CComObjectRootEx: the ATL object root (the documented threading-model
//	template over the real reference count).
//
///////////////////////////////////////////////////////////////////////////

template <class ThreadingModel>
class CComObjectRootEx {
public:
  ULONG m_refCount = 0;
  const void *m_libid = nullptr;	//	the type library ID (the adapter supplies it).
  const void *m_iid = nullptr;	//	the source interface ID.
  unsigned short m_wMajorVerNum = 0;
  unsigned short m_wMinorVerNum = 0;
  ULONG InternalAddRef() { return ++m_refCount; }
  ULONG InternalRelease() { return m_refCount ? --m_refCount : m_refCount; }
  void Lock() {}
  void Unlock() {}
};

///////////////////////////////////////////////////////////////////////////
//
//	CComSingleThreadModel: the ATL threading model (the documented
//	increment/decrement over the real reference count).
//
///////////////////////////////////////////////////////////////////////////

class CComSingleThreadModel {
public:
  static ULONG Increment(ULONG *value) {
    return ++*value;
  }
  static ULONG Decrement(ULONG *value) {
    return --*value;
  }
};

///////////////////////////////////////////////////////////////////////////
//
//	The event-binding surface: the adapter registry resolves the source
//	interface; without the adapter the binding fails explicitly.
//
///////////////////////////////////////////////////////////////////////////

inline HRESULT AtlGetObjectSourceInterface(IUnknown *unknown, const void **libId,
                                           const void **iid, unsigned short *major,
                                           unsigned short *minor) {
  (void)unknown;
  if (libId)
    *libId = nullptr;
  if (iid)
    *iid = nullptr;
  if (major)
    *major = 0;
  if (minor)
    *minor = 0;
  return E_FAIL;
}

inline HRESULT DispEventAdvise(IUnknown *unknown) {
  (void)unknown;
  return E_FAIL;
}

inline HRESULT DispEventUnadvise(IUnknown *unknown) {
  (void)unknown;
  return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////
//
//	CComModule: the ATL module root (the original declared _Module).
//
///////////////////////////////////////////////////////////////////////////

//	The ATL 3.0 object map: the static descriptor of the module's COM
//	classes. The port's activation registry (atlcom_registry.cpp)
//	resolves classes; the map entries carry the class id pointer.
typedef struct _ATL_OBJMAP_ENTRY {
  const void *pclsid;
} _ATL_OBJMAP_ENTRY;

#define BEGIN_OBJECT_MAP(x) static _ATL_OBJMAP_ENTRY x[] = {
#define END_OBJECT_MAP() \
  {nullptr}};

class CComModule {
public:
  HRESULT Init(void *parameters, HINSTANCE instance) {
    (void)parameters;
    (void)instance;
    return S_OK;
  }
  void Term() {}
};

#endif // PW153_PLATFORM_ATLBASE_H
