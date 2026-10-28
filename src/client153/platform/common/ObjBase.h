/*
 * FILE: ObjBase.h (platform seam)
 *
 * DESCRIPTION: The COM object base header (the real ObjBase.h carries the
 * IUnknown surface and the activation entry points); the port's unknwn.h
 * and Windows.h own them. This case-variant forwarder exists because the
 * aaudio SDK headers include <ObjBase.h> by its Windows-cased name.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_OBJBASE_H
#define PW153_PLATFORM_OBJBASE_H

#include "Windows.h"
#include "unknwn.h"

#endif // PW153_PLATFORM_OBJBASE_H
