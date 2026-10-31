/* Original Element/CFWEditorLib/FWStateResize.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 9e8f60706bef93f4d6a0f07bdeafae6e395cc92dd8046c39dc5124253656e932. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATERESIZE_H_
#define _FWSTATERESIZE_H_

#include "FWState.h"
#include <ARect.h>
#include <APoint.h>

class FWGlyph;
typedef unsigned int UINT;

class FWStateResize : public FWState     
{
	FW_DECLARE_DYNAMIC(FWStateResize)
public:
	virtual void OnMouseMove(UINT nFlags, APointI point);
	virtual void OnLButtonUp(UINT nFlags, APointI point);
	virtual void OnLButtonDown(UINT nFlags, APointI point);
	virtual void OnDraw();

	FWStateResize(FWView *pView, FWGlyph *pGlyph);
	virtual ~FWStateResize();

private:
	FWGlyph *m_pGlyph;
	bool m_bLButtonDowned;
	ARectI m_rect;
	APointI m_ptMouseOld;
};

#endif 