/* Original Element/CFWEditorLib/FWStateCreateBezier.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 52d5ce516607029bcf417b5a3f192df3845a0acf72c99d63aaa1b79ee73fce1b. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATECREATEBEZIER_H_
#define _FWSTATECREATEBEZIER_H_

#include "FWState.h"

class FWGlyphBezier;

class FWStateCreateBezier : public FWState  
{
	FW_DECLARE_DYNAMIC(FWStateCreateBezier)

private:
	FWGlyphBezier * m_pBezier;
	bool m_bLButtonDowned;
	bool m_bGraphInUse;
	// position of the current mouse pointer
	APointI m_curentPoint;
public:
	FWStateCreateBezier(FWView *pView, FWGlyphBezier *pGlyph);
	virtual ~FWStateCreateBezier();
	
	virtual void OnLButtonUp(UINT nFlags, APointI point);
	virtual void OnLButtonDown(UINT nFlags, APointI point);
	virtual void OnLClick(UINT nFlags, APointI point);
	virtual void OnMouseMove(UINT nFlags, APointI point);
	virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void OnDraw();
	virtual void OnRClick(UINT nFlags, APointI point);
};

#endif 