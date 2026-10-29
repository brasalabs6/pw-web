/*
 * FILE: AAssist.h
 *
 * DESCRIPTION: Assist routines
 *
 * CREATED BY: duyuxin, 2003/6/6
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 *
 * Adapted from original AAssist.h, revision and hashes in source-manifest.json.
 * The wide-character conversion entry points (a_MultiByteToWideChar,
 * a_WideCharToMultiByte) are declared here and implemented over the
 * platform's UTF-16/UTF-8 converters in AAssist.cpp (ElementClient.cpp is
 * the consumer). AWString's own conversion helpers and the custom
 * memory-statistic surface remain omitted until a port consumer needs them.
 * Templates, macros, string IDs and the random generator keep the original
 * behavior.
 */

#ifndef _AASSIST_H_
#define _AASSIST_H_

//	Wide/narrow conversions (the Win32 WideCharToMultiByte contract; a
//	negative input length means null-terminated input). Implemented in
//	AAssist.cpp over the platform's UTF-16/UTF-8 converters.
int a_WideCharToMultiByte(const wchar_t* wszWideChar, int iWideChar, char* szMultiByte, int iMultiByte);
int a_WideCharToMultiByte(int iCodePage, const wchar_t* wszWideChar, int iWideChar, char* szMultiByte, int iMultiByte);
int a_MultiByteToWideChar(const char* szMultiByte, int iMultiByte, wchar_t* wszWideChar, int iWideChar);
int a_MultiByteToWideChar(int iCodePage, const char* szMultiByte, int iMultiByte, wchar_t* wszWideChar, int iWideChar);





#include "ABaseDef.h"
#include "AString.h"
#include "AWString.h"

#include <cstdlib>

//	The string-class conversions (the Win32 conversion contract over the
//	platform's UTF-16/UTF-8 converters; the code page argument is the
//	Win32 legacy - the port always converts UTF-8 to UTF-16). The
//	AS2AC/AC2AS/... macros in the SDK headers map here.
AWString AS2WC(const char* szAnsi);
AString WC2AS(const wchar_t* szWide);
AWString AS2WC(int iCodePage, const char* szAnsi);
AString WC2AS(int iCodePage, const wchar_t* szWide);

//	Memory statistic functions: the custom memory manager is not
//	ported (a_malloc maps to the platform allocator), so the size
//	statistics report the real process heap and the custom allocator's
//	slot/alloc counters report zero (the debug display's documented
//	limitation on this target). Implemented in AAssist.cpp.
int a_GetMemPeakSize();
DWORD a_GetMemAllocCounter();
int a_GetMemCurSize();
int a_GetMemRawSize();
void a_GetSmallMemInfo(int iSlot, int* piBlkSize, int* piBlkCnt, int* piFreeCnt);
void a_GetLargeMemInfo(int* piBlkCnt, int* piAllocSize);
void a_MemGarbageCollectAuto();
void a_MemGarbageCollectAll();
void a_ExportMemLog(const char* szPath);

///////////////////////////////////////////////////////////////////////////
//
//	Define and Macro
//
///////////////////////////////////////////////////////////////////////////

#define ARAND_MAX	RAND_MAX

#ifdef UNICODE
	#define ACString		AWString
	#define AS2AC(s)		AS2WC(s)
	#define WC2AC(s)		AWString(s)
	#define AC2AS(s)		WC2AS(s)
	#define	AC2WC(s)		AWString(s)
	#define AS2AC_CP(cp, s)	AS2WC(cp, s)
	#define WC2AC_CP(cp, s)	AWString(s)
	#define AC2AS_CP(cp, s)	WC2AS(cp, s)
	#define	AC2WC_CP(cp, s)	AWString(s)
#else
	#define ACString		AString
	#define AS2AC(s)		AString(s)
	#define WC2AC(s)		WC2AS(s)
	#define AC2AS(s)		AString(s)
	#define AC2WC(s)		AS2WC(s)
	#define AS2AC_CP(cp, s)	AString(s)
	#define WC2AC_CP(cp, s)	WC2AS(cp, s)
	#define AC2AS_CP(cp, s)	AString(s)
	#define AC2WC_CP(cp, s)	AS2WC(cp, s)
#endif

#define min2(a, b) (((a) > (b)) ? (b) : (a))
#define min3(a, b, c) (min2(min2((a), (b)), (c)))
#define max2(a, b) (((a) > (b)) ? (a) : (b))
#define max3(a, b, c) (max2(max2((a), (b)), (c)))
#define min4(a, b, c, d) (min2(min2((a), (b)), min2((c), (d))))
#define max4(a, b, c, d) (max2(max2((a), (b)), max2((c), (d))))

///////////////////////////////////////////////////////////////////////////
//
//	Types and Global variables
//
///////////////////////////////////////////////////////////////////////////

//	Make ID from string
DWORD a_MakeIDFromString(const char* szStr);
DWORD a_MakeIDFromLowString(const char* szStr);

//	Convert a UTF-8 path to GB18030 (the encoding the original .pck package
//	indexes use). Pure-ASCII paths pass through unchanged. Returns the
//	converted length, or 0 on failure (caller should fall back to the
//	original). This is the boundary between the adapted UTF-8 source and
//	the original GB18030 game data.
int a_Utf8ToGb18030(const char* szUtf8, char* szGb18030, int iGb18030Size);

//	Convert GB18030 bytes to wide chars (UTF-16). Returns the number of
//	wide chars written, or 0 on failure. Used by AWScriptFile to handle
//	non-UTF-16 script files (the original client's data files use GB18030).
int a_Gb18030ToWide(const char* szGb18030, int iGbLen, wchar_t* szWide, int iWideSize);
bool a_GetStringAfter(const char* szBuffer, const char* szTag, char* szResult);

//	Set / Get current character code page
void a_SetCharCodePage(DWORD dwCodePage);
DWORD a_GetCharCodePage();

//	Conversion between narrow (UTF-8 in this port) and wide (UTF-16) strings.
//	The original converted through the Windows code page; the browser port's
//	narrow encoding is UTF-8 and the wide encoding keeps the original UTF-16
//	width, so these use the strict platform conversions. Malformed input
//	yields an empty result; the code-page argument is accepted and unused.
AWString AS2WC(const char* szAnsi);
AString WC2AS(const wchar_t* szWide);
AWString AS2WC(int iCodePage, const char* szAnsi);
AString WC2AS(int iCodePage, const wchar_t* szWide);

//	Random number generator
bool a_InitRandom();
int a_Random();
float a_Random(float fMin, float fMax);
int a_Random(int iMin, int iMax);

///////////////////////////////////////////////////////////////////////////
//
//	Declare of Global functions
//
///////////////////////////////////////////////////////////////////////////

template <class T>
inline void a_Swap(T& lhs, T& rhs)
{
	T tmp;
	tmp = lhs;
	lhs = rhs;
	rhs = tmp;
}

template <class T>
inline const T& a_Min(const T& x, const T& y)
{
	return y < x ? y : x;
}

template <class T>
inline const T& a_Max(const T& x, const T& y)
{
	return y < x ? x : y;
}

template <class T>
inline const T& a_Min(const T& x, const T& y, const T& z)
{
	return a_Min(a_Min(x, y), z);
}

template <class T>
inline const T& a_Max(const T& x, const T& y, const T& z)
{
	return a_Max(a_Max(x, y), z);
}

template <class T>
inline void a_ClampRoof(T& x, const T& max)
{
	if (x > max) x = max;
}

template <class T>
inline void a_ClampFloor(T& x, const T& min)
{
	if (x < min) x = min;
}

template <class T>
inline void a_Clamp(T& x, const T& min, const T& max)
{
	if (x < min) x = min;
	if (x > max) x = max;
}

#endif	//	_AASSIST_H_
