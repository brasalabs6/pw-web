/* Original Element/CFWEditorLib/Triangle.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 465deb274432b904af843f8272bf0fab832055adeced12acc0e1d460c6de961c. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// Triangle.cpp: implementation of the CTriangle class.
//
//////////////////////////////////////////////////////////////////////

#include "Triangle.h"



#define new A_DEBUG_NEW



CTriangle::~CTriangle()
{

}

CTriangle::CTriangle(int index0, int index1, int index2)
{
	Index0 = index0;
	Index1 = index1;
	Index2 = index2;
}

CTriangle::CTriangle(int index[3])
{
	memcpy(Index, index, sizeof(int) * 3);
}

CTriangle::CTriangle(const CTriangle& src)
{
	*this = src;
}

CTriangle & CTriangle::operator = (const CTriangle & src)
{
	memcpy(this, &src, sizeof(CTriangle));
	return *this;
}

bool CTriangle::operator == (const CTriangle & src)
{
	return (memcmp(Index, src.Index, sizeof(int) * 3) == 0);
}

