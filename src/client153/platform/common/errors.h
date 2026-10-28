/*
 * FILE: errors.h (platform seam)
 *
 * DESCRIPTION: The DirectShow status codes (the SDK's errors.h). The
 * vendored base classes reference the failure codes only in guarded or
 * commented paths; the seam defines the documented VFW_E family (the
 * documented vfwmsgs values) the media cycle's adapter reports so future
 * callers compile against the documented values.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_ERRORS_H
#define PW153_PLATFORM_ERRORS_H

#include "Windows.h"

#define VFW_E_ALREADY_CONNECTED 0x80040204
#define VFW_E_NO_ACCEPTABLE_TYPES 0x80040207
#define VFW_E_NO_ALLOCATOR 0x8004020A
#define VFW_E_RUNTIME_ERROR 0x8004020B
#define VFW_E_BUFFER_NOTSET 0x8004020C
#define VFW_E_NOT_FOUND 0x80040216
#define VFW_E_CANNOT_CONNECT 0x80040217
#define VFW_E_CANNOT_RENDER 0x80040218
#define VFW_E_STATE_CHANGED 0x80040223
#define VFW_E_WRONG_STATE 0x80040227
#define VFW_E_INVALID_RECT 0x80040229
#define VFW_E_TYPE_NOT_ACCEPTED 0x8004022A
#define VFW_E_SAMPLE_REJECTED 0x8004022B
#define VFW_E_TIMEOUT 0x8004022E
#define VFW_E_NO_TRANSPORT 0x80040266

#endif // PW153_PLATFORM_ERRORS_H
