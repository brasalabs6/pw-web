/* Original Element/CFWEditorLib/FWStateCreateText.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 a9847c70e2b6694d70a29df0bf3419c6a9d72c615cbb82ee3cefdb3232618adb. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATECREATETEXT_H_
#define _FWSTATECREATETEXT_H_

#include "FWState.h"

class FWGlyphText;

class FWStateCreateText : public FWState  
{
	FW_DECLARE_DYNAMIC(FWStateCreateText)
private:
	FWGlyphText * m_pGlyph;
	bool m_bGlyphInUse;
public:
	FWStateCreateText(FWView *pView, FWGlyphText *pGlyph);
	virtual ~FWStateCreateText();

	virtual void OnLClick(UINT nFlags, APointI point);
};

#endif 