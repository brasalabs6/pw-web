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
 * The wide-character conversion surface (AWString, a_MultiByteToWideChar,
 * a_WideCharToMultiByte, AS2WC, WC2AS and the ACString/UNICODE macros) and the
 * custom memory-statistic surface are omitted until a port consumer needs them;
 * no skeleton consumer uses them. Templates, macros, string IDs and the random
 * generator keep the original behavior.
 */

#ifndef _AASSIST_H_
#define _AASSIST_H_

#include "ABaseDef.h"
#include "AString.h"
#include "AWString.h"

#include <cstdlib>

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
