/*
 * FILE: evcode.h (platform seam)
 *
 * DESCRIPTION: The DirectShow event codes (the SDK's evcode.h). The
 * client's event pump switches on the completion/abort codes (already
 * defined by the dshow.h seam with the same documented values); the
 * renderer base classes post the repaint/display codes. Guarded defines
 * so either seam can load first.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_EVCODE_H
#define PW153_PLATFORM_EVCODE_H

#include "Windows.h"

#ifndef EC_COMPLETE
#define EC_COMPLETE 0x00000001
#endif
#ifndef EC_USERABORT
#define EC_USERABORT 0x00000002
#endif
#ifndef EC_ERRORABORT
#define EC_ERRORABORT 0x00000003
#endif
#ifndef EC_REPAINT
#define EC_REPAINT 0x0000000A
#endif
#ifndef EC_DISPLAY_CHANGED
#define EC_DISPLAY_CHANGED 0x00000010
#endif
#ifndef EC_DEVICE_LOST
#define EC_DEVICE_LOST 0x0000001F
#endif
#ifndef EC_PAUSED
#define EC_PAUSED 0x000000E
#endif

#endif // PW153_PLATFORM_EVCODE_H
