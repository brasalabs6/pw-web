/* Original Element/CFWEditorLib/FWStateMoveWindow.h, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 ffa96a99e9091f61f2fdf0d3538fd167678f12fa89d232ac012a1e5e2d5a6d9c. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
#ifndef _FWSTATEMOVEWINDOW_H_
#define _FWSTATEMOVEWINDOW_H_

#include "FWState.h"

class FWGlyph;

class FWStateMoveWindow : public FWState  
{
public:
	FWStateMoveWindow(FWView *pView);
	virtual ~FWStateMoveWindow();

	virtual void OnLButtonUp(UINT nFlags, APointI point);
};

#endif 