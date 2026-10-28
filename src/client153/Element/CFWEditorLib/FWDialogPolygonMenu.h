/* Original Element/CFWEditorLib/FWDialogPolygonMenu.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 8304a9762914f2f262cf51c7602f2d56b5dd85683d82e18603b90ba09c645145. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWDIALOGPOLYGONMENU_H_
#define _FWDIALOGPOLYGONMENU_H_

#include "FWDialogBase.h"

class FWDialogPolygonMenu : public FWDialogBase  
{
	FW_DECLARE_DYNCREATE(FWDialogPolygonMenu)
	DECLARE_COMMAND_MAP_FW()
public:
	FWDialogPolygonMenu();
	virtual ~FWDialogPolygonMenu();

	void OnCommandDuplicateHandle();
	void OnCommandRemoveHandle();

	virtual void Show (bool bShow, bool bModal = false);
};

#endif 