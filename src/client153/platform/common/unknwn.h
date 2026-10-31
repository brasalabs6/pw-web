/*
 * FILE: unknwn.h (platform seam)
 *
 * DESCRIPTION: The COM base interface. The SDK headers include it for
 * IUnknown; the port's interface seams already derive the same three-method
 * surface, so this declares the canonical base.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_UNKNWN_H
#define PW153_PLATFORM_UNKNWN_H

#include "Windows.h"

#define __RPC_FAR
#define STDMETHODCALLTYPE

#define STDMETHOD(method) virtual HRESULT method
#define STDMETHOD_(type, method) virtual type method
#define STDMETHODIMP HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type) type STDMETHODCALLTYPE
//	STDAPI/STDAPI_: the free-function forms (the seam's __stdcall is
//	empty; the linkage is the caller's, like STDMETHODIMP above).
#define STDAPI HRESULT
#define STDAPI_(type) type
#define PURE = 0
#define __stdcall
#define STDMETHODCALLTYPE

//	The MIDL interface declaration forms the SDK headers use (the
//	DirectShow base classes declare INonDelegatingUnknown with them).
#ifndef DECLARE_INTERFACE
#define DECLARE_INTERFACE(iface) struct iface
#define DECLARE_INTERFACE_(iface, base) struct iface : base
#endif
#ifndef THIS
#define THIS void
#define THIS_
#endif

#endif // PW153_PLATFORM_UNKNWN_H
