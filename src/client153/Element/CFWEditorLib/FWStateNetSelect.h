/* Original Element/CFWEditorLib/FWStateNetSelect.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 be53648517d227a7933ff1a88a6c745c69b2e931214cde1d80fca083ee2d580d. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATENETSELECT_H_
#define _FWSTATENETSELECT_H_

#include "FWState.h"
#include <ARect.h>
#include <APoint.h>

typedef unsigned int UINT;

class FWStateNetSelect : public FWState  
{
	FW_DECLARE_DYNAMIC(FWStateNetSelect)
public:
	virtual void OnDraw();
	virtual void OnMouseMove(UINT nFlags, APointI point);
	virtual void OnLButtonUp(UINT nFlags, APointI point);
	virtual void OnLButtonDown(UINT nFlags, APointI point);

	FWStateNetSelect(FWView *pView, APointI mousePoint);
	virtual ~FWStateNetSelect();

private:
	ARectI m_Rect;
	APointI m_originalPoint;
};


#endif 