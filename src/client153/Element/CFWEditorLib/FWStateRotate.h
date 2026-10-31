/* Original Element/CFWEditorLib/FWStateRotate.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 a0db2114ac5603172eabee01d5b7983f93eb477235309de93ce5f29158852a38. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATEROTATE_H_
#define _FWSTATEROTATE_H_

#include "FWState.h"

class FWGlyph;
class FWCommandRotate;

class FWStateRotate : public FWState
{
	FW_DECLARE_DYNAMIC(FWStateRotate)
public:
	FWStateRotate(FWView *pView, FWGlyph *pGlyph);
	virtual ~FWStateRotate();

	virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void OnLButtonUp(UINT nFlags, APointI point);
	virtual void OnLButtonDown(UINT nFlags, APointI point);
	virtual void OnMouseMove(UINT nFlags, APointI point);
private:
	FWGlyph *m_pGlyph;
	bool m_bIsLButtonDown;
	double m_fOldAngle;
	double m_fFirstPointAngle;
};

#endif 