/*
 * FILE: AWString.h
 *
 * DESCRIPTION: char16_t String operating class
 *
 * CREATED BY: duyuxin, 2004/2/10
 *
 * HISTORY:
 *
 * Copyright (c) 2001 Archosaur Studio, All Rights Reserved.
 */

#ifndef _AWString_H_
#define _AWString_H_

#include <assert.h>
#include <cstdio>
#include <cstdint>
//	wchar.h CRT replaced by the portable wide seam.
#include "widestr.h"

//	Portable wide-string helpers replacing the wchar_t CRT (char16_t has no
//	standard wcs* functions).
static inline int WideLen(const char16_t* szStr)
{
	int iLen = 0;
	while (szStr[iLen]) iLen++;
	return iLen;
}
static inline int WideCmp(const char16_t* a, const char16_t* b)
{
	int i = 0;
	while (a[i] && a[i] == b[i]) i++;
	return (int)a[i] - (int)b[i];
}
static inline int WideNCmp(const char16_t* a, const char16_t* b, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (a[i] != b[i]) return (int)a[i] - (int)b[i];
		if (!a[i]) return 0;
	}
	return 0;
}
static inline const char16_t* WideStr(const char16_t* hay, const char16_t* needle)
{
	if (!needle[0]) return hay;
	for (int i = 0; hay[i]; i++)
	{
		int j = 0;
		while (needle[j] && hay[i + j] == needle[j]) j++;
		if (!needle[j]) return hay + i;
	}
	return nullptr;
}
static inline const char16_t* WideRChr(const char16_t* s, char16_t ch)
{
	const char16_t* last = nullptr;
	for (int i = 0; s[i]; i++)
		if (s[i] == ch) last = s + i;
	return last;
}

///////////////////////////////////////////////////////////////////////////
//
//	Define and Macro
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//	Types and Global variables
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//	Declare of Global functions
//
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//	class AWString
//
///////////////////////////////////////////////////////////////////////////

class AWString
{
public:		//	Types

	struct s_STRINGDATA
	{
		int		iRefs;		//	Reference count
		int		iDataLen;	//	Length of data (not including terminator)
		int		iMaxLen;	//	Maximum length of data (not including terminator)
		
		char16_t*	Data()	{ return (char16_t*)(this+1); }	//	char16_t* to managed data
	};

	struct s_DOUBLE
	{ 
		unsigned char	aDoubleBits[sizeof (double)];
	};

public:		//	Constructors and Destructors

	AWString() : m_pStr(m_pEmptyStr) {}
	AWString(const AWString& str);
	AWString(const char16_t* szStr);
	AWString(const char16_t* szStr, int iLen);
	AWString(char16_t ch, int iRepeat);
	~AWString();

public:		//	Attributes

public:		//	Operations

	const AWString& operator = (char16_t ch);
	const AWString& operator = (const char16_t* szStr);
	const AWString& operator = (const AWString& str);

	const AWString& operator += (char16_t ch);
	const AWString& operator += (const char16_t* szStr);
	const AWString& operator += (const AWString& str);

	friend AWString operator + (const AWString& str1, const AWString& str2) { return AWString(str1, str2); }
	friend AWString operator + (char16_t ch, const AWString& str) { return AWString(ch, str); }
	friend AWString operator + (const AWString& str, char16_t ch) { return AWString(str, ch); }
	friend AWString operator + (const char16_t* szStr, const AWString& str) { return AWString(szStr, str); }
	friend AWString operator + (const AWString& str, const char16_t* szStr) { return AWString(str, szStr); }

	int Compare(const char16_t* szStr) const;
	int CompareNoCase(const char16_t* szStr) const;

	bool operator == (char16_t ch) const { return (GetLength() == 1 && m_pStr[0] == ch); }
	bool operator == (const char16_t* szStr) const;
	bool operator == (const AWString& str) const;

	bool operator != (char16_t ch) const { return !(GetLength() == 1 && m_pStr[0] == ch); }
	bool operator != (const char16_t* szStr) const { return !((*this) == szStr); }
	bool operator != (const AWString& str) const { return !((*this) == str); }

	bool operator > (const char16_t* szStr) const { return Compare(szStr) > 0; }
	bool operator < (const char16_t* szStr) const { return Compare(szStr) < 0; }
	bool operator >= (const char16_t* szStr) const { return !((*this) < szStr); }
	bool operator <= (const char16_t* szStr) const { return !((*this) > szStr); }

	char16_t operator [] (int n) const { assert(n >= 0 && n <= GetLength()); return m_pStr[n]; }
	char16_t& operator [] (int n);

	operator const char16_t* () const { return m_pStr; }

	//	Get string length
	int	GetLength()	const { return GetData()->iDataLen; }
	//	String is empty ?
	bool IsEmpty() const { return m_pStr == m_pEmptyStr ? true : false; }
	//	Empty string
	void Empty() { FreeBuffer(GetData()); m_pStr = m_pEmptyStr; }
	//	Convert to upper case
	void MakeUpper();
	//	Convert to lower case
	void MakeLower();
	//	Format string
	AWString& Format(const char16_t* szFormat, ...);

	//	Get buffer
	char16_t* GetBuffer(int iMinSize);
	//	Release buffer gotten through GetBuffer()
	void ReleaseBuffer(int iNewSize=-1);
	//	Lock buffer
	char16_t* LockBuffer();
	//	Unlock buffer
	void UnlockBuffer();

	//	Trim left
	void TrimLeft();
	void TrimLeft(char16_t ch);
	void TrimLeft(const char16_t* szChars);
	//	Trim right
	void TrimRight();
	void TrimRight(char16_t ch);
	void TrimRight(const char16_t* szChars);

	//	Cut left sub string
	void CutLeft(int n);
	//	Cut right sub string
	void CutRight(int n);

	//	Finds a character or substring inside a larger string. 
	int Find(char16_t ch, int iStart=0) const;
	int Find(const char16_t* szSub, int iStart=0) const;
	//	Finds a character inside a larger string; starts from the end. 
	int ReverseFind(char16_t ch) const;
	//	Finds the first matching character from a set. 
	int FindOneOf(const char16_t* szCharSet) const;

	//	Get left string
	inline AWString Left(int n) const;
	//	Get right string
	inline AWString Right(int n) const;
	//	Get Mid string
	inline AWString Mid(int iFrom, int iNum=-1) const;

	//	Convert string to integer
	inline int ToInt() const;
	//	Convert string to float
	inline float ToFloat() const;
	//	Convert string to __int64
	inline std::int64_t ToInt64() const;
	//	Convert string to double
	inline double ToDouble() const;
	
protected:	//	Attributes

	char16_t*		m_pStr;			//	String buffer
	static char16_t*	m_pEmptyStr;

protected:	//	Operations

	//	These constructors are used to optimize operations such as 'operator +'
	AWString(const AWString& str1, const AWString& str2);
	AWString(char16_t ch, const AWString& str);
	AWString(const AWString& str, char16_t ch);
	AWString(const char16_t* szStr, const AWString& str);
	AWString(const AWString& str, const char16_t* szStr);

	//	Get string data
	s_STRINGDATA* GetData() const { return ((s_STRINGDATA*)m_pStr)-1; }

	//	Safed strlen()
	static int SafeStrLen(const char16_t* szStr) { return szStr ? (int)WideLen(szStr) : 0; }
	//	String copy
	static void StringCopy(char16_t* szDest, const char16_t* szSrc, int iLen);
	//	Alocate string buffer
	static char16_t* AllocBuffer(int iLen);
	//	Free string data buffer
	static void FreeBuffer(s_STRINGDATA* pStrData);
	//	Judge whether two strings are equal
	static bool StringEqual(const char16_t* s1, const char16_t* s2, int iLen);

	//	Allocate memory and copy string
	static char16_t* AllocThenCopy(const char16_t* szSrc, int iLen);
	static char16_t* AllocThenCopy(const char16_t* szSrc, char16_t ch, int iLen);
	static char16_t* AllocThenCopy(char16_t ch, const char16_t* szSrc, int iLen);
	static char16_t* AllocThenCopy(const char16_t* s1, const char16_t* s2, int iLen1, int iLen2);

	//	Get format string length
	static int GetFormatLen(const char16_t* szFormat, va_list argList);
};

///////////////////////////////////////////////////////////////////////////
//
//	Inline functions
//
///////////////////////////////////////////////////////////////////////////

//	Get left string
AWString AWString::Left(int n) const
{
	assert(n >= 0);
	int iLen = GetLength();
	return AWString(m_pStr, iLen < n ? iLen : n);
}

//	Get right string
AWString AWString::Right(int n) const
{
	assert(n >= 0);
	int iFrom = GetLength() - n;
	return Mid(iFrom < 0 ? 0 : iFrom, n);
}

//	Get Mid string
AWString AWString::Mid(int iFrom, int iNum/* -1 */) const
{
	int iLen = GetLength() - iFrom;
	if (iLen <= 0 || !iNum)
		return AWString();	//	Return empty string

	if (iNum > 0 && iLen > iNum)
		iLen = iNum;

	return AWString(m_pStr+iFrom, iLen);
}

//	Convert string to integer
int AWString::ToInt() const
{
	return IsEmpty() ? 0 : pw153::platform::wide_toi(m_pStr);
}

//	Convert string to float
float AWString::ToFloat() const
{
	if (IsEmpty())
		return 0.0f;
	
	float fValue;
	fValue = pw153::platform::wide_tof(m_pStr);
	return fValue;
}

//	Convert string to __int64
std::int64_t AWString::ToInt64() const
{
	return IsEmpty() ? 0 : pw153::platform::wide_toi64(m_pStr);
}

//	Convert string to double
double AWString::ToDouble() const
{
	if (IsEmpty())
		return 0.0;
	
   const char16_t *endptr = nullptr;
   return pw153::platform::wide_tod(m_pStr, &endptr);
}

#endif	//	_AWString_H_
