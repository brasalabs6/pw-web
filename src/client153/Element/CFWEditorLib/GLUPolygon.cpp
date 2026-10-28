/* Original Element/CFWEditorLib/GLUPolygon.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 d541a29041cca116e9e7178121633526b643a068de6ac7848f1fd3376188fadb. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// GLUPolygon.cpp: implementation of the CGLUPolygon class.
//
//////////////////////////////////////////////////////////////////////

#include "GLUPolygon.h"
#include "GLUContour.h"
#include "CharOutline.h"
#include "CodeTemplate.h"

#define new A_DEBUG_NEW




CGLUPolygon::CGLUPolygon()
{
	m_pListGLUContours = new APtrList<CGLUContour *>;
}

CGLUPolygon::~CGLUPolygon()
{
	Destory();
	SAFE_DELETE(m_pListGLUContours);
}

void CGLUPolygon::Destory()
{
	ALISTPOSITION pos = m_pListGLUContours->GetHeadPosition();
	while (pos)
	{
		CGLUContour *p = m_pListGLUContours->GetNext(pos);
		SAFE_DELETE(p);
	}
	m_pListGLUContours->RemoveAll();
}

bool CGLUPolygon::CreateFromCharOutline(const CCharOutline *pCharOutline)
{
	ASSERT(pCharOutline);
	ASSERT(m_pListGLUContours->GetCount() == 0);

	if (pCharOutline->GetCharContoursList()->GetCount() == 0)
		return true;

	const APtrList<CCharContour *>* pListCharContour = pCharOutline->GetCharContoursList();
	ALISTPOSITION pos = pListCharContour->GetHeadPosition();
	while (pos)
	{
		CGLUContour *pGLUContour = new CGLUContour;
		pGLUContour->CreateFromCharContour(pListCharContour->GetNext(pos));
		m_pListGLUContours->AddTail(pGLUContour);
	}

	return true;
}
