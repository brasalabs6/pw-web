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

//	Windowed video control: the complete documented IVideoWindow
//	contract (Wine control.idl, in declaration order): the caption,
//	style, visibility, position, ownership and cursor surface. The
//	browser media adapter owns the runtime.
struct IVideoWindow : IUnknown {
  virtual HRESULT put_Caption(const wchar_t *caption) = 0;
  virtual HRESULT get_Caption(wchar_t **caption) = 0;
  virtual HRESULT put_WindowStyle(long windowStyle) = 0;
  virtual HRESULT get_WindowStyle(long *windowStyle) = 0;
  virtual HRESULT put_WindowStyleEx(long windowStyleEx) = 0;
  virtual HRESULT get_WindowStyleEx(long *windowStyleEx) = 0;
  virtual HRESULT put_AutoShow(long autoShow) = 0;
  virtual HRESULT get_AutoShow(long *autoShow) = 0;
  virtual HRESULT put_WindowState(long windowState) = 0;
  virtual HRESULT get_WindowState(long *windowState) = 0;
  virtual HRESULT put_BackgroundPalette(long backgroundPalette) = 0;
  virtual HRESULT get_BackgroundPalette(long *backgroundPalette) = 0;
  virtual HRESULT put_Visible(long visible) = 0;
  virtual HRESULT get_Visible(long *visible) = 0;
  virtual HRESULT put_Left(long left) = 0;
  virtual HRESULT get_Left(long *left) = 0;
  virtual HRESULT put_Width(long width) = 0;
  virtual HRESULT get_Width(long *width) = 0;
  virtual HRESULT put_Top(long top) = 0;
  virtual HRESULT get_Top(long *top) = 0;
  virtual HRESULT put_Height(long height) = 0;
  virtual HRESULT get_Height(long *height) = 0;
  virtual HRESULT put_Owner(OAHWND hwnd) = 0;
  virtual HRESULT get_Owner(OAHWND *hwnd) = 0;
  virtual HRESULT put_MessageDrain(OAHWND hwnd) = 0;
  virtual HRESULT get_MessageDrain(OAHWND *hwnd) = 0;
  virtual HRESULT put_BorderColor(long color) = 0;
  virtual HRESULT get_BorderColor(long *color) = 0;
  virtual HRESULT put_FullScreenMode(long fullScreenMode) = 0;
  virtual HRESULT get_FullScreenMode(long *fullScreenMode) = 0;
  virtual HRESULT SetWindowForeground(long focus) = 0;
  virtual HRESULT SetWindowPosition(long left, long top, long width,
                                    long height) = 0;
  virtual HRESULT GetWindowPosition(long *left, long *top, long *width,
                                     long *height) = 0;
  virtual HRESULT GetMinIdealImageSize(long *width, long *height) = 0;
  virtual HRESULT GetMaxIdealImageSize(long *width, long *height) = 0;
  virtual HRESULT HideCursor(long hideCursor) = 0;
  virtual HRESULT IsCursorHidden(long *cursorHidden) = 0;
};

//	Basic video control: the complete documented IBasicVideo contract
//	(Wine control.idl, in declaration order).
struct IBasicVideo : IUnknown {
  virtual HRESULT get_AvgTimePerFrame(long long *pAvgTimePerFrame) = 0;
  virtual HRESULT get_BitRate(long *pBitRate) = 0;
  virtual HRESULT get_BitErrorRate(long *pBitErrorRate) = 0;
  virtual HRESULT get_VideoWidth(long *pVideoWidth) = 0;
  virtual HRESULT get_VideoHeight(long *pVideoHeight) = 0;
  virtual HRESULT put_SourceLeft(long sourceLeft) = 0;
  virtual HRESULT get_SourceLeft(long *sourceLeft) = 0;
  virtual HRESULT put_SourceWidth(long sourceWidth) = 0;
  virtual HRESULT get_SourceWidth(long *sourceWidth) = 0;
  virtual HRESULT put_SourceTop(long sourceTop) = 0;
  virtual HRESULT get_SourceTop(long *sourceTop) = 0;
  virtual HRESULT put_SourceHeight(long sourceHeight) = 0;
  virtual HRESULT get_SourceHeight(long *sourceHeight) = 0;
  virtual HRESULT put_DestinationLeft(long destinationLeft) = 0;
  virtual HRESULT get_DestinationLeft(long *destinationLeft) = 0;
  virtual HRESULT put_DestinationWidth(long destinationWidth) = 0;
  virtual HRESULT get_DestinationWidth(long *destinationWidth) = 0;
  virtual HRESULT put_DestinationTop(long destinationTop) = 0;
  virtual HRESULT get_DestinationTop(long *destinationTop) = 0;
  virtual HRESULT put_DestinationHeight(long destinationHeight) = 0;
  virtual HRESULT get_DestinationHeight(long *destinationHeight) = 0;
  virtual HRESULT SetSourcePosition(long left, long top, long width,
                                    long height) = 0;
  virtual HRESULT GetSourcePosition(long *left, long *top, long *width,
                                     long *height) = 0;
  virtual HRESULT SetDefaultSourcePosition() = 0;
  virtual HRESULT SetDestinationPosition(long left, long top, long width,
                                          long height) = 0;
  virtual HRESULT GetDestinationPosition(long *left, long *top, long *width,
                                          long *height) = 0;
  virtual HRESULT SetDefaultDestinationPosition() = 0;
  virtual HRESULT GetVideoSize(long *width, long *height) = 0;
  virtual HRESULT GetVideoPaletteEntries(long startIndex, long entries,
                                          long *obtained,
                                          unsigned long *palette) = 0;
  virtual HRESULT GetCurrentImage(unsigned long *bufferSize,
                                  long *image) = 0;
  virtual HRESULT IsUsingDefaultSource() = 0;
  virtual HRESULT IsUsingDefaultDestination() = 0;
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
