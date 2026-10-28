/*
 * FILE: amvideo.h (platform seam)
 *
 * DESCRIPTION: The ActiveMovie video type definitions (the SDK's
 * amvideo.h). The vendored base classes (winutil.h's display-format
 * helpers, mtype.h's format blocks) carry the documented VIDEOINFO
 * family; the layouts are the documented ones.
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef PW153_PLATFORM_AMVIDEO_H
#define PW153_PLATFORM_AMVIDEO_H

#include "Windows.h"
#include "strmif.h"

#define iPALETTE_COLORS 256
#define iEGA_COLORS 16
#define iMASK_COLORS 3
#define iTRUECOLOR 16
#define iRED 0
#define iGREEN 1
#define iBLUE 2
#define iPALETTE 8
#define iMAXBITS 8

//	True-color info (the documented palette/bit-mask record).
typedef struct tag_TRUECOLORINFO {
  DWORD dwBitMasks[iMASK_COLORS];
  RGBQUAD bmiColors[iPALETTE_COLORS];
} TRUECOLORINFO;

//	Video info header (the documented media-type format block).
typedef struct tagVIDEOINFOHEADER {
  RECT rcSource;
  RECT rcTarget;
  DWORD dwBitRate;
  DWORD dwBitErrorRate;
  REFERENCE_TIME AvgTimePerFrame;
  BITMAPINFOHEADER bmiHeader;
} VIDEOINFOHEADER;

//	Full video info (the documented VIDEOINFOHEADER plus the palette
//	union; the base classes hold it by value).
typedef struct tagVIDEOINFO {
  RECT rcSource;
  RECT rcTarget;
  DWORD dwBitRate;
  DWORD dwBitErrorRate;
  REFERENCE_TIME AvgTimePerFrame;
  BITMAPINFOHEADER bmiHeader;
  union {
    RGBQUAD bmiColors[iPALETTE_COLORS];
    DWORD dwBitMasks[iMASK_COLORS];
    TRUECOLORINFO TrueColorInfo;
  };
} VIDEOINFO;

//	MPEG-1 video info (the documented extension).
#define MAX_SIZE_MPEG1_SEQUENCE_INFO 140
typedef struct tagMPEG1VIDEOINFO {
  VIDEOINFOHEADER hdr;
  DWORD dwStartTimeCode;
  DWORD cbSequenceHeader;
  BYTE bSequenceHeader[MAX_SIZE_MPEG1_SEQUENCE_INFO];
} MPEG1VIDEOINFO;

#endif // PW153_PLATFORM_AMVIDEO_H
