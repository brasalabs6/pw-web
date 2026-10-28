/* Original Element/CFWEditorLib/FWCommandRemoveHandle.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 aed7590adc78eb507e7b56b6484b5db891ac12fbe29c7f8ebdcbe5addd84bfaa. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWCOMMANDREMOVEHANDLE_H_
#define _FWCOMMANDREMOVEHANDLE_H_


#include "FWCommand.h"
#include <AArray.h>
#include <APoint.h>

class FWGlyph;
typedef AArray<APointI, APointI&> APointArray;

class FWCommandRemoveHandle : public FWCommand  
{
	FWGlyph * m_pGlyph;
	int m_nHandleToRemove;
	APointArray m_aryHandleRemoved;
	void *m_pData;
public:
	// note : for some glyph (such as bezier), handles are divided into 
	// groups. handles of a group must be remove/restore at the same time
	FWCommandRemoveHandle(FWGlyph *pGlyph, int nHandleToRemove);
	virtual ~FWCommandRemoveHandle();

	virtual void Execute();
	virtual void Unexecute();

};

#endif 
