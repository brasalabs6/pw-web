/* Original Element/CFWEditorLib/FWDialogPlay.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 8df3ae092bb54e1d33e5c323a136bb9e1aebeb05afa37b0df4487e80ba0a4fc9. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWDIALOGPLAY_H_
#define _FWDIALOGPLAY_H_

#include "FWDialogBase.h"

class FWDialogPlay : public FWDialogBase  
{
	FW_DECLARE_DYNCREATE(FWDialogPlay)
	DECLARE_EVENT_MAP_FW()
public:
	FWDialogPlay();
	virtual ~FWDialogPlay();

	void OnClickBtnReturn(WPARAM wParam, LPARAM lParam);
	void OnClickBtnStop(WPARAM wParam, LPARAM lParam);
	void OnClickBtnPause(WPARAM wParam, LPARAM lParam);
	void OnClickBtnPlay(WPARAM wParam, LPARAM lParam);
};

#endif 