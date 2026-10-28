#include "DlgFaceName.h"
#include "DlgBase.h"
#include "AUIEditBox.h"
#include "AUIManager.h"
#include "AUIStillImageButton.h"
#include "DlgCustomizePre.h"

AUI_BEGIN_COMMAND_MAP(CDlgFaceName, CDlgCustomizeBase)

AUI_ON_COMMAND("confirm",		CDlgFaceName::OnCommand_confirm)
AUI_ON_COMMAND("IDCANCEL",		CDlgFaceName::OnCommand_CANCEL)

AUI_END_COMMAND_MAP()

CDlgFaceName::CDlgFaceName()
{
	m_pTxtInput = NULL;
	m_pBtnConfirm = NULL;
	wcscpy(m_wLawlessWChar, _AL("/\\:*?\"<>|"));
}

CDlgFaceName::~CDlgFaceName()
{
}

bool CDlgFaceName::OnInitContext()
{
	if (!CDlgCustomizeBase::OnInitContext())
		return false;
		
	UpdateData(false);
	
	return true;
}

void CDlgFaceName::DoDataExchange(bool bSave)
{
	DDX_Control("DEFAULT_Txt_Input", m_pTxtInput);
	DDX_Control("Btn_Confirm", m_pBtnConfirm);
}

void CDlgFaceName::OnCommand_confirm(const char * szCommand)
{
	m_AStrName = m_pTxtInput->GetText();
	if( m_AStrName.GetLength() <= 0)
	{
		return;
	}
	else if (!CheckValid())
	{
		ACHAR msg[256];
		a_sprintf(msg, GetStringFromTable(2902), _AL("/ \\ : * ? \" < > | "));
		GetAUIManager()->MessageBox("", msg, MB_OK, A3DCOLORRGBA(255, 255, 255, 160));
		return;
	}
	CDlgCustomizePre* pDlgCustomizePre = dynamic_cast<CDlgCustomizePre *>(GetAUIManager()->GetDialog("Win_ChoosePreCustomize"));
	if (pDlgCustomizePre->SaveUserCustomizeData((const ACHAR *)m_AStrName))
	{
		Show(false);
	}	
	m_pTxtInput->SetText(_AL(""));
	
}

void CDlgFaceName::OnCommand_CANCEL(const char * szCommand)
{

	m_pTxtInput->SetText(_AL(""));
	Show(false);
}

const ACHAR* CDlgFaceName::GetInput(void)
{
	return (const ACHAR *)m_AStrName;
}

void CDlgFaceName::OnTick()
{
	m_AStrName = m_pTxtInput->GetText();
	if( m_AStrName.GetLength() <= 0)
	{
		m_pBtnConfirm->Enable(false);
	}
	else
	{	
		m_pBtnConfirm->Enable(true);
	}
}

bool CDlgFaceName::CheckValid()
{
	for (int i = 0; i<=8; i++)
	{
		if(NULL != pw153::platform::wide_strchr(m_AStrName, m_wLawlessWChar[i]))	//	the port scanner (the libc wcschr reads 32-bit units).
			return false;
	}
	return true;
}