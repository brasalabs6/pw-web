/* Original Element/CFWEditorLib/FWDialogGraph.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 93e8551dc996730e5c00081d9ac10af5d409885ec9b4ca57135ecb11033b365e. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWDialogGraph.cpp: implementation of the FWDialogGraph class.
//
//////////////////////////////////////////////////////////////////////

#include "FWDialogGraph.h"
#include "AUIEditBox.h"
#include "FWAssemblyCache.h"

#define new A_DEBUG_NEW


FW_IMPLEMENT_DYNCREATE(FWDialogGraph, FWDialogBase)

BEGIN_EVENT_MAP_FW(FWDialogGraph, FWDialogPropBase)

END_EVENT_MAP_FW()



FWDialogGraph::FWDialogGraph() :
	m_fThickness(1.f)
{
}

FWDialogGraph::~FWDialogGraph()
{

}

int FWDialogGraph::GetProfileType()
{
	return FWAssemblyCache::TYPE_TEXT;
}

bool FWDialogGraph::Init(AUIDialog * pDlg)
{
	if (!FWDialogPropBase::Init(pDlg)) return false;

	m_pEditThickness = (AUIEditBox *) GetDlgItem("EDIT_THICKNESS");
	ASSERT(m_pEditThickness);

	return true;
}

void FWDialogGraph::OnOK()
{
	m_fThickness = ACString(m_pEditThickness->GetText()).ToFloat();

	FWDialogPropBase::OnOK();
}


void FWDialogGraph::OnShowDialog()
{
	ACString tmp;
	tmp.Format(_AL("%.3f"), m_fThickness);
	m_pEditThickness->SetText(tmp);

	FWDialogPropBase::OnShowDialog();
}
