/* Original Element/CFWEditorLib/FWDialogScale.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 f09780fa16f369c601ffeeb51cf10e5dc10910e2992bbc972ae385cfec64e25c. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWDIALOGSCALE_H_
#define _FWDIALOGSCALE_H_

#include "FWDialogBase.h"

class AUIEditBox;

class FWDialogScale : public FWDialogBase
{
	FW_DECLARE_DYNCREATE(FWDialogScale)
	DECLARE_EVENT_MAP_FW()
protected:
	AUIEditBox * m_pEditScale;
public:
	float m_fScale;
public:
	FWDialogScale();
	virtual ~FWDialogScale();

	virtual bool Init(AUIDialog * pDlg);
	virtual void Show (bool bShow, bool bModal = false);
	virtual void OnOK();
	virtual void OnCancel();
};

#endif 