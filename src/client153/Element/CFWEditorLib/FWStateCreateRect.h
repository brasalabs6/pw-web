/* Original Element/CFWEditorLib/FWStateCreateRect.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 1111f775d576871d0d9190f4202af388599086947305fed95ca14ab2d3823ea6. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATECREATERECT_H_
#define _FWSTATECREATERECT_H_

#include "FWState.h"
#include <ARect.h>
#include <APoint.h>

class FWGlyph;
class CCreateCommand;
typedef unsigned int UINT;

class FWStateCreateRect : public FWState  
{
	FW_DECLARE_DYNAMIC(FWStateCreateRect);
private:
	bool m_bGlyphInUse;
	FWGlyph* m_pGlyph;
	bool m_bMouseDown;

	ARectI m_rect;
	APointI m_upleftPoint;

	// make rect width and height to be same
	ARectI RegularizeRect(ARectI *pRect);
public:
	virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void OnDraw();
	virtual void OnMouseMove(UINT nFlags, APointI point);
	virtual void OnLButtonUp(UINT nFlags, APointI point);
	virtual void OnLButtonDown(UINT Flags, APointI point);

	FWStateCreateRect(FWView *pView, FWGlyph* pGlyph);
	virtual ~FWStateCreateRect();
};

#endif 