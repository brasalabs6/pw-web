/* Original Element/CFWEditorLib/FWStateCreateFirework.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 ea742ff5e3362dd2fd397dbe26299aa59f0f93e9b668b3044cb8db50e9e04ccd. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATECREATEFIREWORK_H_
#define _FWSTATECREATEFIREWORK_H_

#include "FWState.h"

class FWGlyphFirework;

class FWStateCreateFirework : public FWState  
{
	FW_DECLARE_DYNAMIC(FWStateCreateFirework)
private:
	FWGlyphFirework * m_pGlyph;
	bool m_bGlyphInUse;
public:
	FWStateCreateFirework(FWView *pView, FWGlyphFirework *pGlyph);
	virtual ~FWStateCreateFirework();
	
	virtual void OnDraw();
	virtual void OnLClick(UINT nFlags, APointI point);
};

#endif 