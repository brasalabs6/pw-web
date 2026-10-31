/* Original Element/CFWEditorLib/FWCommandUnCombine.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 fd014ab5a47ba3719a737f4bfeb369a02f71d1ae5541f7cfd61bd7a043df3312. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWCOMMANDUNCOMBINE_H_
#define _FWCOMMANDUNCOMBINE_H_


#include "FWCommand.h"

class FWGlyphComposite;
class FWDoc;

class FWCommandUnCombine : public FWCommand  
{
	FWGlyphComposite * m_pComposite;
	bool m_bCompositeInUse;
	FWDoc * m_pDoc;
public:
	FWCommandUnCombine(FWDoc *pDoc, FWGlyphComposite * pComposite);
	virtual ~FWCommandUnCombine();

	virtual void Execute();
	virtual void Unexecute();
};

#endif 
