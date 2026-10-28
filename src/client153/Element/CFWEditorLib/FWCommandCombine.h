/* Original Element/CFWEditorLib/FWCommandCombine.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 3c40868b0422291419e923247ade560f5ab92a341e8fb80925ea8f0d6c1cb0ba. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWCOMMANDCOMBINE_H_
#define _FWCOMMANDCOMBINE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FWCommand.h"

class FWGlyphComposite;
class FWGlyph;
class FWDoc;

class FWCommandCombine : public FWCommand  
{
	FWGlyphComposite * m_pComposite;
	bool m_bCompositeInUse;
	FWDoc * m_pDoc;
public:
	FWCommandCombine(FWDoc *pDoc, FWGlyphComposite *pComposite);
	virtual ~FWCommandCombine();

	virtual void Execute();
	virtual void Unexecute();
};

#endif 
