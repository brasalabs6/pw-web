/* Original Element/CFWEditorLib/FWCommandDuplicateHandle.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 5529b6ea6e2a8895074ab89a7f9c98f642a9dace2aaf85e31053a68a058ccccb. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWCOMMANDDUPLICATEHANDLE_H_
#define _FWCOMMANDDUPLICATEHANDLE_H_

#include "FWCommand.h"

class FWGlyph;

class FWCommandDuplicateHandle : public FWCommand  
{
	FWGlyph * m_pGlyph;
	int m_nHandleToDuplicate;
public:
	FWCommandDuplicateHandle(FWGlyph *pGlyph, int nHandleToDuplicate);
	virtual ~FWCommandDuplicateHandle();

	virtual void Execute();
	virtual void Unexecute();

};

#endif 
