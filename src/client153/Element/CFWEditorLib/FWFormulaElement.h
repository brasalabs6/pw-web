/* Original Element/CFWEditorLib/FWFormulaElement.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 bbf3af8297013a478c222cdbc6661401d29c5158d8a18b973361089ad70a0be5. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWFORMULAELEMENT_H_
#define _FWFORMULAELEMENT_H_

#include "CodeTemplate.h"
#include "FWConfig.h"
#include <AString.h>

#define FE_TYPE_OPERATOR		0
#define FE_TYPE_LEFT_BRACKET	1
#define FE_TYPE_RIGHT_BRACKET	2
#define FE_TYPE_OPERAND			3

class FWFormulaElement  
{
protected:
	AString m_strRaw;	// element in string form
	int m_nType;		// see FE_TYPE_xxx
public:
	int GetType() const { return m_nType; }
	AString GetRawString() const { return m_strRaw; }

	FWFormulaElement();
	FWFormulaElement(const AString & raw);
	FWFormulaElement(const AString & raw, int nType);
	virtual ~FWFormulaElement();
	FWFormulaElement& operator = (const FWFormulaElement & src);
	IMPLEMENT_CLONE_INLINE(FWFormulaElement, FWFormulaElement);

	static int DetermineType(const AString & raw);
};

#endif 