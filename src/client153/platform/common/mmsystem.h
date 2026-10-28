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

//	Multimedia timer types (the documented MMRESULT and callback form;
//	the base-class helper declares its compatible timer entry through
//	them).
typedef UINT MMRESULT;
typedef void (*LPTIMECALLBACK)(UINT uID, UINT uMsg, DWORD_PTR dwUser,
                               DWORD_PTR dw1, DWORD_PTR dw2);

#ifndef PW153_TIMEGETTIME_IN_WINDOWS
inline DWORD timeGetTime() { return GetTickCount(); }
#endif

//	Four-character-code builder (the documented mmsystem.h macro; the
//	wave-file loader matches RIFF chunk ids with it).
#define mmioFOURCC(ch0, ch1, ch2, ch3)                                   \
  ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |                      \
   ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))
inline UINT timeBeginPeriod(UINT period) {
  (void)period;
  return TIMERR_NOERROR;
}
inline UINT timeEndPeriod(UINT period) {
  (void)period;
  return TIMERR_NOERROR;
}

#endif // PW153_PLATFORM_MMSYSTEM_H
