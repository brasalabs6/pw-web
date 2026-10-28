/* Original Element/CFWEditorLib/FWDialogLanch.cpp, upstream e3d6d0075d0a745c9f9859f2cb25b01d71dec045, SHA-256 0a434793c173b3ef13b123fca43de155676a3eaa47bde7410f3535623cd56e6d. Authorization: docs/LEGAL.md (2026-10-25). GB18030 comments transcoded to UTF-8. */
// FWDialogLanch.cpp: implementation of the FWDialogLanch class.
//
//////////////////////////////////////////////////////////////////////

#include "FWDialogLanch.h"
#include "FWAssemblyCache.h"

#define new A_DEBUG_NEW


FW_IMPLEMENT_DYNCREATE(FWDialogLanch, FWDialogBase)

BEGIN_EVENT_MAP_FW(FWDialogLanch, FWDialogPropBase)

END_EVENT_MAP_FW()



FWDialogLanch::FWDialogLanch()
{

}

FWDialogLanch::~FWDialogLanch()
{

}

int FWDialogLanch::GetProfileType()
{
	return FWAssemblyCache::TYPE_LANCH;
}
