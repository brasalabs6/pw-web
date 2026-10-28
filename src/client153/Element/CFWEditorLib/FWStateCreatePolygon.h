/* Original Element/CFWEditorLib/FWStateCreatePolygon.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 4c4642f61ec1bc53e94883ded07ea434a0054833ad7f1af1fb90f998bf640fff. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATECREATEPOLYGON_H_
#define _FWSTATECREATEPOLYGON_H_

#include "FWState.h"
#include "FWArray.h"

class FWGlyphPolygon;

class FWStateCreatePolygon : public FWState     
{
	FW_DECLARE_DYNAMIC(FWStateCreatePolygon)
private:
	bool m_bIsGlyphInUse;
	FWGlyphPolygon * m_pGlyph;
	FWArray<APointI, APointI &> m_HandleArray;
	bool m_bIsLButtonDown;
	APointI m_curentPoint;
public:
	virtual void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual void OnLButtonDown(UINT nFlags, APointI point){m_bIsLButtonDown = true;}
	virtual void OnLButtonUp(UINT nFlags, APointI point){m_bIsLButtonDown = false;}
	virtual void OnMouseMove(UINT nFlags, APointI point);
	virtual void OnDraw();
	virtual void OnLClick(UINT nFlags, APointI point);
	virtual void OnRClick(UINT nFlags, APointI point);

	FWStateCreatePolygon(FWView *pView, FWGlyphPolygon* pGlyph);
	virtual ~FWStateCreatePolygon();
};

#endif 