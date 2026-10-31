/* Original Element/CFWEditorLib/FWFormulaElement.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 6cfb579b2e2052ecfc50b92493aa52eb97707cfcbb8962305d19a0a505940141. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FormulaElement.cpp: implementation of the FWFormulaElement class.
//
//////////////////////////////////////////////////////////////////////

#include "FWFormulaElement.h"
#include <A3DMacros.h>

#define new A_DEBUG_NEW




FWFormulaElement::FWFormulaElement()
{

}

FWFormulaElement::~FWFormulaElement()
{

}

FWFormulaElement::FWFormulaElement(const AString & raw)
{
	m_strRaw = raw;
	m_nType = DetermineType(raw);
}

FWFormulaElement::FWFormulaElement(const AString & raw, int nType)
{
	m_strRaw = raw;
	m_nType = nType;
}

FWFormulaElement& FWFormulaElement::operator = (const FWFormulaElement & src)
{
	m_strRaw = src.GetRawString();
	m_nType = src.GetType();
	return *this;
}


int FWFormulaElement::DetermineType(const AString & raw)
{
	ASSERT(raw.GetLength() > 0);
	
	if (raw.GetLength() == 1)
	{
		static const AString opSet = "+-*/";
		if (opSet.Find(raw[0]) != -1)
			return FE_TYPE_OPERATOR;
		else if (raw[0] == '[')
			return FE_TYPE_LEFT_BRACKET;
		else if (raw[0] == ']')
		{
			return FE_TYPE_RIGHT_BRACKET;
		}
	}
	return FE_TYPE_OPERAND;
}
