/*
 * FILE: control.h (platform seam)
 *
 * DESCRIPTION: The DirectShow control interfaces (the SDK's control.h).
 * The media-level surfaces the client drives (IMediaControl, IMediaEvent,
 * IMediaPosition, IBasicAudio) live in the dshow.h seam; this header adds
 * the windowed-renderer control surfaces the vendored base classes expose
 * (CBaseVideoWindow/CBaseBasicVideo derive from them). The documented COM
 * signatures; the browser video adapter implements them in the media
 * cycle.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_CONTROL_H
#define PW153_PLATFORM_CONTROL_H

#include "strmif.h"

//	Windowed video control (the documented IVideoWindow surface; the
//	automation methods the base classes declare arrive through the
//	derived classes).
struct IVideoWindow : IUnknown {
  virtual HRESULT put_Caption(const wchar_t *caption) = 0;
  virtual HRESULT get_Caption(wchar_t **caption) = 0;
  virtual HRESULT put_WindowStyle(long windowStyle) = 0;
  virtual HRESULT get_WindowStyle(long *windowStyle) = 0;
  virtual HRESULT put_Visible(long visible) = 0;
  virtual HRESULT get_Visible(long *visible) = 0;
  virtual HRESULT SetWindowPosition(long left, long top, long width,
                                    long height) = 0;
};

//	Basic video control (the documented IBasicVideo surface).
struct IBasicVideo : IUnknown {
  virtual HRESULT get_VideoWidth(long *pVideoWidth) = 0;
  virtual HRESULT get_VideoHeight(long *pVideoHeight) = 0;
  virtual HRESULT put_DestinationLeft(long DestinationLeft) = 0;
  virtual HRESULT get_DestinationLeft(long *pDestinationLeft) = 0;
  virtual HRESULT put_DestinationTop(long DestinationTop) = 0;
  virtual HRESULT get_DestinationTop(long *pDestinationTop) = 0;
  virtual HRESULT put_DestinationWidth(long DestinationWidth) = 0;
  virtual HRESULT get_DestinationWidth(long *pDestinationWidth) = 0;
  virtual HRESULT put_DestinationHeight(long DestinationHeight) = 0;
  virtual HRESULT get_DestinationHeight(long *pDestinationHeight) = 0;
  virtual HRESULT SetDestinationPosition(long Left, long Top, long Width,
                                         long Height) = 0;
  virtual HRESULT GetVideoSize(long *pWidth, long *pHeight) = 0;
};

//	Basic video 2 (the documented IBasicVideo2 extension).
struct IBasicVideo2 : IBasicVideo {
  virtual HRESULT GetPreferredAspectRatio(long *plAspectX,
                                          long *plAspectY) = 0;
};

//	Deferred command (the documented IDeferredCommand surface the
//	command queue hands out).
struct IDeferredCommand : IUnknown {
  virtual HRESULT Cancel() = 0;
  virtual HRESULT Confidence(LONG *pConfidence) = 0;
  virtual HRESULT Postpone(REFTIME newtime) = 0;
  virtual HRESULT GetHResult(HRESULT *phrResult) = 0;
};

#endif // PW153_PLATFORM_CONTROL_H
