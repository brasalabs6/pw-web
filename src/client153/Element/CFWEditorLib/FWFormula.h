/* Original Element/CFWEditorLib/FWFormula.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 a26491e40ee72d9973120539f1a0825e1285b0faeeb76f1947b0388d517a81d0. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWFORMULA_H_
#define _FWFORMULA_H_

#include <AStack.h>
#include <AList2.h>
#include <AString.h>
#include "FWConfig.h"

class FWFormulaElement;

typedef AStack<FWFormulaElement *, FWFormulaElement*> AFEStack;
typedef APtrList<FWFormulaElement *> AFEList;
class AScriptFile;

class FWFormula  
{
	// expression at left of "="
	AString m_strLValue;

	// token of expression in postfix form
	AFEList m_lstPostfix;	
public:
	FWFormula();
	virtual ~FWFormula();
	
	// load from ini file
	bool Load(AScriptFile *pFile);
	
	bool Compute() const;
protected:
	// called by Load(), build expression as postfix form
	void ReadAndBuildPostfixExpression(AScriptFile *pFile);
};

#endif 