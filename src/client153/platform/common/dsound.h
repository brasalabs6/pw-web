/*
 * FILE: dsound.h (platform seam)
 *
 * DESCRIPTION: The DirectSound boundary the client's media library compiles
 * against. The interfaces carry the documented Win32 signatures for the
 * methods the client calls; the WebAudio adapter implements them in the
 * audio cycle. Sources calling methods not declared here keep failing on
 * those methods - the honest compile-first signal.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_DSOUND_H
#define PW153_PLATFORM_DSOUND_H

#include "Windows.h"

//	The COM result macros the client checks with.
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

//	Result codes: the documented DirectSound values the client checks.
#define DS_OK 0
#define DSERR_GENERIC 0x88780000
#define DSERR_ALLOCATED 0x88780001
#define DSERR_INVALIDPARAM 0x88780087
#define DSERR_BUFFERLOST 0x88780096
#define DSERR_UNSUPPORTED 0x887800BA
#define DSERR_NODRIVER 0x88780078

//	Playback, lock and status flags.
#define DSBPLAY_LOOPING 0x00000001
#define DSBLOCK_ENTIREBUFFER 0x00000002
#define DSBLOCK_FROMWRITECURSOR 0x00000001
#define DSBSTATUS_PLAYING 0x00000001
#define DSBSTATUS_BUFFERLOST 0x00000002
#define DSBSTATUS_LOOPING 0x00000004
#define DSBSTATUS_TERMINATED 0x00000008

//	Volume range: the documented DirectSound decibel bounds.
#define DSBVOLUME_MIN (-10000)
#define DSBVOLUME_MAX 0
#define DSBFREQUENCY_MIN 100
#define DSBFREQUENCY_MAX 100000
#define DSBFREQUENCY_ORIGINAL 0

//	Cooperative levels.
#define DSSCL_NORMAL 0x00000001
#define DSSCL_PRIORITY 0x00000002
#define DSSCL_EXCLUSIVE 0x00000003
#define DSSCL_WRITEPRIMARY 0x00000004

#pragma pack(push, 1)
struct WAVEFORMAT {
  WORD wFormatTag;
  WORD nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD nBlockAlign;
};

struct PCMWAVEFORMAT {
  WAVEFORMAT wf;
  WORD wBitsPerSample;
};

struct WAVEFORMATEX {
  WORD wFormatTag;
  WORD nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD nBlockAlign;
  WORD wBitsPerSample;
  WORD cbSize;
};
#pragma pack(pop)

#define WAVE_FORMAT_PCM 0x0001

struct DSBUFFERDESC {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwBufferBytes;
  DWORD dwReserved;
  WAVEFORMATEX *lpwfxFormat;
};

struct DSCAPS {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwMinSecondarySampleRate;
  DWORD dwMaxSecondarySampleRate;
  DWORD dwPrimaryBuffers;
};

struct DSBCAPS {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwBufferBytes;
  DWORD dwUnlockTransferRate;
  DWORD dwPlayCpuOverhead;
};

//	COM base for the DirectSound interfaces.
struct IDSoundUnknown {
  virtual ~IDSoundUnknown() = default;
  virtual HRESULT QueryInterface(const void *riid, void **ppvObject) = 0;
  virtual ULONG AddRef() = 0;
  virtual ULONG Release() = 0;
};

struct IDirectSoundBuffer : IDSoundUnknown {
  virtual HRESULT GetCaps(DSBCAPS *pBufferCaps) = 0;
  virtual HRESULT GetCurrentPosition(DWORD *pdwCurrentPlayCursor,
                                     DWORD *pdwCurrentWriteCursor) = 0;
  virtual HRESULT Lock(DWORD dwOffset, DWORD dwBytes, void **ppvAudioPtr1,
                       DWORD *pdwAudioBytes1, void **ppvAudioPtr2,
                       DWORD *pdwAudioBytes2, DWORD dwFlags) = 0;
  virtual HRESULT Play(DWORD dwReserved1, DWORD dwPriority,
                       DWORD dwFlags) = 0;
  virtual HRESULT SetCurrentPosition(DWORD dwNewPosition) = 0;
  virtual HRESULT SetFrequency(DWORD dwFrequency) = 0;
  virtual HRESULT SetVolume(LONG lVolume) = 0;
  virtual HRESULT Stop() = 0;
  virtual HRESULT GetStatus(DWORD *pdwStatus) = 0;
  virtual HRESULT SetPan(LONG lPan) = 0;
  virtual HRESULT GetFrequency(DWORD *pdwFrequency) = 0;
  virtual HRESULT GetVolume(LONG *plVolume) = 0;
  virtual HRESULT Unlock(void *pvAudioPtr1, DWORD dwAudioBytes1,
                         void *pvAudioPtr2, DWORD dwAudioBytes2) = 0;
  virtual HRESULT Restore() = 0;
};

struct IDirectSoundBuffer8 : IDirectSoundBuffer {
};

struct DSBPOSITIONNOTIFY {
  DWORD dwOffset;
  HANDLE hEventNotify;
};

struct IDirectSoundNotify : IDSoundUnknown {
  virtual HRESULT SetNotificationPositions(DWORD dwNumElements,
                                           const DSBPOSITIONNOTIFY *pcElements) = 0;
};

struct IDirectSoundNotify8 : IDirectSoundNotify {
};

//	3D positional audio: the documented modes and the buffer interface.
#define DS3DMODE_NORMAL 0x00000000
#define DS3DMODE_HEADRELATIVE 0x00000001
#define DS3DMODE_DISABLE 0x00000002
#define DS3D_IMMEDIATE 0x00000000
#define DS3D_DEFERRED 0x00000001
#define DS3D_DEFFERED DS3D_DEFERRED	//	original's spelling.

struct IDirectSound3DBuffer : IDSoundUnknown {
  virtual HRESULT SetPosition(float x, float y, float z, DWORD apply) = 0;
  virtual HRESULT SetVelocity(float x, float y, float z, DWORD apply) = 0;
  virtual HRESULT SetMode(DWORD mode, DWORD apply) = 0;
  virtual HRESULT GetPosition(float *x, float *y, float *z) = 0;
};

struct IDirectSound3DBuffer8 : IDirectSound3DBuffer {
};

struct IDirectSound3DListener : IDSoundUnknown {
  virtual HRESULT SetDistanceFactor(float factor, DWORD apply) = 0;
  virtual HRESULT SetRolloffFactor(float factor, DWORD apply) = 0;
  virtual HRESULT SetDopplerFactor(float factor, DWORD apply) = 0;
  virtual HRESULT SetOrientation(float xFront, float yFront, float zFront,
                                 float xTop, float yTop, float zTop,
                                 DWORD apply) = 0;
};

struct IDirectSound3DListener8 : IDirectSound3DListener {
};

struct IDirectSound : IDSoundUnknown {
  virtual HRESULT SetCooperativeLevel(HWND hwnd, DWORD dwLevel) = 0;
};

struct IDirectSound8 : IDirectSound {
};

//	The factory: the audio adapter implements the real device creation.
HRESULT DirectSoundCreate8(const void *pcGuidBuffer, IDirectSound8 **ppDS8,
                           void *pUnkOuter);

#endif // PW153_PLATFORM_DSOUND_H
