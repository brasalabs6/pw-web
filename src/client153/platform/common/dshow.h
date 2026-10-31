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

//	The DirectShow SDK interfaces (strmif.h seam: the documented IPin/
//	IBaseFilter/... surfaces the vendored base classes derive from, plus
//	the identifier GUIDs and the media-type records).
#include "strmif.h"

//	DirectShow result codes the client checks.
#define S_OK 0
#define S_FALSE 1
#define E_FAIL 0x80004005
#define VFW_S_NO_MORE_ITEMS 0x00000001L
#define NOERROR 0

//	Media event codes the client's event pump switches on.
#define EC_COMPLETE 0x00000001
#define EC_USERABORT 0x00000002
#define EC_ERRORABORT 0x00000003

#define OATRUE (-1)
#define OAFALSE 0

//	COM base for the media-level consumer interfaces (the graph and
//	control surfaces the client drives directly; the identifier arrives
//	as the GUID reference the client passes).
struct IDShowUnknown {
  virtual ~IDShowUnknown() = default;
  virtual HRESULT QueryInterface(const GUID &riid, void **ppvObject) = 0;
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

//	The pin/filter surfaces (IPin/IEnumPins/IBaseFilter with their full
//	documented method sets) come from the strmif.h seam above; the
//	vendored base classes derive from them.

struct FILTER_INFO {
  wchar_t achName[128];
  IFilterGraph *pGraph;
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

//	The graph builder (the documented IGraphBuilder surface: the
//	RenderFile auto-build plus the manual-build calls InitSafe drives).
struct IGraphBuilder : IDShowUnknown {
  virtual HRESULT RenderFile(const wchar_t *lpwstrFile,
                             const void *pUnkReserved) = 0;
  virtual HRESULT AddFilter(void *pFilter, const wchar_t *pName) = 0;
  virtual HRESULT RemoveFilter(void *pFilter) = 0;
  virtual HRESULT Connect(IPin *ppinOut, IPin *ppinIn) = 0;
  virtual HRESULT Disconnect(IPin *pin) = 0;
  virtual HRESULT Render(IPin *ppinOut) = 0;
  virtual HRESULT FindFilterByName(const wchar_t *pName,
                                   IBaseFilter **ppFilter) = 0;
  virtual HRESULT AddSourceFilter(const wchar_t *lpwstrFileName,
                                  const wchar_t *lpwstrFilterName,
                                  IBaseFilter **ppFilter) = 0;
};

struct IFilterGraph : IDShowUnknown {
  virtual HRESULT AddFilter(void *pFilter, const wchar_t *pName) = 0;
};

//	The COM task allocator the DirectShow contracts hand out buffers
//	with (the documented CoTaskMemAlloc pair; plain heap storage on
//	this target).
inline void *CoTaskMemAlloc(std::size_t size) {
  return std::malloc(size);
}
inline void CoTaskMemFree(void *block) {
  std::free(block);
}

#endif // PW153_PLATFORM_DSHOW_H
