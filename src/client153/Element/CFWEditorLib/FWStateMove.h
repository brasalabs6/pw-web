/* Original Element/CFWEditorLib/FWStateMove.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 136c90c7c3fd962ea1293eedd57277d2e157d1ff19ee069dc962406ed9e391fd. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATEMOVE_H_
#define _FWSTATEMOVE_H_

#include "FWState.h"
#include <AList2.h>
#include <APoint.h>
#include <ARect.h>

class CMoveCommand;
class FWGlyphList;
typedef unsigned int UINT;

class FWStateMove : public FWState  
{
	FW_DECLARE_DYNAMIC(FWStateMove)
public:
	virtual void OnMouseMove(UINT nFlags, APointI point);
	virtual void OnLButtonUp(UINT nFlags, APointI point);

	FWStateMove(FWView *pView, APointI mousePoint);
	virtual ~FWStateMove();
private:
	AList2<APointI, APointI&> m_centerList;

	APointI m_originalPoint;
	ARectI m_originalRect;

	void ClampOffsetToView(const FWGlyphList* pList, APointI& offset);
};

#endif 