/*
 * FILE: dshow.h (platform seam)
 *
 * DESCRIPTION: The DirectShow boundary the client's video-clip playback
 * compiles against. The interfaces carry the documented COM signatures for
 * the methods the client calls (graph building, media control, event
 * pumping, position, audio); the browser video adapter (HTML5 media)
 * implements them in the media cycle. CoCreateInstance routes through the
 * adapter registry the media cycle installs.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_DSHOW_H
#define PW153_PLATFORM_DSHOW_H

#include "Windows.h"

//	DirectShow result codes the client checks.
#define S_OK 0
#define S_FALSE 1
#define E_FAIL 0x80004005
#define VFW_S_NO_MORE_ITEMS 0x00000001L

//	Media event codes the client's event pump switches on.
#define EC_COMPLETE 0x00000001
#define EC_USERABORT 0x00000002
#define EC_ERRORABORT 0x00000003

#define OATRUE (-1)
#define OAFALSE 0

//	Filter-graph time format: the documented 100-nanosecond units; media
//	position uses the documented double-precision seconds.
typedef std::int64_t REFERENCE_TIME;
typedef double REFTIME;

//	COM base for the DirectShow interfaces.
struct IDShowUnknown {
  virtual ~IDShowUnknown() = default;
  virtual HRESULT QueryInterface(const void *riid, void **ppvObject) = 0;
  virtual ULONG AddRef() = 0;
  virtual ULONG Release() = 0;
};

struct IMediaControl : IDShowUnknown {
  virtual HRESULT Run() = 0;
  virtual HRESULT Pause() = 0;
  virtual HRESULT Stop() = 0;
};

//	The event pump (the documented IMediaEvent surface; IMediaEventEx
//	extends it with the notify window below).
struct IMediaEvent : IDShowUnknown {
  virtual HRESULT GetEvent(LONG *lEventCode, LONG_PTR *lParam1,
                           LONG_PTR *lParam2, LONG msTimeout) = 0;
  virtual HRESULT FreeEventParams(LONG lEventCode, LONG_PTR lParam1,
                                  LONG_PTR lParam2) = 0;
};

struct IMediaEventEx : IMediaEvent {
  virtual HRESULT SetNotifyWindow(HWND hwnd, UINT message,
                                  LONG_PTR lInstanceParam) = 0;
};

//	The filter-graph pin surface the video clip walks (the documented
//	DirectShow signatures; the media cycle's adapter implements them).
enum PIN_DIRECTION { PINDIR_INPUT = 0, PINDIR_OUTPUT = 1 };

struct IPin : IDShowUnknown {
  virtual HRESULT QueryDirection(PIN_DIRECTION *pPinDir) = 0;
  virtual HRESULT QueryId(wchar_t **pId) = 0;
};

struct IEnumPins : IDShowUnknown {
  virtual HRESULT Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched) = 0;
};

struct IFilterGraph;

struct FILTER_INFO {
  wchar_t achName[128];
  IFilterGraph *pGraph;
};

struct IBaseFilter : IDShowUnknown {
  virtual HRESULT EnumPins(IEnumPins **ppEnum) = 0;
  virtual HRESULT QueryFilterInfo(FILTER_INFO *pInfo) = 0;
};

struct IMediaPosition : IDShowUnknown {
  virtual HRESULT get_Duration(REFTIME *pDuration) = 0;
  virtual HRESULT put_CurrentPosition(REFTIME llTime) = 0;
  virtual HRESULT get_CurrentPosition(REFTIME *pllTime) = 0;
};

struct IBasicAudio : IDShowUnknown {
  virtual HRESULT put_Volume(LONG lVolume) = 0;
  virtual HRESULT get_Volume(LONG *plVolume) = 0;
};

struct IGraphBuilder : IDShowUnknown {
  virtual HRESULT RenderFile(const wchar_t *lpwstrFile,
                             const void *pUnkReserved) = 0;
};

struct IFilterGraph : IDShowUnknown {
  virtual HRESULT AddFilter(void *pFilter, const wchar_t *pName) = 0;
};


#endif // PW153_PLATFORM_DSHOW_H
