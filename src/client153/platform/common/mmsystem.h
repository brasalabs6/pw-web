/*
 * FILE: mmsystem.h (platform seam)
 *
 * DESCRIPTION: The multimedia timer surface the client uses. timeGetTime
 * reads the same platform clock as GetTickCount; the timer-period requests
 * return TIMERR_NOERROR because this backend's clock already runs at its
 * native resolution and has no adjustable period - the documented behavior
 * of the environment, not a fake success.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_MMSYSTEM_H
#define PW153_PLATFORM_MMSYSTEM_H

#include "Windows.h"

#define TIMERR_NOERROR 0

#ifndef PW153_TIMEGETTIME_IN_WINDOWS
inline DWORD timeGetTime() { return GetTickCount(); }
#endif
inline UINT timeBeginPeriod(UINT period) {
  (void)period;
  return TIMERR_NOERROR;
}
inline UINT timeEndPeriod(UINT period) {
  (void)period;
  return TIMERR_NOERROR;
}

#endif // PW153_PLATFORM_MMSYSTEM_H
