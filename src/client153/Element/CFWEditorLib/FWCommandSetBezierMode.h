/* Original Element/CFWEditorLib/FWCommandSetBezierMode.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 85557850794817816b8c21753f529b8ece9a336ea2cddb22615365274f7c0896. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _SETBEZIERMODECOMMAND_H_
#define _SETBEZIERMODECOMMAND_H_

#include "FWCommand.h"
#include <AArray.h>

class FWGlyphBezier;
class FWView;

typedef AArray<int, int &> AIntArray;

class FWCommandSetBezierMode : public FWCommand  
{
	int m_nHandle;
	FWGlyphBezier * m_pBezier;
	int m_nNewMode;
	AIntArray m_aryOldMode;
public:
	FWCommandSetBezierMode(FWGlyphBezier * pBezier, int nHandle, int nMode);
	virtual ~FWCommandSetBezierMode();

	virtual void Execute();
	virtual void Unexecute();
};

#endif 
