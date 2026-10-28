/* Original Element/CFWEditorLib/FWDialogMain.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 48991494e07cce6b820b0f0c501d099c563a2758472813d5c029a9a528d5c3ee. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWDMAIN_H_
#define _FWDMAIN_H_

#include "FWDialogBase.h"


class FWDialogMain : public FWDialogBase
{
	FW_DECLARE_DYNCREATE(FWDialogMain)
	DECLARE_EVENT_MAP_FW()
public:
	FWDialogMain(){};
	virtual ~FWDialogMain(){};

	void OnClickBtnLanch(WPARAM wParam, LPARAM lParam);
	void OnClickBtnPreview(WPARAM wParam, LPARAM lParam);
	void OnClickBtnCustom(WPARAM wParam, LPARAM lParam);
	void OnClickBtnText(WPARAM wParam, LPARAM lParam);
	void OnClickBtnSimple(WPARAM wParam, LPARAM lParam);
	void OnClickBtnGraph(WPARAM wParam, LPARAM lParam);

	virtual void OnOK();
	virtual void OnCancel();
};

#endif 