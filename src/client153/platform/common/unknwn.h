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

struct IUnknown {
  virtual ~IUnknown() = default;
  virtual HRESULT QueryInterface(const void *riid, void **ppvObject) = 0;
  virtual ULONG AddRef() = 0;
  virtual ULONG Release() = 0;
};

#endif // PW153_PLATFORM_UNKNWN_H
