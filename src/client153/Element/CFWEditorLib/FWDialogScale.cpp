/* Original Element/CFWEditorLib/FWDialogScale.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 d250d44a15f3f836fa9a018cc3c803e18ea2b38f00d1ccf950a5b6ca501def00. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWDialogScale.cpp: implementation of the FWDialogScale class.
//
//////////////////////////////////////////////////////////////////////

#include "FWDialogScale.h"
#include "AUIEditBox.h"
#include "FWView.h"
#include "FWDoc.h"

#define new A_DEBUG_NEW


FW_IMPLEMENT_DYNCREATE(FWDialogScale, FWDialogBase)

BEGIN_EVENT_MAP_FW(FWDialogScale, FWDialogBase)

//ON_EVENT_FW("BTN_OK", WM_LCLICK, FWDialogScale::OnClickBtnOK)
//ON_EVENT_FW("BTN_CANCEL", WM_LCLICK, FWDialogScale::OnClickBtnCancel)

END_EVENT_MAP_FW()



FWDialogScale::FWDialogScale() :
	m_fScale(.5f)
{

}

FWDialogScale::~FWDialogScale()
{

}

void FWDialogScale::Show (bool bShow, bool bModal)
{
	if (bShow)
	{
		ACString str;
		str.Format(_AL("%.3f"), m_fScale);
		m_pEditScale->SetText(str);
	}

	FWDialogBase::Show(bShow, bModal);
}

void FWDialogScale::OnOK()
{
	m_fScale = ACString(m_pEditScale->GetText()).ToFloat();
	
	Show(false);

	FWDialogBase::OnOK();
}

void FWDialogScale::OnCancel()
{
	Show(false);

	FWDialogBase::OnCancel();
}

bool FWDialogScale::Init(AUIDialog * pDlg)
{
	FWDialogBase::Init(pDlg);

	m_pEditScale = (AUIEditBox *) GetDlgItem("EDIT_SCALE");
	ASSERT(m_pEditScale);

	return true;
}


