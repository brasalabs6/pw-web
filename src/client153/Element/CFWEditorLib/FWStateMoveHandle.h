/* Original Element/CFWEditorLib/FWStateMoveHandle.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 69c639dc224eed7652b8501419efc9b7b25d217cf27d72505ff505027ef27e20. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATEMOVEHANDLE_H_
#define _FWSTATEMOVEHANDLE_H_

#include "FWState.h"

class FWCommandMoveHandle;
class FWGlyph;

class FWStateMoveHandle : public FWState  
{
	FW_DECLARE_DYNAMIC(FWStateMoveHandle)
public:
	virtual void OnMouseMove(UINT nFlags, APointI point);
	virtual void OnLButtonUp(UINT nFlags, APointI point);

	FWStateMoveHandle(FWView *pView, FWGlyph* pGlyph, int nHandle);
	virtual ~FWStateMoveHandle();

private:
	FWGlyph* m_pGlyph;
	int m_nHandle;
	APointI m_oldPoint;
};



#endif 