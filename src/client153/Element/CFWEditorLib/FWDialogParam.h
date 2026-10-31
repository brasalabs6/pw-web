/* Original Element/CFWEditorLib/FWDialogParam.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 ef999c1b2304046392939fc43c93b34a6585ecc35195d6b6c6acc60da7334b06. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWDIALOGPARAM_H_
#define _FWDIALOGPARAM_H_

#include "FWDialogBase.h"
#include "FWParam.h"

class AUIListBox;
class AUIEditBox;
class AUILabel;

class FWDialogParam : public FWDialogBase  
{
	FW_DECLARE_DYNCREATE(FWDialogParam)
	DECLARE_EVENT_MAP_FW()
protected:
	void UpdateParam();
	void ShowProp();

protected:
	int m_nLastSelInListParam;
	AUIListBox * m_pListParam;
	AUILabel * m_pLabelParam;
	AUIEditBox * m_pEditParam;
public:
	FWDialogParam();
	virtual ~FWDialogParam();

	void OnSelChangeListParam(int nSelIndex);
	void OnLButtonDownListParam(WPARAM wParam, LPARAM lParam);
	FWParamArray m_aryParam;
	virtual bool Init(AUIDialog * pDlg);
	virtual void OnOK();
	virtual void OnCancel();
	virtual void Show (bool bShow, bool bModal = false);
};

#endif 