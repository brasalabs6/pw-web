/*
 * FILE: strmif.h (platform seam)
 *
 * DESCRIPTION: The DirectShow SDK interface subset (the SDK's IDL-generated
 * strmif.h) the client's video-clip playback compiles against. The interfaces
 * carry the documented COM signatures for the methods the vendored DirectShow
 * base classes (Angelica2/Output/3rdsdk/include) override and the media cycle
 * calls; the browser video adapter (HTML5 media) implements them in the media
 * cycle. The identifier values are the documented DirectShow GUIDs.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_STRMIF_H
#define PW153_PLATFORM_STRMIF_H

#include "Windows.h"
#include "wtypes.h"

//	SAL annotations the SDK headers annotate with (no-ops here: the
//	contracts they name are the callers' on this target).
#ifndef __out
#define __out
#endif
#ifndef __in
#define __in
#endif
#ifndef __inout
#define __inout
#endif
#ifndef __in_opt
#define __in_opt
#endif
#ifndef __out_opt
#define __out_opt
#endif
#ifndef __inout_opt
#define __inout_opt
#endif
#ifndef __deref_out
#define __deref_out
#endif
#ifndef __deref_out_opt
#define __deref_out_opt
#endif
#ifndef __in_bcount
#define __in_bcount(x)
#endif
#ifndef __in_ecount
#define __in_ecount(x)
#endif
#ifndef __out_ecount
#define __out_ecount(x)
#endif
#ifndef __deref_in
#define __deref_in
#endif
#ifndef __deref_inout
#define __deref_inout
#endif
#ifndef __deref_inout_opt
#define __deref_inout_opt
#endif
#ifndef __inout_ecount
#define __inout_ecount(x)
#endif
#ifndef __deref_out_ecount
#define __deref_out_ecount(x)
#endif
#ifndef __out_ecount_part
#define __out_ecount_part(x, y)
#endif
#ifndef __deref_out_ecount_part
#define __deref_out_ecount_part(x, y)
#endif
#ifndef __field_ecount_opt
#define __field_ecount_opt(x)
#endif
#ifndef __field_ecount
#define __field_ecount(x)
#endif
#ifndef __in_bcount_opt
#define __in_bcount_opt(x)
#endif
#ifndef __out_bcount_opt
#define __out_bcount_opt(x)
#endif
#ifndef __deref_inout_opt
#define __deref_inout_opt
#endif
//	The remaining annotation forms the vendored DirectShow SDK headers
//	annotate with (the byte-count/ecount/range set; this seam owns the
//	whole SAL surface because the Windows.h umbrella is included before
//	the C++ standard headers and GCC 16's libstdc++ uses the __in/__out
//	identifiers as structured-binding names).
#ifndef __in_bcount
#define __in_bcount(x)
#endif
#ifndef __out_bcount
#define __out_bcount(x)
#endif
#ifndef __out_bcount_part
#define __out_bcount_part(x, y)
#endif
#ifndef __in_ecount_opt
#define __in_ecount_opt(x)
#endif
#ifndef __out_ecount_opt
#define __out_ecount_opt(x)
#endif
#ifndef __inout_ecount_full
#define __inout_ecount_full(x)
#endif
#ifndef __inout_ecount_part
#define __inout_ecount_part(x, y)
#endif
#ifndef __range
#define __range(min, max)
#endif

//	The MSVC novtable optimization marker (the base-class aggregate's
//	AM_NOVTABLE; a plain marker on this target).
#ifndef AM_NOVTABLE
#define AM_NOVTABLE
#endif

//	Filter-graph time format: the documented 100-nanosecond units; media
//	position uses the documented double-precision seconds.
typedef std::int64_t REFERENCE_TIME;
typedef double REFTIME;

//	Event/semaphore handles the clock advise calls carry (the documented
//	HANDLE spellings).
typedef HANDLE HEVENT;
typedef HANDLE HSEMAPHORE;

//	Pin direction (the documented enum the pin enumeration walks).
enum PIN_DIRECTION { PINDIR_INPUT = 0, PINDIR_OUTPUT = 1 };

//	Filter state (the documented IMediaFilter state machine).
enum FILTER_STATE {
  State_Stopped = 0,
  State_Paused = 1,
  State_Running = 2
};

//	Media type (the documented AM_MEDIA_TYPE layout; the vendored base
//	classes derive CMediaType from the struct tag _AMMediaType).
typedef struct _AMMediaType {
  GUID majortype;
  GUID subtype;
  BOOL bFixedSizeSamples;
  BOOL bTemporalCompression;
  ULONG lSampleSize;
  GUID formattype;
  IUnknown *pUnk;
  ULONG cbFormat;
  BYTE *pbFormat;
} AM_MEDIA_TYPE;

struct IBaseFilter;
struct IFilterGraph;
struct IPin;
struct IEnumPins;
struct IEnumMediaTypes;
struct IReferenceClock;
struct FILTER_INFO;
struct IFilterMapper;
struct IFilterMapper2;

//	Pin info (the documented IBaseFilter::QueryFilterInfo record).
typedef struct _PinInfo {
  IBaseFilter *pFilter;
  PIN_DIRECTION dir;
  WCHAR achName[128];
} PIN_INFO;

//	Allocator requirements (the documented IMemAllocator negotiation).
typedef struct _AllocatorProperties {
  LONG cBuffers;
  LONG cbBuffer;
  LONG cbAlign;
  LONG cbPrefix;
} ALLOCATOR_PROPERTIES;

//	Quality message (the documented IQualityControl::Notify record).
typedef enum tagQualityMessageType {
  Famine = 0,
  Flood = 1
} QualityMessageType;
typedef struct tagQuality {
  QualityMessageType Type;
  LONG Proportion;
  REFERENCE_TIME Late;
  REFERENCE_TIME TimeStamp;
} Quality;

//	Automation types the control interfaces pass (the documented forms;
//	VARIANT comes complete from wtypes.h).
typedef long DISPID;
typedef DWORD LCID;

typedef struct tagDISPPARAMS {
  VARIANT *rgvarg;
  DISPID *rgdispid;
  UINT cArgs;
  UINT cNamedArgs;
} DISPPARAMS;

struct ITypeInfo;

//	Filter-registration records (the documented axextend forms; the
//	vendored combase.h composes its AMOVIESETUP records from them).
typedef struct _RegPinTypes {
  const CLSID *clsMajorType;
  const CLSID *clsMinorType;
} REGPINTYPES;

typedef struct _RegFilterPins {
  LPWSTR strName;
  BOOL bRendered;
  BOOL bOutput;
  BOOL bZero;
  BOOL bMany;
  const CLSID *clsConnectsToFilter;
  const WCHAR *strConnectsToPin;
  UINT nMediaTypes;
  const REGPINTYPES *lpMediaType;
} REGFILTERPINS;

//	COM base for the DirectShow interfaces (the canonical IUnknown from
//	the platform seam).
struct IPin : IUnknown {
  virtual HRESULT Connect(IPin *pReceivePin,
                          const AM_MEDIA_TYPE *pmt) = 0;
  virtual HRESULT ReceiveConnection(IPin *pConnector,
                                    const AM_MEDIA_TYPE *pmt) = 0;
  virtual HRESULT Disconnect() = 0;
  virtual HRESULT ConnectedTo(IPin **pPin) = 0;
  virtual HRESULT QueryDirection(PIN_DIRECTION *pPinDir) = 0;
  virtual HRESULT QueryId(LPWSTR *pId) = 0;
  virtual HRESULT QueryAccept(const AM_MEDIA_TYPE *pmt) = 0;
  virtual HRESULT EnumMediaTypes(IEnumMediaTypes **ppEnum) = 0;
  //	The documented form carries the internal-connection array (the
  //	vendored amfilter.h's CBasePin implements the IPin** slots form).
  virtual HRESULT QueryInternalConnections(IPin **apPin, ULONG *nPin) = 0;
  virtual HRESULT EndOfStream() = 0;
  virtual HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop,
                             double dRate) = 0;
};

struct IEnumMediaTypes : IUnknown {
  virtual HRESULT Next(ULONG cMediaTypes, AM_MEDIA_TYPE **ppMediaTypes,
                       ULONG *pcFetched) = 0;
  virtual HRESULT Skip(ULONG cMediaTypes) = 0;
  virtual HRESULT Reset() = 0;
  virtual HRESULT Clone(IEnumMediaTypes **ppEnum) = 0;
};

struct IEnumPins : IUnknown {
  virtual HRESULT Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched) = 0;
  virtual HRESULT Skip(ULONG cPins) = 0;
  virtual HRESULT Reset() = 0;
  virtual HRESULT Clone(IEnumPins **ppEnum) = 0;
};

//	Persist surface (the documented IPersist pair; IMediaFilter and the
//	filter derive from it).
struct IPersist : IUnknown {
  virtual HRESULT GetClassID(CLSID *pClsID) = 0;
};

//	Media filter (the documented IMediaFilter state machine; the base
//	filter classes derive from it).
struct IMediaFilter : IPersist {
  virtual HRESULT GetState(DWORD dwMSecs, FILTER_STATE *State) = 0;
  virtual HRESULT SetSyncSource(IReferenceClock *pClock) = 0;
  virtual HRESULT GetSyncSource(IReferenceClock **pClock) = 0;
  virtual HRESULT Stop() = 0;
  virtual HRESULT Pause() = 0;
  virtual HRESULT Run(REFERENCE_TIME tStart) = 0;
};

//	The filter surface (the documented IBaseFilter pin/vendor surface
//	over the media-filter state machine).
struct IBaseFilter : IMediaFilter {
  virtual HRESULT EnumPins(IEnumPins **ppEnum) = 0;
  virtual HRESULT FindPin(LPCWSTR pId, IPin **ppPin) = 0;
  virtual HRESULT QueryFilterInfo(FILTER_INFO *pInfo) = 0;
  virtual HRESULT JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName) = 0;
  virtual HRESULT QueryVendorInfo(LPWSTR *pVendorInfo) = 0;
};

//	Filter registration (the documented IAMovieSetup pair).
struct IAMovieSetup : IUnknown {
  virtual HRESULT Register() = 0;
  virtual HRESULT Unregister() = 0;
};

//	Reference clock (the documented IReferenceClock surface).
struct IReferenceClock : IUnknown {
  virtual HRESULT GetTime(REFERENCE_TIME *pTime) = 0;
  virtual HRESULT AdviseTime(REFERENCE_TIME baseTime, REFERENCE_TIME streamTime,
                             HEVENT hEvent, DWORD_PTR *pdwAdviseCookie) = 0;
  virtual HRESULT AdvisePeriodic(REFERENCE_TIME startTime,
                                 REFERENCE_TIME period, HSEMAPHORE hSemaphore,
                                 DWORD_PTR *pdwAdviseCookie) = 0;
  virtual HRESULT Unadvise(DWORD_PTR dwAdviseCookie) = 0;
};

//	Clock resolution control (the documented IReferenceClockTimerControl;
//	the base clock class overrides it).
struct IReferenceClockTimerControl : IUnknown {
  virtual HRESULT SetDefaultTimerResolution(REFERENCE_TIME timerResolution) = 0;
  virtual HRESULT GetDefaultTimerResolution(
      REFERENCE_TIME *pTimerResolution) = 0;
};

//	Quality control (the documented Notify/SetSink pair).
struct IQualityControl : IUnknown {
  virtual HRESULT Notify(IBaseFilter *pSelf, Quality q) = 0;
  virtual HRESULT SetSink(IQualityControl *piqc) = 0;
};

//	Media sample (the documented transport record; the base classes pass
//	it between pins).
struct IMediaSample : IUnknown {
  virtual HRESULT GetPointer(BYTE **ppBuffer) = 0;
  virtual LONG GetSize() = 0;
  virtual HRESULT GetTime(REFERENCE_TIME *pTimeStart,
                          REFERENCE_TIME *pTimeEnd) = 0;
  virtual HRESULT SetTime(REFERENCE_TIME *pTimeStart,
                          REFERENCE_TIME *pTimeEnd) = 0;
  virtual HRESULT IsSyncPoint() = 0;
  virtual HRESULT SetSyncPoint(BOOL bIsSyncPoint) = 0;
  virtual HRESULT IsPreroll() = 0;
  virtual HRESULT SetPreroll(BOOL bIsPreroll) = 0;
  virtual LONG GetActualDataLength() = 0;
  virtual HRESULT SetActualDataLength(LONG len) = 0;
  virtual HRESULT GetMediaType(AM_MEDIA_TYPE **ppMediaType) = 0;
  virtual HRESULT SetMediaType(AM_MEDIA_TYPE *pMediaType) = 0;
  virtual HRESULT IsDiscontinuity() = 0;
  virtual HRESULT SetDiscontinuity(BOOL bDiscontinuity) = 0;
  virtual HRESULT GetMediaTime(REFERENCE_TIME *pTimeStart,
                               REFERENCE_TIME *pTimeEnd) = 0;
  virtual HRESULT SetMediaTime(REFERENCE_TIME *pTimeStart,
                               REFERENCE_TIME *pTimeEnd) = 0;
};

struct IMediaSample2 : IMediaSample {
  virtual HRESULT GetProperties(DWORD cbProperties, BYTE *pbProperties) = 0;
  virtual HRESULT SetProperties(DWORD cbProperties,
                                const BYTE *pbProperties) = 0;
};

//	Sample properties (the documented IMediaSample2 property record the
//	input pins carry per receive).
typedef struct tagAM_SAMPLE2_PROPERTIES {
  DWORD cbData;
  DWORD dwTypeSpecificFlags;
  DWORD dwSampleFlags;
  LONG lActual;
  REFERENCE_TIME tStart;
  REFERENCE_TIME tStop;
  DWORD dwStreamId;
  AM_MEDIA_TYPE *pMediaType;
  BYTE *pbBuffer;
  LONG cbBuffer;
} AM_SAMPLE2_PROPERTIES;

//	Graph reconfiguration (the documented IGraphConfig surface the
//	dynamic output pin records).
struct IGraphConfig : IUnknown {
  virtual HRESULT Reconnect(IPin *ppinOut, IPin *ppinIn,
                            const AM_MEDIA_TYPE *pmt, IBaseFilter *pFilter,
                            void *pSyncBlock, DWORD dwFlags) = 0;
  virtual HRESULT Reconfigure(void *pfnReconfigure, void *pContext,
                              DWORD dwFlags, HRESULT *phr) = 0;
};

//	Allocator (the documented IMemAllocator negotiation surface).
struct IMemAllocator : IUnknown {
  virtual HRESULT SetProperties(ALLOCATOR_PROPERTIES *pRequest,
                                ALLOCATOR_PROPERTIES *pActual) = 0;
  virtual HRESULT GetProperties(ALLOCATOR_PROPERTIES *pProps) = 0;
  virtual HRESULT Commit() = 0;
  virtual HRESULT Decommit() = 0;
  virtual HRESULT GetBuffer(IMediaSample **ppBuffer,
                            REFERENCE_TIME *pStartTime,
                            REFERENCE_TIME *pEndTime, DWORD dwFlags) = 0;
  virtual HRESULT ReleaseBuffer(IMediaSample *pBuffer) = 0;
};

//	Allocator release callback (the documented notification surface the
//	callback-capable allocator exposes).
struct IMemAllocatorNotifyCallbackTemp : IUnknown {
  virtual HRESULT NotifyRelease(IMediaSample *pSample, BOOL bDenied) = 0;
  virtual HRESULT NotifyAllocate() = 0;
};

//	Callback-capable allocator (the documented IMemAllocatorCallbackTemp
//	extension the base allocator exposes).
struct IMemAllocatorCallbackTemp : IMemAllocator {
  virtual HRESULT SetNotify(IMemAllocatorNotifyCallbackTemp *pNotify) = 0;
  virtual HRESULT GetFreeInfo(ALLOCATOR_PROPERTIES *pProps) = 0;
};

//	Input pin transport (the documented IMemInputPin surface).
struct IMemInputPin : IUnknown {
  virtual HRESULT GetAllocator(IMemAllocator **ppAllocator) = 0;
  virtual HRESULT NotifyAllocator(IMemAllocator *pAllocator,
                                  BOOL bReadOnly) = 0;
  virtual HRESULT GetAllocatorRequirements(
      ALLOCATOR_PROPERTIES *pProps) = 0;
  virtual HRESULT Receive(IMediaSample *pSample) = 0;
  virtual HRESULT ReceiveMultiple(IMediaSample **pSamples, long nSamples,
                                  long *nSamplesProcessed) = 0;
  virtual HRESULT ReceiveCanBlock() = 0;
};

//	Async reader transport (the documented IAsyncReader surface; the
//	encrypted-file source pin exposes it).
struct IAsyncReader : IUnknown {
  virtual HRESULT RequestAllocator(IMemAllocator *pPreferred,
                                   ALLOCATOR_PROPERTIES *pProps,
                                   IMemAllocator **ppActual) = 0;
  virtual HRESULT Request(IMediaSample *pSample, DWORD_PTR dwUser) = 0;
  virtual HRESULT WaitForNext(DWORD dwTimeout, IMediaSample **ppSample,
                              DWORD_PTR *pdwUser) = 0;
  virtual HRESULT SyncReadAligned(IMediaSample *pSample) = 0;
  virtual HRESULT SyncRead(LONGLONG llPosition, LONG lLength,
                           BYTE *pBuffer) = 0;
  virtual HRESULT Length(LONGLONG *pTotal, LONGLONG *pAvailable) = 0;
  virtual HRESULT BeginFlush() = 0;
  virtual HRESULT EndFlush() = 0;
};

//	File source (the documented IFileSourceFilter pair; the encrypted
//	reader filter implements it).
struct IFileSourceFilter : IUnknown {
  virtual HRESULT Load(LPCOLESTR pszFileName,
                       const AM_MEDIA_TYPE *pmt) = 0;
  virtual HRESULT GetCurFile(LPOLESTR *ppszFileName,
                             AM_MEDIA_TYPE *pmt) = 0;
};

//	Pin flow control (the documented IPinFlowControl block).
struct IPinFlowControl : IUnknown {
  virtual HRESULT Block(DWORD dwBlockFlags, void *ppEvent) = 0;
};

//	Event sink (the documented IMediaEventSink::Notify the filter graph
//	uses to post events).
struct IMediaEventSink : IUnknown {
  virtual HRESULT Notify(long EventCode, LONG_PTR EventParam1,
                         LONG_PTR EventParam2) = 0;
};

//	Media seeking (the documented IMediaSeeking surface; the pass-through
//	position object implements it).
struct IMediaSeeking : IUnknown {
  virtual HRESULT GetCapabilities(DWORD *pCapabilities) = 0;
  virtual HRESULT CheckCapabilities(DWORD *pCapabilities) = 0;
  virtual HRESULT IsFormatSupported(const GUID *pFormat) = 0;
  virtual HRESULT QueryPreferredFormat(GUID *pFormat) = 0;
  virtual HRESULT GetTimeFormat(GUID *pFormat) = 0;
  virtual HRESULT IsUsingTimeFormat(const GUID *pFormat) = 0;
  virtual HRESULT SetTimeFormat(const GUID *pFormat) = 0;
  virtual HRESULT GetDuration(LONGLONG *pDuration) = 0;
  virtual HRESULT GetStopPosition(LONGLONG *pStop) = 0;
  virtual HRESULT GetCurrentPosition(LONGLONG *pCurrent) = 0;
  virtual HRESULT ConvertTimeFormat(LONGLONG *pTarget,
                                    const GUID *pTargetFormat, LONGLONG Source,
                                    const GUID *pSourceFormat) = 0;
  virtual HRESULT SetPositions(LONGLONG *pCurrent, DWORD dwCurrentFlags,
                               LONGLONG *pStop, DWORD dwStopFlags) = 0;
  virtual HRESULT GetPositions(LONGLONG *pCurrent, LONGLONG *pStop) = 0;
  virtual HRESULT GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest) = 0;
  virtual HRESULT SetRate(double dRate) = 0;
  virtual HRESULT GetRate(double *pdRate) = 0;
  virtual HRESULT GetPreroll(LONGLONG *pllPreroll) = 0;
};

//	The OLE structured-storage stream (the documented IStream of
//	objidl.h; the vendored pstream.h persists over it and the OLE seam has
//	no objidl.h, so the interface lives beside its IPersistStream consumer
//	here — the media adapter owns the runtime).
typedef struct tagSTATSTG {
  LPOLESTR pwcsName;
  DWORD type;
  ULARGE_INTEGER cbSize;
  FILETIME mtime;
  FILETIME ctime;
  FILETIME atime;
  DWORD grfMode;
  DWORD grfLocksSupported;
  CLSID clsid;
  DWORD grfStateBits;
  DWORD reserved;
} STATSTG;

struct IStream : IUnknown {
  virtual HRESULT Read(void *pv, ULONG cb, ULONG *pcbRead) = 0;
  virtual HRESULT Write(const void *pv, ULONG cb, ULONG *pcbWritten) = 0;
  virtual HRESULT Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
                       ULARGE_INTEGER *plibNewPosition) = 0;
  virtual HRESULT SetSize(ULARGE_INTEGER libNewSize) = 0;
  virtual HRESULT CopyTo(IStream *pstm, ULARGE_INTEGER cb,
                         ULARGE_INTEGER *pcbRead,
                         ULARGE_INTEGER *pcbWritten) = 0;
  virtual HRESULT Commit(DWORD grfCommitFlags) = 0;
  virtual HRESULT Revert() = 0;
  virtual HRESULT LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                             DWORD dwLockType) = 0;
  virtual HRESULT UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                               DWORD dwLockType) = 0;
  virtual HRESULT Stat(STATSTG *pstatstg, DWORD grfStatFlag) = 0;
  virtual HRESULT Clone(IStream **ppstm) = 0;
};
typedef IStream *LPSTREAM;

//	Persist stream (the documented IPersistStream extension; the
//	documented signatures carry the OLE stream above and the byte-count
//	record, which the vendored pstream.h overrides).
struct IPersistStream : IPersist {
  virtual HRESULT IsDirty() = 0;
  virtual HRESULT Load(IStream *pStm) = 0;
  virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) = 0;
  virtual HRESULT GetSizeMax(ULARGE_INTEGER *pcbSize) = 0;
};

//	Stream control (the documented IAMStreamControl surface the pins use;
//	the info record and flag values are the documented strmif forms the
//	vendored strmctl.h declares against).
typedef struct _AM_STREAM_INFO {
  REFERENCE_TIME tStart;
  REFERENCE_TIME tStop;
  DWORD dwStartCookie;
  DWORD dwStopCookie;
  DWORD dwFlags;
} AM_STREAM_INFO;

#define AM_STREAM_INFO_START_DEFINED 0x00000001
#define AM_STREAM_INFO_STOP_DEFINED 0x00000002
#define AM_STREAM_INFO_DISCARDING 0x00000004
#define AM_STREAM_INFO_STOP_SEND_EXTRA 0x00000010

struct IAMStreamControl : IUnknown {
  virtual HRESULT StartAt(const REFERENCE_TIME *ptStart = NULL,
                          DWORD dwStartCookie = 0) = 0;
  virtual HRESULT StopAt(const REFERENCE_TIME *ptStop = NULL,
                         BOOL bSendExtra = FALSE,
                         DWORD dwStopCookie = 0) = 0;
  virtual HRESULT GetInfo(AM_STREAM_INFO *pInfo) = 0;
};

//	Clock adjustment (the documented IAMClockAdjust pair).
struct IAMClockAdjust : IUnknown {
  virtual HRESULT SetClockDelta(REFERENCE_TIME rtDelta) = 0;
};

//	Seeking pass-through (the documented ISeekingPassThru init).
struct ISeekingPassThru : IUnknown {
  virtual HRESULT Init(BOOL bSupportRendering, IPin *pPin) = 0;
};

//	Property-page site (the documented IPropertyPageSite of ocidl.h; the
//	vendored cprop.h carries the site pointer the page notifies).
struct IPropertyPageSite : IUnknown {
  virtual HRESULT OnStatusChange(DWORD dwFlags) = 0;
  virtual HRESULT GetLocaleID(LCID *pLocaleID) = 0;
  virtual HRESULT GetPageContainer(IUnknown **ppUnk) = 0;
  virtual HRESULT GetObjects(IUnknown **ppUnk) = 0;
  virtual HRESULT GetTranslation(LONG *plTranslateIndex) = 0;
};
typedef IPropertyPageSite *LPPROPERTYPAGESITE;

//	Property-page info (the documented PROPPAGEINFO record of ocidl.h;
//	GetPageInfo fills it for the frame).
typedef struct tagPROPPAGEINFO {
  ULONG cb;
  LPOLESTR pszTitle;
  SIZE size;
  LPOLESTR pszDocString;
  LPOLESTR pszHelpFile;
  DWORD dwHelpContext;
} PROPPAGEINFO;
typedef PROPPAGEINFO *LPPROPPAGEINFO;

//	Property page (the documented IPropertyPage surface the base property
//	page implements).
struct IPropertyPage : IUnknown {
  virtual HRESULT SetPageSite(IPropertyPageSite *pPageSite) = 0;
  virtual HRESULT Activate(HWND hwndParent, LPCRECT pRect, BOOL bModal) = 0;
  virtual HRESULT Deactivate() = 0;
  virtual HRESULT GetPageInfo(PROPPAGEINFO *pPageInfo) = 0;
  virtual HRESULT SetObjects(ULONG cObjects, IUnknown **ppUnk) = 0;
  virtual HRESULT Show(UINT nCmdShow) = 0;
  virtual HRESULT Move(LPCRECT pRect) = 0;
  virtual HRESULT IsPageDirty() = 0;
  virtual HRESULT Apply() = 0;
  virtual HRESULT Help(LPCOLESTR pszHelpDir) = 0;
  virtual HRESULT TranslateAccelerator(LPMSG pMsg) = 0;
};

//	Running object table (the documented ROT surfaces; the debug-only
//	graph registration path names them).
struct IMoniker : IUnknown {};
struct IRunningObjectTable : IUnknown {
  virtual HRESULT Register(DWORD grfFlags, IUnknown *pUnkObject,
                           IMoniker *pmkObjectName,
                           DWORD *pdwRegister) = 0;
};

//	The ROT lookup entry points (the documented ole32 signatures; the
//	browser exposes no running object table, so activation fails
//	explicitly and the callers' FAILED guards take the no-ROT path).
inline HRESULT GetRunningObjectTable(DWORD reserved,
                                     IRunningObjectTable **pprot) {
  (void)reserved;
  if (pprot)
    *pprot = NULL;
  return E_FAIL;
}
inline HRESULT CreateItemMoniker(const wchar_t *lpszDelim,
                                 const wchar_t *lpszItem, IMoniker **ppmk) {
  (void)lpszDelim;
  (void)lpszItem;
  if (ppmk)
    *ppmk = NULL;
  return E_FAIL;
}

//	The DirectShow class/interface identifiers (the documented GUID
//	values; inline variables so every includer shares one definition).
inline const GUID CLSID_FilterGraph = {
    0xe436ebb3, 0x524f, 0x11ce, {0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID CLSID_MPEG1Splitter = {
    0x336475d0, 0x942a, 0x11ce, {0xa8, 0x70, 0x00, 0xaa, 0x00, 0x2f, 0xea, 0xb5}};
inline const GUID CLSID_CMpegVideoCodec = {
    0xfeb50740, 0x7bef, 0x11ce, {0x9b, 0xd9, 0x00, 0x00, 0xe2, 0x02, 0x59, 0x9c}};
inline const GUID CLSID_CMpegAudioCodec = {
    0x4a2286e0, 0x7bef, 0x11ce, {0x9b, 0xd9, 0x00, 0x00, 0xe2, 0x02, 0x59, 0x9c}};
inline const GUID CLSID_DSoundRender = {
    0x79376820, 0x07d0, 0x11cf, {0xa2, 0x4d, 0x00, 0x20, 0xaf, 0xd7, 0x97, 0x67}};

inline const GUID IID_IFilterGraph = {
    0x56a8689f, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IBaseFilter = {
    0x56a86895, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IGraphBuilder = {
    0x56a868a9, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IMediaControl = {
    0x56a868b1, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IMediaEvent = {
    0x56a868b6, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IMediaPosition = {
    0x56a868b2, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IBasicAudio = {
    0x56a868b3, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IFileSourceFilter = {
    0x56a868a6, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IAsyncReader = {
    0x56a868aa, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IPin = {
    0x56a86891, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IEnumPins = {
    0x56a86892, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IEnumMediaTypes = {
    0x89c31040, 0x846b, 0x11ce, {0x97, 0xd3, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a}};
inline const GUID IID_IMemAllocator = {
    0x56a8689c, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IMemInputPin = {
    0x56a8689d, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IMediaSample = {
    0x56a8689a, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IReferenceClock = {
    0x56a86897, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IQualityControl = {
    0x56a868a5, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IMediaSeeking = {
    0x36b73880, 0xc2c8, 0x11cf, {0x8b, 0x46, 0x00, 0x80, 0x5f, 0x6c, 0xef, 0x60}};
inline const GUID IID_IMediaEventSink = {
    0x56a868a2, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IAMovieSetup = {
    0xa3d8cec0, 0x7e5a, 0x11cf, {0xbb, 0xc5, 0x00, 0x80, 0x5f, 0x6c, 0xef, 0x20}};
inline const GUID IID_IVideoWindow = {
    0x56a868b4, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID IID_IBasicVideo = {
    0x56a868b5, 0x0ad4, 0x11ce, {0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};

//	Media types the source filter builds (the documented stream pair;
//	the null subtype is the documented all-zero GUID).
inline const GUID MEDIATYPE_Stream = {
    0xe436eb83, 0x524f, 0x11ce, {0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70}};
inline const GUID MEDIASUBTYPE_NULL = {};

//	The quality-property interface the vendored DirectShow base classes
//	(renbase.h's CBaseVideoRenderer) derive from. The full DX SDK declared
//	it in amvideo.h; the vendored 3rdsdk ships a partial set, so the
//	declaration surface lives here (IQualityControl is declared above;
//	the browser media adapter owns the runtime).
struct IQualProp : IUnknown {};

#endif // PW153_PLATFORM_STRMIF_H
