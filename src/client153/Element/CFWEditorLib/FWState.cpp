/* Original Element/CFWEditorLib/FWState.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 db82f1e9b8299a04b5ac5300237122645c1e319ce7254ece26048ebb27b72a01. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#include "FWState.h"
#include "FWDoc.h"
#include "FWView.h"
#include "FWFlatCollector.h"
#include "FWGlyphList.h"


#define new A_DEBUG_NEW



FW_IMPLEMENT_DYNAMIC(FWState, FWObject)

const int FWState::MAXCLICKTIME = 150;
const int FWState::LTIMERID = 1001;
const int FWState::RTIMERID = 1002;

FWState::FWState(FWView *pView) : 
	m_bDeleted(false),
	m_pView(pView)
{
}


FWState::~FWState()
{
	m_bDeleted = true;
}

void FWState::OnDraw()
{
	FWGlyphList* pList = GetDocument()->GetGlyphList();
	pList->Draw(GetView());
	pList->DrawHandles(GetView());
	GetView()->GetFlatCollector()->Flush_2D();
}

void FWState::OnLButtonUpImpl(UINT nFlags, APointI point)
{
	if(GetView()->KillTimer(LTIMERID) || point == m_leftPoint)
		OnLClick(nFlags, point);
	if (m_bDeleted) return;
	OnLButtonUp(nFlags, point);
}

void FWState::OnLButtonDownImpl(UINT nFlags, APointI point)
{
	m_leftFlags = nFlags;
	m_leftPoint = point;
	GetView()->SetTimer(LTIMERID, MAXCLICKTIME);
	OnLButtonDown(nFlags, point);
}

void FWState::OnRButtonUpImpl(UINT nFlags, APointI point)
{
	if(GetView()->KillTimer(RTIMERID) || m_rightPoint == point)
		OnRClick(nFlags, point);
	if (m_bDeleted) return;
	OnRButtonUp(nFlags, point);
}

void FWState::OnRButtonDownImpl(UINT nFlags, APointI point)
{
	m_rightFlags = nFlags;
	m_rightPoint = point;
	GetView()->SetTimer(RTIMERID, MAXCLICKTIME);
	OnRButtonDown(nFlags, point);
}

void FWState::OnTimerImpl(UINT nIDEvent)
{
	switch(nIDEvent)
	{
	case LTIMERID:
		GetView()->KillTimer(LTIMERID);
		OnLBeginDrag(m_leftFlags, m_leftPoint);
		return;
	case RTIMERID:
		GetView()->KillTimer(RTIMERID);
		OnRBeginDrag(m_rightFlags, m_rightPoint);
		return;
	}
	OnTimer(nIDEvent);
}


FWView * FWState::GetView()
{
	return m_pView;
}

FWDoc * FWState::GetDocument()
{
	return GetView()->GetDocument();
}

